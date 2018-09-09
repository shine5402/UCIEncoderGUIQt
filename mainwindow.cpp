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
    hideAdvanced();
    QRegExp regx(R"(^[1-9]\d*$)");
    QRegExpValidator* numValidator = new QRegExpValidator(regx,this);
    ui->i420hLineEdit->setValidator(numValidator);
    ui->i420wLineEdit->setValidator(numValidator);
    ui->i444hLineEdit->setValidator(numValidator);
    ui->i444wLineEdit->setValidator(numValidator);
#ifdef Q_OS_WIN64
    this->setWindowTitle(this->windowTitle() + " x64");
#endif
if (QSysInfo::currentCpuArchitecture() == "x86_64"){
    if (QFileInfo("./TOOLCHAIN_WIN64_WITH_WIN32").exists()){
        LeafLogger::LogMessage(u8"检测到TOOLCHAIN_WIN64_WITH_WIN32文件。");
        QComboBox* toolchainBox = new QComboBox(this);
        QStringList toolchains {"x64","x86"};
        toolchainBox->addItems(toolchains);
        toolchainBox->setInsertPolicy(QComboBox::NoInsert);
        connect(toolchainBox,SIGNAL(currentIndexChanged(QString)),this,SLOT(toolChainBoxChanged(QString)));
        ui->runButtonLayout->insertWidget(1,toolchainBox);}
    if (QSysInfo::buildCpuArchitecture() == "i386"){
        MessageBox::information(this,u8"看起来您的系统架构为x86_64，但您运行了i386的UCI Encoder GUI。\n您仍可以使用UCI Encoder GUI的全部功能（甚至可以调用x64工具链）。但我们仍建议您使用x64的UCI Encoder GUI。");
    }
}

    updateCRFFromCRFValue(CRF_X264_DEFAULT);

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
        LeafLogger::LogMessage(QString(u8"通过添加按钮添加了此文件：%1").arg(fileName));
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
        ui->fileListWidget->addItem(url.toLocalFile());
        LeafLogger::LogMessage(QString(u8"通过拖拽添加了此文件：%1").arg(url.toLocalFile()));
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
        auto advancedNativeArguments = QString();
        if (isAdvancedModified())
            advancedNativeArguments = getAdvancedNativeArguments();
        foreach (auto path, paths) {
            magickhelper->handleNewPicture(path,CRF,ui->x264radioButton->isChecked()?(PictureHandler::UCIMode::avc):(PictureHandler::UCIMode::hevc),QStringList(),advancedNativeArguments);
        }
        magickhelper->runAll();
        startUIProgress(1,2*ui->fileListWidget->count());
        disableNewTasks();
    }
    catch (exceptions::FileListEmpty &){
        MessageBox::warning(this,L"文件列表为空的说！");
    }

}

void MainWindow::on_x264radioButton_clicked()
{
    if (ui->x264radioButton->isChecked())
        updateCRFFromCRFValue(CRF_X264_DEFAULT);
}

void MainWindow::on_x265radioButton_clicked()
{
    if (ui->x265radioButton->isChecked())
        updateCRFFromCRFValue(CRF_X265_DEFAULT);
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
    if (output.contains(u8"error",Qt::CaseInsensitive)){
        addCommandError(ProcessID,output);
        return;
    }
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
    centralWidget()->setEnabled(false);
}
void MainWindow::enableNewTasks(){
    centralWidget()->setEnabled(true);
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
        //setDockSize(ui->dockWidget,600,150);
    }
    else
        ui->actionCommandOutput->setChecked(false);
}

void MainWindow::on_actionAboutQt_triggered()
{
    QMessageBox::aboutQt(this,u8"关于Qt");
}

void MainWindow::on_commandOuptutBrowser_textChanged()
{
    ui->commandOuptutBrowser->moveCursor(QTextCursor::End);
}

void MainWindow::on_actionAbout_triggered()
{
    auto version = QString("%1").arg(VERSION_STRING);
#ifdef Q_OS_WIN64
    version.append(u8" x64");
#elif defined(Q_OS_WIN32)
    version.append(u8" x86");
#endif
    auto showString = QString(u8"<h3>关于 UCI Encoder GUI</h3>"
                              u8"<p>版本：%1 </p><p>编译时间：%2 %3</p>"
                              u8"<p>版权所有 2018 <a href=\"https://shine5402.top/\">shine_5402</a></p>"
                              u8"<p>UCI Encoder GUI 是 UCI Encoder 的一个简单前端GUI。它可以让使用者更加轻松地应对使用 UCI Encoder 进行大量图片操作的情况。</p>"
                              u8"<p>Github 仓库地址：<a href=\"https://github.com/shine5402/UCIEncoderGUIQt\">https://github.com/shine5402/UCIEncoderGUIQt</a>"
                              u8"<p>贴吧地址：<a href=\"https://tieba.baidu.com/p/5703612353\">https://tieba.baidu.com/p/5703612353</a>"
                              u8"<p></br></p>"
                              u8"<p>本程序源代码以 GNU GPL v3 协议开源，您可以依据此协议的条款重新分发和/或修改它。</p>"
                              u8"<p>我们分发这个程序的目的是希望它有用，但是我们对它的可用性没有任何保证，甚至也没有对于适销性或适用于特定用途的默示保证。有关详细信息，请参阅GNU通用公共许可证。</p>"
                              u8"<p>您应该在收到本程序的同时就已经收到GNU通用公共许可证的副本。 如果没有，请参阅"
                              u8"<a href=\"http://www.gnu.org/licenses/\">http://www.gnu.org/licenses/</a>。</p>"
                              u8"<p>本程序的发行包中包含了以下开源软件：</p>"
                              u8"<ul>"
                              u8"<li>UCI Encoder (<a href=\"https://www.gnu.org/licenses/lgpl.html\">GNU LGPL v3</a>)</li>"
                              u8"<li>ImageMagick (<a href=\"https://www.imagemagick.org/script/license.php\">Apache 2.0 license</a>)</li>"
                              u8"<li>x264 (<a href=\"https://www.gnu.org/licenses/old-licenses/gpl-2.0.html\">GNU GPL v2</a>)</li>"
                              u8"<li>x265 (<a href=\"https://www.gnu.org/licenses/old-licenses/gpl-2.0.html\">GNU GPL v2</a>)</li>"
                              u8"</ul>"
                              u8"<p>此外，本程序的发行包中一些开源软件使用了其不同于官方的分支版本。您可以手动调用其帮助信息，或在贴吧页面中查看详情。</p>").arg(version).arg(__DATE__).arg(__TIME__);
    QMessageBox::about(this,u8"关于 UCI Encoder GUI",showString);
}

void MainWindow::hideAdvanced()
{
    ui->advancedParameterWidget->hide();
}
void MainWindow::showAdvanced()
{
    ui->advancedParameterWidget->show();
}

void MainWindow::on_showAdvancedButton_clicked()
{
    if (ui->advancedParameterWidget->isHidden())
    {
        ui->advancedParameterWidget->show();
        ui->showAdvancedButton->setText(u8"显示更少参数");
    }
    else
    {
        ui->advancedParameterWidget->hide();
        if (isAdvancedModified())
            ui->showAdvancedButton->setText(u8"显示更多参数（已修改）");
        else
            ui->showAdvancedButton->setText(u8"显示更多参数");
    }
}


void MainWindow::on_QcheckBox_stateChanged(int arg1)
{
    if (arg1 == Qt::CheckState::Checked){
        ui->CRFQWidget->setEnabled(true);
    }
    else
    {
        ui->CRFQWidget->setEnabled(false);
    }
}

void MainWindow::on_xCheckBox_stateChanged(int arg1)
{
    if (arg1 == Qt::CheckState::Checked){
        ui->xPushButton->setEnabled(true);
    }
    else
    {
        ui->xPushButton->setEnabled(false);
        xString.clear();
    }
}

void MainWindow::on_XCheckBox_stateChanged(int arg1)
{
    if (arg1 == Qt::CheckState::Checked){
        ui->XPushButton->setEnabled(true);
    }
    else
    {
        ui->XPushButton->setEnabled(false);
        XString.clear();
    }
}

void MainWindow::on_pCheckBox_stateChanged(int arg1)
{
    if (arg1 == Qt::CheckState::Checked){
        ui->pLineEdit->setEnabled(true);
    }
    else
    {
        ui->pLineEdit->setEnabled(false);
    }
}

void MainWindow::on_i420CheckBox_stateChanged(int arg1)
{
    if (arg1 == Qt::CheckState::Checked){
        ui->i420wLineEdit->setEnabled(true);
        ui->sCheckBox->setChecked(false);
        ui->o420CheckBox->setChecked(false);
        ui->o444CheckBox->setChecked(false);
        ui->sCheckBox->setEnabled(false);
        ui->o420CheckBox->setEnabled(false);
        ui->o444CheckBox->setEnabled(false);
    }
    else
    {
        ui->i420hLineEdit->setEnabled(false);
        ui->i420wLineEdit->setEnabled(false);
        ui->i420hLineEdit->clear();
        ui->i420wLineEdit->clear();
        ui->sCheckBox->setChecked(true);
        ui->o420CheckBox->setChecked(true);
        ui->o444CheckBox->setChecked(true);
        ui->sCheckBox->setEnabled(true);
        ui->o420CheckBox->setEnabled(true);
        ui->o444CheckBox->setEnabled(true);
    }
}



void MainWindow::on_i420wLineEdit_textChanged(const QString &arg1)
{
    if (arg1 != ""){
        ui->i420hLineEdit->setEnabled(true);
    } else {
        ui->i420hLineEdit->setEnabled(false);
    }
}

void MainWindow::on_xPushButton_clicked()
{
    xString = QInputDialog::getText(this,u8"输入自定义参数",u8"请输入UCI Encoder调用x264/5时应使用的参数：");
}

void MainWindow::on_XPushButton_clicked()
{
    XString = QInputDialog::getText(this,u8"输入自定义参数",u8"请输入UCI Encoder调用x264/5压制Alpha通道时应使用的参数：");
}

bool MainWindow::isAdvancedModified()
{
    return ui->o420CheckBox->isChecked() || ui->o444CheckBox->isChecked() ||ui->i420CheckBox->isChecked() ||ui->i444CheckBox->isChecked() ||ui->sCheckBox->isChecked() ||ui->XCheckBox->isChecked() ||ui->xCheckBox->isChecked() ||ui->quietCheckBox->isChecked() ||ui->o10bCheckBox->isChecked() ||ui->i10bCheckBox->isChecked() ||ui->QcheckBox->isChecked() ||ui->pCheckBox->isChecked() || ui->infoCheckBox->isChecked();
}
QString MainWindow::getAdvancedNativeArguments()
{
    QString result;
    if (ui->QcheckBox->isChecked())
        result.append(QString(" -Q %1").arg(CRF_Q));
    if (ui->xCheckBox->isChecked())
        result.append(QString(" -x \"%1\"").arg(xString));
    if (ui->XCheckBox->isChecked())
        result.append(QString(" -X \"%1\"").arg(xString));
    if (ui->pCheckBox->isChecked())
        result.append(QString(" -p %1").arg(ui->pLineEdit->text()));
    if (ui->sCheckBox->isChecked())
        result.append(u8" -s");
    if (ui->o444CheckBox->isChecked())
        result.append(u8" -o444");
    if (ui->o420CheckBox->isChecked())
        result.append(u8" -o420");
    if (ui->o10bCheckBox->isChecked())
        result.append(u8" -o10b");
    if (ui->i444CheckBox->isChecked())
        result.append(QString(u8" -i444 %1 %2").arg(ui->i444wLineEdit->text(),ui->i444hLineEdit->text()));
    if (ui->i420CheckBox->isChecked())
        result.append(QString(u8" -i420 %1 %2").arg(ui->i420wLineEdit->text(),ui->i420hLineEdit->text()));
    if (ui->i10bCheckBox->isChecked())
        result.append(u8" -i10b");
    if (ui->infoCheckBox->isChecked())
        result.append(u8" -info");
    if (ui->quietCheckBox->isChecked())
        result.append(u8" -quiet");
    return result;
}

void MainWindow::on_i444CheckBox_stateChanged(int arg1)
{
    if (arg1 == Qt::CheckState::Checked){
        ui->i444wLineEdit->setEnabled(true);
    }
    else
    {
        ui->i444hLineEdit->setEnabled(false);
        ui->i444wLineEdit->setEnabled(false);
        ui->i444hLineEdit->clear();
        ui->i444wLineEdit->clear();
    }
}
#ifdef Q_OS_WIN64
void MainWindow::toolChainBoxChanged(const QString &text)
{
    isToolchain_x64 = (text == "x64");
}
#endif
double MainWindow::calculateCRFFromPercent(int value,int percent_max){
    if (ui->x264radioButton->isChecked())
    {
        double value_d = value;
        if (value_d >=CRF_SLIDER_USEFULRANGE_MIN && value_d <=CRF_SLIDER_USEFULRANGE_MAX)
        {
            value_d = 0 + percent_max * (value_d - CRF_SLIDER_USEFULRANGE_MIN)/(CRF_SLIDER_USEFULRANGE_MAX-CRF_SLIDER_USEFULRANGE_MIN);
            return (CRF_X264_USEFULRANGE_MAX - CRF_X264_USEFULRANGE_MIN)*(1 - value_d/percent_max) + CRF_X264_USEFULRANGE_MIN;
        }
        else if (value_d < CRF_SLIDER_USEFULRANGE_MIN)
        {
            return CRF_MAX - value_d/(CRF_SLIDER_USEFULRANGE_MIN) * (CRF_MAX - CRF_X264_USEFULRANGE_MAX);
        }
        else {
            return (percent_max - value_d) / percent_max * (percent_max - CRF_SLIDER_USEFULRANGE_MAX) * (CRF_X264_USEFULRANGE_MIN - CRF_MIN) + CRF_MIN;
        }
    }
    else
    {
        double value_d = value;
        if (value_d >=CRF_SLIDER_USEFULRANGE_MIN && value_d <=CRF_SLIDER_USEFULRANGE_MAX)
        {
            value_d = 0 + percent_max * (value_d - CRF_SLIDER_USEFULRANGE_MIN)/(CRF_SLIDER_USEFULRANGE_MAX-CRF_SLIDER_USEFULRANGE_MIN);
            return (CRF_X265_USEFULRANGE_MAX - CRF_X265_USEFULRANGE_MIN)*(1 - value_d/percent_max) + CRF_X265_USEFULRANGE_MIN;
        }
        else if (value_d < CRF_SLIDER_USEFULRANGE_MIN)
        {
            return CRF_MAX - value_d/(CRF_SLIDER_USEFULRANGE_MIN) * (CRF_MAX - CRF_X265_USEFULRANGE_MAX);
        }
        else {
            return (percent_max - value_d) / percent_max * (percent_max - CRF_SLIDER_USEFULRANGE_MAX) * (CRF_X265_USEFULRANGE_MIN - CRF_MIN) + CRF_MIN;
        }
    }
}

int MainWindow::calculatePercentFromCRF(double CRF,int max){
    if (ui->x264radioButton->isChecked()){
        if (CRF >= CRF_X264_USEFULRANGE_MIN && CRF <= CRF_X264_USEFULRANGE_MAX){
            return static_cast<int>((CRF_X264_USEFULRANGE_MAX - CRF)/(CRF_X264_USEFULRANGE_MAX-CRF_X264_USEFULRANGE_MIN)*(CRF_SLIDER_USEFULRANGE_MAX - CRF_SLIDER_USEFULRANGE_MIN) + CRF_SLIDER_USEFULRANGE_MIN);
        }
        else if (CRF <= CRF_X264_USEFULRANGE_MIN){
            return static_cast<int>(max - (max - CRF_SLIDER_USEFULRANGE_MAX) * (CRF / (CRF_X264_USEFULRANGE_MIN - CRF_MIN)));
        }
        else {
            return static_cast<int>(CRF_SLIDER_USEFULRANGE_MIN - (CRF - CRF_X264_USEFULRANGE_MAX) / (CRF_MAX - CRF_X264_USEFULRANGE_MAX) * (CRF_SLIDER_USEFULRANGE_MIN - 0));
        }
    }
    else
    {
        if (CRF >= CRF_X265_USEFULRANGE_MIN && CRF <= CRF_X265_USEFULRANGE_MAX){
            return static_cast<int>((CRF_X265_USEFULRANGE_MAX - CRF)/(CRF_X265_USEFULRANGE_MAX-CRF_X265_USEFULRANGE_MIN)*(CRF_SLIDER_USEFULRANGE_MAX - CRF_SLIDER_USEFULRANGE_MIN) + CRF_SLIDER_USEFULRANGE_MIN);
        }
        else if (CRF <= CRF_X265_USEFULRANGE_MIN){
            return static_cast<int>(max - (max - CRF_SLIDER_USEFULRANGE_MAX) * (CRF / (CRF_X265_USEFULRANGE_MIN - CRF_MIN)));
        }
        else {
            return static_cast<int>(CRF_SLIDER_USEFULRANGE_MIN - (CRF - CRF_X265_USEFULRANGE_MAX) / (CRF_MAX - CRF_X265_USEFULRANGE_MAX) * (CRF_SLIDER_USEFULRANGE_MIN - 0));
        }
    }
}

void MainWindow::updateCRFFromSliderValue(int value)
{
    CRF = calculateCRFFromPercent(value,ui->CRFhorizontalSlider->maximum());
    ui->CRFLabel->setText(QString(u8"%1% (CRF:%2)").arg(value).arg(CRF));
}
void MainWindow::updateCRFQFromSliderValue(int value)
{
    CRF_Q = calculateCRFFromPercent(value,ui->CRFhorizontalSlider_Q->maximum());
    ui->CRFLabel_Q->setText(QString(u8"%1% (CRF:%2)").arg(value).arg(CRF_Q));
}

void MainWindow::updateCRFFromCRFValue(double CRF)
{
    this->CRF = CRF;
    //    ui->CRFhorizontalSlider->setValue(static_cast<int>(ui->CRFhorizontalSlider->maximum() - (CRF-CRF_MIN)/(CRF_MAX-CRF_MIN)*ui->CRFhorizontalSlider->maximum()));
    ui->CRFhorizontalSlider->setValue(calculatePercentFromCRF(CRF,ui->CRFhorizontalSlider->maximum()));
    ui->CRFLabel->setText(QString(u8"%1% (CRF:%2)").arg(ui->CRFhorizontalSlider->value()).arg(CRF));
}
void MainWindow::updateCRFQFromCRFValue(double CRF_Q)
{
    this->CRF_Q = CRF_Q;
    ui->CRFhorizontalSlider_Q->setValue(calculatePercentFromCRF(CRF_Q,ui->CRFhorizontalSlider_Q->maximum()));
    ui->CRFLabel->setText(QString(u8"%1% (CRF:%2)").arg(ui->CRFhorizontalSlider_Q->value()).arg(CRF_Q));
}

void MainWindow::on_CRFhorizontalSlider_valueChanged(int value)
{
    updateCRFFromSliderValue(value);
    if (!ui->QcheckBox->isChecked())
    {
        updateCRFQFromSliderValue(value);
        ui->CRFhorizontalSlider_Q->setValue(value);
    }
}

void MainWindow::on_CRFhorizontalSlider_Q_valueChanged(int value)
{
    updateCRFQFromSliderValue(value);
}

void MainWindow::on_CRFLabel_doubleClicked()
{
    bool ok = false;
    auto newCRF = QInputDialog::getDouble(this,u8"输入一个CRF值",u8"输入一个CRF值。范围为0.0~51.0。",CRF,CRF_MIN,CRF_MAX,2,&ok);
    if (ok)
        updateCRFFromCRFValue(newCRF);
}

void MainWindow::on_CRFLabel_Q_doubleClicked()
{
    bool ok = false;
    auto newCRF = QInputDialog::getDouble(this,u8"输入一个CRF值",u8"输入一个CRF值。范围为0.0~51.0。",CRF_Q,CRF_MIN,CRF_MAX,2,&ok);
    if (ok)
        updateCRFQFromCRFValue(newCRF);
}
