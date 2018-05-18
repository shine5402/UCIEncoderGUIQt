#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#define VERSION_STRING "0.1.1"

#include <QMainWindow>
#include <QListWidget>
#include <QFile>
#include <QMessageBox>
#include <QFileDialog>
#include <QDragEnterEvent>
#include <QMimeData>
#include <exception>
#include <QDebug>
#include <QTextCodec>
#include <QProcess>
#include "magickhelper.h"
#include <QProgressBar>
#include <QTextCursor>
#include <QTextBrowser>
#include <QTextDocument>
#include <QTimer>
namespace Ui {
    class MainWindow;
}


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void refreshUIProgress(int progressValue);
    void startUIProgress(int progressMin,int progressMax);
    void progressDone();
    void showStatusMessage(const QString message);
    void showUnremovedTempFiles(const QStringList unRemovedFiles);
    void showUnrenamedTempFiles(const QList<QPair<QString, QString> > unRenamedFiles);

    void addCommandOutput(const int ProcessID, const QString output);
    void addCommandError(const int ProcessID, const QString error);
    void addCommandMessage(const QString message);

private slots:
    void on_clearButton_clicked();

    void on_deleteButton_clicked();

    void on_addButton_clicked();

    void on_runButton_clicked();

    void on_x264radioButton_clicked();

    void on_x265radioButton_clicked();

    void on_actionCommandOutput_triggered(bool checked);

    void on_dockWidget_visibilityChanged(bool visible);

    void on_actionAboutQt_triggered();
 void returnToOldMaxMinSizes();
 void on_commandOuptutBrowser_textChanged();

 void on_actionAbout_triggered();


private:
    Ui::MainWindow *ui;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    QStringList getFilePaths();

    MagickHelper *magickhelper = new MagickHelper(this);
    struct exceptions
    {
        exceptions() {}
        class FileListEmpty : public std::runtime_error{
        public:
            FileListEmpty():runtime_error(u8"文件列表为空。"){}
        };
    };

    void clearFileList();
    QProgressBar* progress = new QProgressBar(this);
    void disableNewTasks();
    void enableNewTasks();
    void setDockSize(QDockWidget *dock, int setWidth, int setHeight);
};

namespace MessageBox{
     void warning(QWidget* parent,const QString& messageText);
     void warning(QWidget* parent,const wchar_t* messageText);
     void information(QWidget* parent,const QString& messageText);
     void information(QWidget* parent,const wchar_t* messageText);
}

#endif // MAINWINDOW_H
