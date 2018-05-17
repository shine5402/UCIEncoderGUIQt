#ifndef MAGICKHELPER_H
#define MAGICKHELPER_H

#include "processmanager.h"
#include <QObject>
#include <QProcess>
#include <QDebug>
#include "public_namespaces.h"
#include <QDir>
#include <QFileInfo>
#include "../LeafETestSystemAnswerer/leaflogger.h"
#include <exception>
class PictureHandler : public QObject
{
    Q_OBJECT
public:
    enum class UCIMode{avc,hevc};
    explicit PictureHandler(QObject* parent,ProcessManager* processManagerPtr,const QString &filePath, const qreal CRF, const UCIMode mode, const QStringList &otherArgument = QStringList()):QObject(parent),filePath(filePath),prm(processManagerPtr),CRF(CRF),mode(mode),otherArgument(otherArgument) {}
    virtual ~PictureHandler() {}
    void run(){
        StartTurn2BMP(filePath,otherArgument);
    }
private slots:
    void turn2BMP_processFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void turn2UCI_processFinished(int exitCode, QProcess::ExitStatus exitStatus);
private:
    void StartTurn2BMP(const QString &filePath, const QStringList &otherArgument= QStringList());
    QString filePath;
    QString bmpPath;
    ProcessManager* prm;
    qreal CRF;
    UCIMode mode;
    QStringList otherArgument;
    qint32 doTurnProcess(const QString &programDir, const QString &programFileName, const QString &filePath, const QString &newSuffix, const QStringList &otherArgument= QStringList(), const QString &nativeArguments = "");
    void StartTurn2UCI(const QString &filePath, const qreal CRF, const UCIMode mode, const QStringList &otherArgument = QStringList());
signals:
    void addTempFile(const QString filePath);
    void FinishAPicture();
    void showStatusMessage(const QString message);
    void addCommandMessage(const QString message);
};
class MagickHelper : public QObject
{
    Q_OBJECT
public:
    MagickHelper(QObject* parent):QObject(parent) {
       // connect(prm,SIGNAL(processDone()),this,SLOT(deleteTempFiles()));
    }
    void handleNewPicture(const QString &filePath, const qreal CRF, const PictureHandler::UCIMode mode, const QStringList &otherArgument = QStringList()){
        pictures.append(new PictureHandler(this,prm,filePath,CRF,mode,otherArgument));
        connect(pictures.value(pictures.count()-1),SIGNAL(addTempFile(QString)),this,SLOT(addTempFile(QString)));
        connect(pictures.value(pictures.count()-1),SIGNAL(FinishAPicture()),this,SLOT(addPictureFinished()));
        connect(pictures.value(pictures.count()-1),SIGNAL(showStatusMessage(QString)),this,SIGNAL(showStatusMessage(QString)));
        connect(pictures.value(pictures.count()-1),SIGNAL(addCommandMessage(QString)),this,SIGNAL(addCommandMessage(QString)));
    }
    void runAll(){
        foreach (auto i, pictures) {
            i->run();
        }
        prm->runAll();
    }
    ProcessManager* getProcessManager(){
        return prm;
    }

private:
    void doClean(){
        deleteTempFiles();
        pictures.clear();
        pictureFinished = 0;
        prm->clear();
        tempFilePaths.clear();
    }
    ProcessManager* prm = new ProcessManager(this);
    QList<PictureHandler*> pictures;
    QStringList tempFilePaths;
    qint32 pictureFinished = 0;
private slots:
    void addTempFile(const QString filePath){
        tempFilePaths.append(filePath);
    }
    void deleteTempFiles();
    void addPictureFinished(){
        if(++pictureFinished < pictures.count())
            return;
        else
        {
            doClean();
            AllFinished();
        }
    }
signals:
    void tempFileUnremoved(const QStringList unRemovedFiles);
    void AllFinished();
    void showStatusMessage(const QString message);
    void addCommandMessage(const QString message);
};

#endif // MAGICKHELPER_H
