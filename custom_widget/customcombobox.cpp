#include "customcombobox.h"
#include <QVBoxLayout>
#include <QLineEdit>
#include <QTimer>

CustomComboBox::CustomComboBox(QWidget *parent)
    : QComboBox(parent)
{
    this->setStyleSheet("border: 1px solid gray; border-radius: 10px; padding: 5px;");
    this->setEditable(true);

    // 创建QDialog作为下拉列表
    listDialog = new QDialog(this);
    listWidget = new QListWidget(listDialog);
    // 设置QDialog的属性
    listDialog->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
    listDialog->setLayout(new QVBoxLayout);
    listDialog->layout()->addWidget(listWidget);
    listDialog->layout()->setContentsMargins(0, 0, 0, 0);

    connect(listWidget, &QListWidget::itemClicked, [&](QListWidgetItem *item) {
        this->setEditable(false);
        this->setText(item->text());

        listDialog->hide();

        this->setEditable(true);
    });
}

void CustomComboBox::addItems(const QStringList &texts) {
    listWidget->addItems(texts);
}

void CustomComboBox::clearItems() {
    this->listWidget->clear();
}

void CustomComboBox::setText(const QString &text) {
    this->clear();
    this->addItem(text);
    this->setCurrentIndex(0);
}

QString CustomComboBox::text() const {
    return this->currentText();
}

void CustomComboBox::showPopup()
{
    // 获取 QComboBox 在父窗口中的相对坐标
    QRect comboBoxGeometry = this->rect(); // 使用 rect() 获取 QComboBox 自身的矩形区域
    QPoint comboBoxGlobalPos = this->mapToGlobal(comboBoxGeometry.topLeft()); // 转换为全局坐标

    // 计算 QDialog 应该显示的位置
    int dialogX = comboBoxGlobalPos.x(); // 水平位置与 QComboBox 左对齐
    int dialogY = comboBoxGlobalPos.y() + comboBoxGeometry.height(); // 垂直位置在 QComboBox 的下边缘

    // 设置 QDialog 的位置
    listDialog->setFixedWidth(comboBoxGeometry.width()); // 将 QDialog 宽度设置为与 QComboBox 一致
    listDialog->move(dialogX, dialogY);

    listDialog->show();
}

void CustomComboBox::hidePopup()
{
    listDialog->hide();
}
