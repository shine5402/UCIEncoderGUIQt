#include "magickhelper.h"
namespace  {
    LeafLogger logger;
}
void PictureHandler::StartTurn2BMP(const QString &filePath, const QStringList &otherArgument)
{
    if (QFileInfo(filePath).suffix() == "bmp"){
        StartTurn2UCI(filePath,CRF,mode,otherArgument,UCINativeArguments);
        return;
    }
    auto id = doTurnProcess(paths::ImageMagickPath,"convert.exe",filePath,".bmp",otherArgument);
    connect(prm->getProcessPtr(id),SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(turn2BMP_processFinished(int,QProcess::ExitStatus)));
    showStatusMessage(QString(u8"对%1的BMP转换任务已经开始。").arg(filePath));

}

void PictureHandler::StartTurn2UCI(const QString &filePath, const qreal CRF, const UCIMode mode, const QStringList &otherArgument, const QString &UCINativeArguments)
{
    QString nativeArgument;
    if (mode == UCIMode::hevc)
        nativeArgument.append(" -hevc ");
    nativeArgument.append(QString(" -q %1 ").arg(CRF));
    foreach (auto i, otherArgument) {
        nativeArgument.append(i).append(" ");
    }
    nativeArgument.append(UCINativeArguments);
    auto id = doTurnProcess(paths::UCIENCPath,"ucienc.exe",filePath,".uci",QStringList(),nativeArgument);
    connect(prm->getProcessPtr(id),SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(turn2UCI_processFinished(int,QProcess::ExitStatus)));
    showStatusMessage(QString(u8"对%1的UCI转换任务已经开始。").arg(filePath));
}
qint32 PictureHandler::doTurnProcess(const QString& programDir,const QString &programFileName,const QString& filePath,const QString& newSuffix,const QStringList &otherArgument,const QString &nativeArguments){
    QString program = QDir::toNativeSeparators(programDir+programFileName);
    QStringList argument;
    argument << QDir::toNativeSeparators(filePath) << otherArgument;

    if (newSuffix == ".bmp"){
        auto baseName = QDir::toNativeSeparators(QString(QDir::toNativeSeparators(QFileInfo(filePath).canonicalPath()).append("/")).append(QFileInfo(filePath).completeBaseName()));
        bmpPath = baseName + newSuffix;
        if (QFileInfo(bmpPath).exists()){
            QString newBaseName;
            while (QFileInfo(bmpPath).exists()){
                newBaseName =baseName + QString("%1").arg(QRandomGenerator::global()->generate());
                bmpPath = newBaseName + newSuffix;
            }
            addNeedRenamePaths(newBaseName + ".uci",baseName + ".uci");
        }

        addTempFile(bmpPath);
        argument << bmpPath;
    }
    return prm->runNewProcess(program,argument,nativeArguments);
}
void PictureHandler::logAndCommandAndStatus(const QString message)
{
    logger.LogMessage(message);
    addCommandMessage(message);
    showStatusMessage(message);
}

void PictureHandler::turn2BMP_processFinished(int exitCode, QProcess::ExitStatus exitStatus){
    if (exitStatus == QProcess::NormalExit){
        logAndCommandAndStatus(QString(u8"对文件 %1 的BMP转换进程调用完成。").arg(filePath));
        logAndCommandAndStatus(QString(u8"对文件 %1 的UCI转换进程调用开始。").arg(filePath));
        StartTurn2UCI(bmpPath,CRF,mode,otherArgument,UCINativeArguments);
    } else{
        logAndCommandAndStatus(QString(u8"对文件 %1 的BMP转换进程出现错误。exitCode为 %2").arg(filePath).arg(exitCode));
    }

}
void PictureHandler::turn2UCI_processFinished(int exitCode, QProcess::ExitStatus exitStatus){
    if (exitStatus == QProcess::NormalExit){
        logAndCommandAndStatus(QString(u8"对文件 %1 的UCI转换进程调用完成。").arg(filePath));
        FinishAPicture();
    } else{
        logAndCommandAndStatus(QString(u8"对文件 %1 的UCI转换进程出现错误。exitCode为 %2").arg(filePath).arg(exitCode));
    }
}

void MagickHelper::doRename()
{
    QList<QPair<QString,QString>> unrenamed;
    foreach (auto filePath, needRenamePaths) {
        logAndCommandAndStatus(QString(u8"正在重命名文件%1 -> %2").arg(filePath.first).arg(filePath.second));
        if (!QFile(filePath.first).rename(filePath.second))
            unrenamed.append(filePath);
        else
           logAndCommandAndStatus(QString(u8"重命名文件%1 -> %2完成").arg(filePath.first).arg(filePath.second));
    }
    if (unrenamed.count() != 0)
        fileUnrenamed(unrenamed);
}

void MagickHelper::logAndCommandAndStatus(const QString message)
{
    logger.LogMessage(message);
    addCommandMessage(message);
    showStatusMessage(message);
}

void MagickHelper::deleteTempFiles(){
    QStringList unRemoved;
    foreach (auto i, tempFilePaths) {
        QFile file(i);
        logAndCommandAndStatus(QString(u8"正在删除临时文件%1").arg(i));
        if (!file.remove())
            unRemoved << i;
        else
            logAndCommandAndStatus(QString(u8"临时文件%1已删除").arg(i));
    }
    if (unRemoved.count() != 0){
        tempFileUnremoved(unRemoved);
    }
}
