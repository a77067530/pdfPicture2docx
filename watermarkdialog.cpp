#include "watermarkdialog.h"
#include "ui_watermarkdialog.h"
#include <QColorDialog>

WatermarkDialog::WatermarkDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::WatermarkDialog)
{
    ui->setupUi(this);
    setWindowTitle("编辑水印");

    // 连接 QDialogButtonBox 的信号槽
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

WatermarkDialog::~WatermarkDialog()
{
    delete ui;
}

QString WatermarkDialog::getWatermarkText() const
{
    return ui->watermarkTextEdit->text();
}

QColor WatermarkDialog::getWatermarkColor() const
{
    return ui->colorLabel->palette().color(QPalette::Window);
}

int WatermarkDialog::getWatermarkFontSize() const
{
    return ui->fontSizeSpinBox->value();
}

int WatermarkDialog::getWatermarkAngle() const
{
    return ui->angleSpinBox->value();
}

int WatermarkDialog::getWatermarkSpacing() const
{
    return ui->spacingSpinBox->value();
}

void WatermarkDialog::setWatermarkText(const QString &text)
{
    ui->watermarkTextEdit->setText(text);
}

void WatermarkDialog::setWatermarkColor(const QColor &color)
{
    QPalette palette = ui->colorLabel->palette();
    palette.setColor(QPalette::Window, color);
    ui->colorLabel->setPalette(palette);
    ui->colorLabel->setAutoFillBackground(true);
    ui->colorLabel->setText(color.name());
}

void WatermarkDialog::setWatermarkFontSize(int size)
{
    ui->fontSizeSpinBox->setValue(size);
}

void WatermarkDialog::setWatermarkAngle(int angle)
{
    ui->angleSpinBox->setValue(angle);
}

void WatermarkDialog::setWatermarkSpacing(int spacing)
{
    ui->spacingSpinBox->setValue(spacing);
}

void WatermarkDialog::on_colorButton_clicked()
{
    QColor color = QColorDialog::getColor(getWatermarkColor(), this, "选择水印颜色");
    if (color.isValid()) {
        setWatermarkColor(color);
    }
}
