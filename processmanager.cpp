#include "processmanager.h"
namespace  {
    LeafLogger logger;
}
ProcessManager::ProcessManager(QObject *parent) : QObject(parent)
{

}

qint32 ProcessManager::runNewProcess(const QString &program, const QStringList &argument,const QString& nativeArguments)
{
    auto id = newProcess(program,argument,nativeArguments);
    startProcess(id);
    return processCount;
}

qint32 ProcessManager::newProcess(const QString &program, const QStringList &argument,const QString& nativeArguments)
{
    auto newProcess = new process(this,++processCount,program,argument,nativeArguments);
    processes.insert(processCount,newProcess);
    connect(newProcess,SIGNAL(processErrorAvailable(int,QString)),this,SIGNAL(processErrorAvailable(int,QString)));
    connect(newProcess,SIGNAL(processOutputAvailable(int,QString)),this,SIGNAL(processOutputAvailable(int,QString)));
    return processCount;
}

process::process(QObject *parent, const int ID, const QString &program, const QStringList &argument, const QString& nativeArguments) : QObject(parent),ID(ID)
{
    QProcess* ptr = new QProcess(this);
    ptr->setProgram(program);
    ptr->setArguments(argument);
    processPtr = ptr;
    ptr->setNativeArguments(nativeArguments);
    connect(processPtr,SIGNAL(readyReadStandardOutput()),this,SLOT(StandardOutput()));
    connect(processPtr,SIGNAL(readyReadStandardError()),this,SLOT(StandardError()));
    connect(processPtr,SIGNAL(readyReadStandardError()),this,SLOT(StandardOutput()));
}

void process::StandardOutput(){
    logger.LogMessage(QString(u8"来自程序运行器ID %1 的输出：").arg(ID));
    auto output = QString::fromLocal8Bit(processPtr->readAllStandardOutput());
    logger.LogMessage(output,__func__);
    processOutputAvailable(ID,output);
}
/*
 * 这是一段被弃用的代码。原因是UCI Encoder无论什么输出，走的都是stderr通道
void process::StandardError(){
    logger.LogMessage(QString(u8"来自程序运行器ID %1 的错误：").arg(ID));
    auto error = QString::fromLocal8Bit(processPtr->readAllStandardError());
    logger.LogMessage(error,__func__);
    processErrorAvailable(ID,error);
}
*/
void process::StandardError(){
    logger.LogMessage(QString(u8"来自程序运行器ID %1 的错误：").arg(ID));
    auto error = QString::fromLocal8Bit(processPtr->readAllStandardError());
    logger.LogMessage(error,__func__);
    processErrorAvailable(ID,error);
}
