#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStringList>
#include <poppler-qt6.h>
#include <memory>
#include <QFutureWatcher>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_editWatermarkButton_clicked(); // 编辑水印
    void on_importButton_clicked();       // 导入 PDF
    void on_chooseOutputDir_clicked();    // 选择生成目录
    void on_generateButton_clicked();     // 生成水印图片
    void on_cancelButton_clicked();       // 终止生成任务

signals:
    void progressUpdated(int current, int total); // 进度更新信号

private:
    Ui::MainWindow *ui;
    QStringList pdfFiles; // 保存导入的 PDF 文件路径
    QString outputDir;    // 保存生成目录
    QString watermarkText; // 水印文字（默认为空）
    QColor watermarkColor = QColor(255, 100, 100, 150); // 深红色，透明度 150
    int watermarkSpacing = 260; // 水印行间距
    int watermarkAngle = -45;   // 水印倾斜角度
    int watermarkFontSize = 70; // 水印字体大小
    bool useWatermark = false;  // 是否使用水印
    QFutureWatcher<void> watcher; // 用于监控生成任务

    void exportPdfToImages(const QString &filePath); // 导出 PDF 图片
    void addWatermark(QImage &image); // 添加水印
    void updatePdfList(); // 更新 PDF 文件列表
    void setDefaultOutputDir(); // 设置默认生成目录
    void applyStyleSheet(); // 应用样式表
    void setControlsEnabled(bool enabled); // 启用/禁用控件
};

#endif // MAINWINDOW_H
