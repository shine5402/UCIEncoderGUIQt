#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{

    QApplication a(argc, argv);
    LeafLogger::LogInit();
    MainWindow w;
    w.show();
    try
    {
        return a.exec();
    }
    catch(std::exception& e){
        LeafLogger::LogMessage(QString(u8"程序运行过程中出现没有被处理的异常。异常信息为%1。程序被迫退出。").arg(e.what()));
    }
    catch(...){
        LeafLogger::LogMessage(QString(u8"程序运行中出现了一个非exception的异常。程序被迫退出。"));
    }
}
