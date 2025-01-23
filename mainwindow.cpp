#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "watermarkdialog.h" // 包含水印对话框头文件
#include <QFileDialog>
#include <QDir>
#include <QMessageBox>
#include <QImage>
#include <QPainter>
#include <QFontMetrics>
#include <QDateTime>
#include <QtConcurrent/QtConcurrent>
#include <QListWidgetItem>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 初始化默认值
    ui->watermarkLabel->setText("未设置");
    ui->useWatermarkCheckBox->setChecked(false); // 默认不启用水印

    // 应用样式表
    applyStyleSheet();

    // 连接信号槽
    connect(this, &MainWindow::progressUpdated, this, [this](int current, int total) {
        ui->progressBar->setValue(current);
        ui->progressBar->setMaximum(total);
        ui->progressBar->setFormat(QString("%1/%2 (%3%)").arg(current).arg(total).arg(current * 100 / total));
    });

    // 监控生成任务
    connect(&watcher, &QFutureWatcher<void>::finished, this, [this]() {
        setControlsEnabled(true); // 任务完成后启用控件
        QMessageBox::information(this, "完成", "所有 PDF 文件已导出为图片！");
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_editWatermarkButton_clicked()
{
    WatermarkDialog dialog(this);
    dialog.setWatermarkText(watermarkText);
    dialog.setWatermarkColor(watermarkColor);
    dialog.setWatermarkFontSize(watermarkFontSize);
    dialog.setWatermarkAngle(watermarkAngle);
    dialog.setWatermarkSpacing(watermarkSpacing);

    if (dialog.exec() == QDialog::Accepted) {
        watermarkText = dialog.getWatermarkText();
        watermarkColor = dialog.getWatermarkColor();
        watermarkFontSize = dialog.getWatermarkFontSize();
        watermarkAngle = dialog.getWatermarkAngle();
        watermarkSpacing = dialog.getWatermarkSpacing();

        ui->watermarkLabel->setText(watermarkText.isEmpty() ? "未设置" : watermarkText);
    }
}

void MainWindow::on_importButton_clicked()
{
    pdfFiles = QFileDialog::getOpenFileNames(this, "选择 PDF 文件", "", "PDF 文件 (*.pdf)");
    if (!pdfFiles.isEmpty()) {
        ui->pdfLabel->setText(QString("已选择 %1 个文件").arg(pdfFiles.size()));

        // 如果未选择生成目录，设置默认目录并显示
        if (outputDir.isEmpty()) {
            setDefaultOutputDir();
        }

        // 更新 PDF 文件列表
        updatePdfList();
    }
}

void MainWindow::on_chooseOutputDir_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, "选择生成目录", outputDir);
    if (!dir.isEmpty()) {
        outputDir = dir;
        ui->outputDirLabel->setText(outputDir);
    }
}

void MainWindow::on_generateButton_clicked()
{
    if (pdfFiles.isEmpty()) {
        QMessageBox::warning(this, "错误", "请先导入 PDF 文件！");
        return;
    }

    // 如果未选择生成目录，设置默认目录
    if (outputDir.isEmpty()) {
        setDefaultOutputDir();
    }

    // 创建生成目录
    QDir dir(outputDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    // 重置进度条
    ui->progressBar->setValue(0);
    ui->progressBar->setMaximum(pdfFiles.size());

    // 禁用控件
    setControlsEnabled(false);

    // 使用 QtConcurrent 在后台线程中执行生成任务
    QFuture<void> future = QtConcurrent::run([this]() {
        for (int i = 0; i < pdfFiles.size(); ++i) {
            if (watcher.isCanceled()) break; // 检查是否取消
            exportPdfToImages(pdfFiles[i]);
            emit progressUpdated(i + 1, pdfFiles.size()); // 发射进度更新信号
        }
    });
    watcher.setFuture(future);
}

void MainWindow::on_cancelButton_clicked()
{
    watcher.cancel(); // 取消生成任务
    setControlsEnabled(true); // 启用控件
    QMessageBox::information(this, "提示", "生成任务已终止！");
}

void MainWindow::exportPdfToImages(const QString &filePath)
{
    // 加载 PDF 文件
    std::unique_ptr<Poppler::Document> document = Poppler::Document::load(filePath);
    if (!document || document->isLocked()) {
        QMessageBox::warning(this, "错误", "无法打开 PDF 文件：" + filePath);
        return;
    }

    // 设置渲染选项
    document->setRenderHint(Poppler::Document::Antialiasing, true);
    document->setRenderHint(Poppler::Document::TextAntialiasing, true);

    // 获取 PDF 文件名（不含扩展名）
    QFileInfo fileInfo(filePath);
    QString baseName = fileInfo.baseName();

    // 遍历每一页并导出为图片
    for (int i = 0; i < document->numPages(); ++i) {
        if (watcher.isCanceled()) break; // 检查是否取消
        std::unique_ptr<Poppler::Page> page(document->page(i));
        if (!page) {
            continue;
        }

        // 渲染页面为图片
        QImage image = page->renderToImage(300, 300); // 300 DPI
        if (image.isNull()) {
            continue;
        }

        // 如果启用水印，添加水印
        if (ui->useWatermarkCheckBox->isChecked() && !watermarkText.isEmpty()) {
            addWatermark(image);
        }

        // 保存图片
        QString imageName = QString("%1/%2_%3.png").arg(outputDir).arg(baseName).arg(i + 1);
        if (!image.save(imageName)) {
            QMessageBox::warning(this, "错误", QString("无法保存图片：%1").arg(imageName));
        }
    }
}

void MainWindow::addWatermark(QImage &image)
{
    QPainter painter(&image);

    // 基准尺寸（A4 纸张尺寸：2481 × 3508 像素）
    const int baseWidth = 2481;
    const int baseHeight = 3508;

    // 计算当前图片与基准尺寸的比例
    double widthRatio = static_cast<double>(image.width()) / baseWidth;
    double heightRatio = static_cast<double>(image.height()) / baseHeight;
    double scaleFactor = qMax(widthRatio, heightRatio);

    // 动态调整字体大小和间距
    int fontSize = static_cast<int>(watermarkFontSize * scaleFactor);
    int spacing = static_cast<int>(watermarkSpacing * scaleFactor);

    // 设置水印字体
    QFont font;
    font.setPointSize(fontSize); // 动态字体大小
    font.setBold(true);          // 加粗
    painter.setFont(font);

    // 设置水印颜色和透明度
    painter.setPen(watermarkColor);

    // 计算水印文字的宽度和高度
    QFontMetrics metrics(font);
    int textWidth = metrics.horizontalAdvance(watermarkText);
    int textHeight = metrics.height();

    // 设置水印的旋转角度
    painter.save();
    painter.translate(image.width() / 2, image.height() / 2);
    painter.rotate(watermarkAngle); // 使用可编辑的倾斜角度

    // 计算水印的绘制区域
    for (int y = -image.height(); y < image.height(); y += spacing) {
        for (int x = -image.width(); x < image.width(); x += textWidth + spacing) {
            painter.drawText(x, y, watermarkText);
        }
    }

    painter.restore();
}

void MainWindow::updatePdfList()
{
    ui->pdfListWidget->clear();
    for (const QString &filePath : pdfFiles) {
        QFileInfo fileInfo(filePath);
        QString fileName = fileInfo.fileName(); // 只显示文件名

        QListWidgetItem *item = new QListWidgetItem();
        item->setSizeHint(QSize(0, 40)); // 设置列表项高度为 40 像素

        QWidget *widget = new QWidget();
        QHBoxLayout *layout = new QHBoxLayout(widget);
        QLabel *label = new QLabel(fileName);
        QPushButton *deleteButton = new QPushButton("✖"); // 使用 Unicode 字符作为删除按钮

        // 设置删除按钮样式
        deleteButton->setStyleSheet(
            "QPushButton {"
            "   color: red;"
            "   font-size: 16px;"
            "   border: none;"
            "   background: transparent;"
            "}"
            "QPushButton:hover {"
            "   color: darkred;"
            "}"
            );
        deleteButton->setFixedSize(24, 24); // 设置按钮大小

        layout->addWidget(label);
        layout->addWidget(deleteButton);
        layout->setContentsMargins(10, 0, 10, 0); // 设置边距
        layout->setSpacing(10); // 设置间距
        widget->setLayout(layout);

        // 连接删除按钮的点击事件
        connect(deleteButton, &QPushButton::clicked, this, [this, filePath]() {
            pdfFiles.removeOne(filePath);
            updatePdfList();
            ui->pdfLabel->setText(QString("已选择 %1 个文件").arg(pdfFiles.size()));
        });

        ui->pdfListWidget->addItem(item);
        ui->pdfListWidget->setItemWidget(item, widget);
    }
}

void MainWindow::setDefaultOutputDir()
{
    QFileInfo fileInfo(pdfFiles.first());
    outputDir = fileInfo.dir().path() + "/output_" + QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    ui->outputDirLabel->setText(outputDir);
}

void MainWindow::applyStyleSheet()
{
    // 设置全局样式表
    QString styleSheet = R"(
        QMainWindow {
            background-color: #F5F5F5;
        }
        QPushButton {
            background-color: #4CAF50;
            color: white;
            border: none;
            padding: 8px 16px;
            font-size: 14px;
            border-radius: 4px;
        }
        QPushButton:hover {
            background-color: #45a049;
        }
        QLabel {
            font-size: 14px;
            color: #333;
        }
        QProgressBar {
            border: 1px solid #ccc;
            border-radius: 4px;
            text-align: center;
            background-color: #fff;
        }
        QProgressBar::chunk {
            background-color: #4CAF50;
        }
        QListWidget {
            background-color: #fff;
            border: 1px solid #ccc;
            border-radius: 5px;
            padding: 5px;
        }
        QListWidget::item {
            height: 40px;
            border-bottom: 1px solid #eee;
        }
        QListWidget::item:hover {
            background-color: #f5f5f5;
        }
    )";
    this->setStyleSheet(styleSheet);
}

void MainWindow::setControlsEnabled(bool enabled)
{
    ui->editWatermarkButton->setEnabled(enabled);
    ui->importButton->setEnabled(enabled);
    ui->chooseOutputDir->setEnabled(enabled);
    ui->generateButton->setEnabled(enabled);
    ui->useWatermarkCheckBox->setEnabled(enabled);
    ui->cancelButton->setEnabled(!enabled); // 终止按钮在生成任务期间启用
}
