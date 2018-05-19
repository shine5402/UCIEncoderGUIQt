#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->statusBar->addPermanentWidget(progress);
    connect(magickhelper->getProcessManager(),SIGNAL(refreshProgress(int)),this,SLOT(refreshUIProgress(int)));
    connect(magickhelper,SIGNAL(AllFinished()),this,SLOT(progressDone()));
    connect(magickhelper,SIGNAL(tempFileUnremoved(QStringList)),this,SLOT(showUnremovedTempFiles(QStringList)));
    //此处为了迎合UCI Encoder的输出策略，Err也输出为Output
    connect(magickhelper->getProcessManager(),SIGNAL(processErrorAvailable(int,QString)),this,SLOT(addCommandOutput(int,QString)));
    connect(magickhelper->getProcessManager(),SIGNAL(processOutputAvailable(int,QString)),this,SLOT(addCommandOutput(int,QString)));
    ui->statusBar->showMessage(u8"就绪");
    connect(magickhelper,SIGNAL(showStatusMessage(QString)),this,SLOT(showStatusMessage(QString)));
    setDockSize(ui->dockWidget,600,150);
    ui->showAdvancedButton->setVisible(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::clearFileList()
{
    ui->fileListWidget->clear();
}

void MainWindow::on_clearButton_clicked()
{
    clearFileList();
}

void MainWindow::on_deleteButton_clicked()
{
    if (ui->fileListWidget->currentRow() != -1){
        //        delete ui->fileListWidget->takeItem(ui->fileListWidget->currentRow());
        auto selectedItems = ui->fileListWidget->selectedItems();
        foreach (auto i, selectedItems) {
            ui->fileListWidget->removeItemWidget(i);
            delete i;
        }
    } else
        MessageBox::warning(this,L"没有选中文件的说！");

}

void MainWindow::on_addButton_clicked()
{
    auto fileNames = QFileDialog::getOpenFileNames(this,QStringLiteral("打开文件"),"",QStringLiteral("所有文件 (*.*)"));
    foreach (auto fileName, fileNames) {
        ui->fileListWidget->addItem(fileName);
    }

}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("text/uri-list")) {
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent *event)
{
    auto urls = event->mimeData()->urls();
    if(urls.isEmpty())
        return;

    //往文本框中追加文件名
    foreach(auto url, urls) {
        //QString file_name = url.toLocalFile();
        //textEdit->append(file_name);
        ui->fileListWidget->addItem(url.toLocalFile());
    }
}

QStringList MainWindow::getFilePaths()
{
    if (ui->fileListWidget->count() != 0){
        QStringList tmp;
        for (int i = 0;i<ui->fileListWidget->count();++i){
            tmp.append(ui->fileListWidget->item(i)->text());
        }
        return tmp;
    } else
        throw exceptions::FileListEmpty();
}


void MainWindow::on_runButton_clicked()
{
    try{
        auto paths = getFilePaths();
        foreach (auto path, paths) {
            // qDebug() << path;
            magickhelper->handleNewPicture(path,ui->CRFBox->value(),ui->x264radioButton->isChecked()?(PictureHandler::UCIMode::avc):(PictureHandler::UCIMode::hevc));
        }
        magickhelper->runAll();
        startUIProgress(1,2*ui->fileListWidget->count());
        //clearFileList();
        disableNewTasks();
    }
    catch (exceptions::FileListEmpty &){
        MessageBox::warning(this,L"文件列表为空的说！");
    }

}

void MainWindow::on_x264radioButton_clicked()
{
    if (ui->x264radioButton->isChecked())
        ui->CRFBox->setValue(27.0);
}

void MainWindow::on_x265radioButton_clicked()
{
    if (ui->x265radioButton->isChecked())
        ui->CRFBox->setValue(53.0);
}

void MainWindow::refreshUIProgress(int progressValue)
{
    progress->setValue(progressValue);
}

void MainWindow::startUIProgress(int progressMin, int progressMax)
{
    progress->setMinimum(progressMin);
    progress->setMaximum(progressMax);
}

void MainWindow::progressDone()
{
    progress->reset();
    MessageBox::information(this,u8"任务完成！");
    clearFileList();
    enableNewTasks();
    ui->statusBar->clearMessage();
    ui->statusBar->showMessage(u8"就绪");
}

void MainWindow::showStatusMessage(const QString message)
{
    ui->statusBar->showMessage(message);
}

void MainWindow::showUnremovedTempFiles(const QStringList unRemovedFiles)
{
    QString message;
    message.append(u8"<h3>程序无法删除某些临时文件……</h3>") .append( u8"<p>请您手动删除他们。</p>").append(u8"<p>以下是文件路径：</p>");
    foreach (auto i, unRemovedFiles) {
        message.append(QString(u8"<p>%1</p>").arg(i));
    }
    MessageBox::warning(this,message);
}

void MainWindow::showUnrenamedTempFiles(const QList<QPair<QString,QString>> unRenamedFiles)
{
    QString message;
    message.append(u8"<h3>程序无法重命名某些文件……</h3>") .append( u8"<p>请您手动重命名他们。</p>").append(u8"<p>以下是文件列表：</p>");
    foreach (auto i, unRenamedFiles) {
        message.append(QString(u8"<p>%1 -> %2</p>").arg(i.first).arg(i.second));
    }
    MessageBox::warning(this,message);
}

void MainWindow::addCommandOutput(const int ProcessID,const QString output)
{
    ui->commandOuptutBrowser->insertPlainText(QString(u8"来自程序运行器ID %1 的输出：").arg(ProcessID));
    ui->commandOuptutBrowser->insertHtml("<p><br/></p>");
    ui->commandOuptutBrowser->insertPlainText(output);
    ui->commandOuptutBrowser->insertHtml("<p><br/></p>");
    ui->commandOuptutBrowser->insertPlainText(u8"================================");
    ui->commandOuptutBrowser->insertHtml("<p><br/></p>");
    ui->commandOuptutBrowser->repaint();
}
void MainWindow::addCommandError(const int ProcessID,const QString error)
{
    ui->commandOuptutBrowser->insertHtml(QString(u8"<p style=\"color: red;\">来自程序运行器ID %1 的错误：</p>").arg(ProcessID));
    ui->commandOuptutBrowser->insertHtml("<p><br/></p>");
    ui->commandOuptutBrowser->insertHtml(QString(u8"<p style=\"color: red;\">%1<br/></p>").arg(error));
    ui->commandOuptutBrowser->insertHtml(u8"<p style=\"color: red;\">================================</br></p>");
    ui->commandOuptutBrowser->repaint();
}

void MainWindow::addCommandMessage(const QString message)
{
    ui->commandOuptutBrowser->insertPlainText(message);
    ui->commandOuptutBrowser->insertHtml("<p><br/></p>");
}

void MainWindow::disableNewTasks(){
    ui->addButton->setEnabled(false);
    ui->clearButton->setEnabled(false);
    ui->deleteButton->setEnabled(false);
    ui->CRFBox->setEnabled(false);
    ui->fileListWidget->setEnabled(false);
    ui->runButton->setEnabled(false);
    ui->x264radioButton->setEnabled(false);
    ui->x265radioButton->setEnabled(false);
}
void MainWindow::enableNewTasks(){
    ui->addButton->setEnabled(true);
    ui->clearButton->setEnabled(true);
    ui->deleteButton->setEnabled(true);
    ui->CRFBox->setEnabled(true);
    ui->fileListWidget->setEnabled(true);
    ui->runButton->setEnabled(true);
    ui->x264radioButton->setEnabled(true);
    ui->x265radioButton->setEnabled(true);
}
namespace MessageBox
{
    void warning(QWidget* parent,const QString& messageText){
        QMessageBox::warning(parent,QString::fromWCharArray(L"(｀･ω･´)"),messageText);
    }
    void warning(QWidget* parent,const wchar_t* messageText){
        QMessageBox::warning(parent,QString::fromWCharArray(L"(｀･ω･´)"),QString::fromWCharArray(messageText));
    }
    void information(QWidget* parent,const QString& messageText){
        QMessageBox::information(parent,QString::fromWCharArray(L"(`・ω・´)"),messageText);
    }
    void information(QWidget* parent,const wchar_t* messageText){
        QMessageBox::warning(parent,QString::fromWCharArray(L"(`・ω・´)"),QString::fromWCharArray(messageText));
    }
}

void MainWindow::on_actionCommandOutput_triggered(bool checked)
{
    if (checked)
        ui->dockWidget->show();
    else
        ui->dockWidget->hide();
}

void MainWindow::on_dockWidget_visibilityChanged(bool visible)
{
    if (visible){
        ui->actionCommandOutput->setChecked(true);
        setDockSize(ui->dockWidget,600,150);
    }
    else
        ui->actionCommandOutput->setChecked(false);
}

void MainWindow::on_actionAboutQt_triggered()
{
    QMessageBox::aboutQt(this,u8"关于Qt");
}
QSize oldMaxSize, oldMinSize;

void MainWindow::setDockSize(QDockWidget* dock, int setWidth,int setHeight)
{

    oldMaxSize=dock->maximumSize();
    oldMinSize=dock->minimumSize();

    if (setWidth>=0)
        if (dock->width()<setWidth)
            dock->setMinimumWidth(setWidth);
        else dock->setMaximumWidth(setWidth);
    if (setHeight>=0)
        if (dock->height()<setHeight)
            dock->setMinimumHeight(setHeight);
        else dock->setMaximumHeight(setHeight);

    QTimer::singleShot(1, this, SLOT(returnToOldMaxMinSizes()));
}

void MainWindow::returnToOldMaxMinSizes()
{
    ui->dockWidget->setMinimumSize(oldMinSize);
    ui->dockWidget->setMaximumSize(oldMaxSize);
}

void MainWindow::on_commandOuptutBrowser_textChanged()
{
    ui->commandOuptutBrowser->moveCursor(QTextCursor::End);
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this,u8"关于 UCI Encoder GUI",QString(u8"<h3>关于 UCI Encoder GUI</h3>"
                                                           u8"<p>版本 %1 编译时间：%2 %3</p>"
                                                           u8"<p>版权所有 2018 shine_5402</p>"
                                                           u8"<p>UCI Encoder GUI 是 UCI Encoder 的一个简单前端GUI。它可以让使用者更加轻松地应对使用 UCI Encoder 进行大量图片操作的情况。</p>"
                                                           u8"<p>本程序源代码以 GNU GPL v3 协议开源，您可以依据此协议的条款重新分发和/或修改它。</p>"
                                                           u8"<p>我们分发这个程序的目的是希望它有用，但是我们对它的可用性没有任何保证，甚至也没有对于适销性或适用于特定用途的默示保证。有关详细信息，请参阅GNU通用公共许可证。</p>"
                                                           u8"<p>您应该在收到本程序的同时就已经收到GNU通用公共许可证的副本。 如果没有，请参阅"
                                                           u8"<a href=\"http://www.gnu.org/licenses/\">http://www.gnu.org/licenses/</a>。</p>"
                                                           u8"<p>本程序的发行包中包含了以下开源软件：</p>"
                                                           u8"<ul>"
                                                           u8"<li>UCI Encoder (<a href=\"https://www.gnu.org/licenses/lgpl.html\">GNU LGPL v3</a>)</li>"
                                                           u8"<li>ImageMagick (<a href=\"https://www.imagemagick.org/script/license.php\">Apache 2.0 license</a>)</li>"
                                                           u8"<li>x264 (xiaowan) (<a href=\"https://www.gnu.org/licenses/old-licenses/gpl-2.0.html\">GNU GPL v2</a>)</li>"
                                                           u8"<li>x265 (<a href=\"https://www.gnu.org/licenses/old-licenses/gpl-2.0.html\">GNU GPL v2</a>)</li>"
                                                           u8"</ul>").arg(VERSION_STRING).arg(__DATE__).arg(__TIME__));
}
