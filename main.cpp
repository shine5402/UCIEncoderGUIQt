#include "mainwindow.h"
#include <QApplication>
#include <QDateTime>
#include <stdio.h>
  #include <stdlib.h>

int main(int argc, char *argv[])
{

    QApplication a(argc, argv);
    QDateTime current_date_time =QDateTime::currentDateTime();
    QDir* dir = new QDir;
    if (!dir->exists("./log")){
        dir->mkdir("log");
    }
    delete dir;
    LeafLogger::setFilePath(QString("./log/log%1.txt").arg(current_date_time.toString("yyyyMMddhhmmsszzz")));
    MainWindow w;
    w.show();

    return a.exec();
}
