#ifndef WATERMARKDIALOG_H
#define WATERMARKDIALOG_H

#include <QDialog>

namespace Ui {
class WatermarkDialog;
}

class WatermarkDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WatermarkDialog(QWidget *parent = nullptr);
    ~WatermarkDialog();

    // 获取水印属性
    QString getWatermarkText() const;
    QColor getWatermarkColor() const;
    int getWatermarkFontSize() const;
    int getWatermarkAngle() const;
    int getWatermarkSpacing() const;

    // 设置水印属性
    void setWatermarkText(const QString &text);
    void setWatermarkColor(const QColor &color);
    void setWatermarkFontSize(int size);
    void setWatermarkAngle(int angle);
    void setWatermarkSpacing(int spacing);

private slots:
    void on_colorButton_clicked(); // 选择颜色

private:
    Ui::WatermarkDialog *ui;
};

#endif // WATERMARKDIALOG_H
