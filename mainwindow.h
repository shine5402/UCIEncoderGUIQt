#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "publicdefines.h"
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
#include <QBoxLayout>
#include <QRegExp>
#include <QRegExpValidator>
#include <QInputDialog>
#include <QComboBox>
namespace Ui {
    class MainWindow;
}
constexpr double CRF_MAX = 51;
constexpr double CRF_MIN = 0;
constexpr double CRF_DEFALUT_QUALITY = 0.6;
constexpr int CRF_SLIDER_DEFAULT_VALUE = 60;

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

    void on_showAdvancedButton_clicked();

    void on_QcheckBox_stateChanged(int arg1);

    void on_xCheckBox_stateChanged(int arg1);

    void on_XCheckBox_stateChanged(int arg1);

    void on_pCheckBox_stateChanged(int arg1);

    void on_i420CheckBox_stateChanged(int arg1);

    void on_i420wLineEdit_textChanged(const QString &arg1);

    void on_xPushButton_clicked();

    void on_XPushButton_clicked();

    void on_i444CheckBox_stateChanged(int arg1);
#ifdef Q_OS_WIN64
    void toolChainBoxChanged(const QString &text);
#endif
    void on_CRFhorizontalSlider_valueChanged(int value);

    void on_CRFhorizontalSlider_Q_valueChanged(int value);

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
    void hideAdvanced();
    void showAdvanced();
    QString xString;
    QString XString;
    bool isAdvancedModified();
    QString getAdvancedNativeArguments();

    double CRF = (CRF_MAX - CRF_MIN)*(1 - CRF_DEFALUT_QUALITY) + CRF_MIN;
    double CRF_Q = (CRF_MAX - CRF_MIN)*(1 - CRF_DEFALUT_QUALITY) + CRF_MIN;
    void updateCRFFromSliderValue(int value);
    void updateCRFQFromSliderValue(int value);
    void updateCRFFromCRFValue(double CRF);
    void updateCRFQFromCRFValue(double CRF_Q);
};

namespace MessageBox{
    void warning(QWidget* parent,const QString& messageText);
    void warning(QWidget* parent,const wchar_t* messageText);
    void information(QWidget* parent,const QString& messageText);
    void information(QWidget* parent,const wchar_t* messageText);
}

#endif // MAINWINDOW_H
