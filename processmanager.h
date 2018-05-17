#ifndef PROCESSMANAGER_H
#define PROCESSMANAGER_H

#include <QObject>
#include <QProcess>
#include <QDateTime>
#include <exception>
#include <QDebug>
#include "../LeafETestSystemAnswerer/leaflogger.h"
class process :public QObject
{
    Q_OBJECT
public:
//        process(QObject* parent):QObject(parent){
//           processPtr = nullptr;
//        }
    explicit process(QObject* parent,const int ID,QProcess* processPtr):QObject(parent),processPtr(processPtr),ID(ID) {
    }
    explicit process(QObject* parent,const int ID, const QString& program, const QStringList& argument, const QString &nativeArguments = "");
    QProcess* getPtr() const{
        return processPtr;
    }
    QDateTime getStartTime()const{
        return startTime;
    }
    QDateTime getFinishTime() const{
        return finishTime;
    }
    QProcess::ProcessState getState() const{
        return processPtr->state();
    }
    void start(){
        processPtr->start();
    }
signals:
    void processOutputAvailable(const int ProcessID,const QString output);
    void processErrorAvailable(const int ProcessID,const QString error);
private:
    QProcess* processPtr;
    QDateTime startTime;
    QDateTime finishTime;
    int ID;
private slots:
    void StandardOutput();
    void StandardError();
    void setStartTime(){
        startTime = QDateTime::currentDateTime();
    }
    void setFinishTime(){
        finishTime = QDateTime::currentDateTime();
    }
};
class ProcessManager : public QObject
{
    Q_OBJECT
public:
    explicit ProcessManager(QObject *parent = nullptr);
    qint32 runNewProcess(const QString& program, const QStringList& argument, const QString &nativeArguments = "");
    qint32 newProcess(const QString& program, const QStringList& argument, const QString &nativeArguments = "");
    bool deleteProcess(const qint32 id){
        if (processes.value(id)->getState() != QProcess::Running)
            return processes.remove(id);//这里转换返回值为bool。
        throw std::runtime_error(QString("Process %1 is still running.").arg(id).toStdString());
    }
    QDateTime getStartTime(qint32 id) const{
        return processes.value(id)->getStartTime();
    }
    QDateTime getFinishTime(qint32 id) const{
        return processes.value(id)->getFinishTime();
    }
    QProcess* getProcessPtr(qint32 id) const{
        return processes.value(id)->getPtr();
    }
    QProcess::ProcessState getState(qint32 id) const{
        return processes.value(id)->getState();
    }
    qint32 getProcessCount()const{
        return processCount;
    }
    void startProcess(qint32 id){
        processes.value(id)->start();
    }
    void runAll(){
        for (int i = 1;i<=processCount;++i){
            startProcess(i);
        }
    }
    void clear(){
        processes.clear();
        processCount = 0;
    }
signals:
    void refreshProgress(int progressValue);
    void processOutputAvailable(const int ProcessID,const QString output);
    void processErrorAvailable(const int ProcessID,const QString error);
public slots:
private:

    QHash<qint32,process*> processes;
    qint32 processCount = 0;
};

#endif // PROCESSMANAGER_H
