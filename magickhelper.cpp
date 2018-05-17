#include "magickhelper.h"
namespace  {
    LeafLogger logger;
}
void PictureHandler::StartTurn2BMP(const QString &filePath, const QStringList &otherArgument)
{
    if (QFileInfo(filePath).suffix() == "bmp"){
        StartTurn2UCI(filePath,CRF,mode,otherArgument);
        return;
    }
    auto id = doTurnProcess(paths::ImageMagickPath,"convert.exe",filePath,".bmp",otherArgument);
    connect(prm->getProcessPtr(id),SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(turn2BMP_processFinished(int,QProcess::ExitStatus)));
    showStatusMessage(QString(u8"对%1的BMP转换任务已经开始。").arg(filePath));

}

void PictureHandler::StartTurn2UCI(const QString &filePath,const qreal CRF, const UCIMode mode,const QStringList &otherArgument)
{
    QString nativeArgument;
    //QStringList argument;
    //argument << QString(" -q %1").arg(CRF);
    if (mode == UCIMode::hevc)
        nativeArgument.append(" -hevc ");
    //argument << otherArgument;

    nativeArgument.append(QString(" -q %1 ").arg(CRF));
    foreach (auto i, otherArgument) {
        nativeArgument.append(i).append(" ");
    }
    auto id = doTurnProcess(paths::UCIENCPath,"ucienc.exe",filePath,".uci",QStringList());
    connect(prm->getProcessPtr(id),SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(turn2UCI_processFinished(int,QProcess::ExitStatus)));
    showStatusMessage(QString(u8"对%1的UCI转换任务已经开始。").arg(filePath));
}
qint32 PictureHandler::doTurnProcess(const QString& programDir,const QString &programFileName,const QString& filePath,const QString& newSuffix,const QStringList &otherArgument,const QString &nativeArguments){
    QString program = QDir::toNativeSeparators(programDir+programFileName);
    QStringList argument;
    argument << QDir::toNativeSeparators(filePath) << otherArgument;

    if (newSuffix == ".bmp"){
        bmpPath = QDir::toNativeSeparators(QString(QDir::toNativeSeparators(QFileInfo(filePath).canonicalPath()).append("/")).append(QFileInfo(filePath).completeBaseName()).append(newSuffix));
        addTempFile(bmpPath);
        argument << bmpPath;
    }
    return prm->runNewProcess(program,argument,nativeArguments);
}
void PictureHandler::turn2BMP_processFinished(int exitCode, QProcess::ExitStatus exitStatus){
    if (exitStatus == QProcess::NormalExit){
        logger.LogMessage(QString(u8"对文件 %1 的BMP转换进程调用完成。").arg(filePath));
        addCommandMessage(QString(u8"对文件 %1 的BMP转换进程调用完成。").arg(filePath));
        logger.LogMessage(QString(u8"对文件 %1 的UCI转换进程调用开始。").arg(filePath));
        addCommandMessage(QString(u8"对文件 %1 的UCI转换进程调用开始。").arg(filePath));
        StartTurn2UCI(bmpPath,CRF,mode,otherArgument);
    } else{
        logger.LogMessage(QString(u8"对文件 %1 的BMP转换进程出现错误。exitCode为 %2").arg(filePath).arg(exitCode));
        addCommandMessage(QString(u8"对文件 %1 的BMP转换进程出现错误。exitCode为 %2").arg(filePath).arg(exitCode));
    }

}
void PictureHandler::turn2UCI_processFinished(int exitCode, QProcess::ExitStatus exitStatus){
    if (exitStatus == QProcess::NormalExit){
        logger.LogMessage(QString(u8"对文件 %1 的UCI转换进程调用完成。").arg(filePath));
        addCommandMessage(QString(u8"对文件 %1 的UCI转换进程调用完成。").arg(filePath));
        FinishAPicture();
    } else{
        logger.LogMessage(QString(u8"对文件 %1 的UCI转换进程出现错误。exitCode为 %2").arg(filePath).arg(exitCode));
        addCommandMessage(QString(u8"对文件 %1 的UCI转换进程出现错误。exitCode为 %2").arg(filePath).arg(exitCode));
    }
}

void MagickHelper::deleteTempFiles(){
    QStringList unRemoved;
    foreach (auto i, tempFilePaths) {
        QFile file(i);
        showStatusMessage(QString(u8"正在删除临时文件%1").arg(i));
        if (!file.remove())
            unRemoved << i;
        else
            showStatusMessage(QString(u8"临时文件%1已删除").arg(i));
    }
    if (unRemoved.count() != 0){
        tempFileUnremoved(unRemoved);
    }
}
