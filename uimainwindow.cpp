#include "uimainwindow.h"
#include "ui_uimainwindow.h"
#include "uipwddetail.h"
#include "uimanager.h"
#include "database.h"
#include <QLabel>
#include <QMessageBox>
#include <unordered_set>
#include <QTimer>

UiMainWindow::UiMainWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::UiMainWindow)
{
    ui->setupUi(this);
    ui->scrollArea_VBLayout->setAlignment(Qt::AlignTop);
    connect(ui->button_addNewPwd, &QPushButton::clicked, []() {
        UiPwdDetail *uiInstance = qobject_cast<UiPwdDetail *>(UiManager::getInstance().getUiInstance(UiName::eUiPwdDetail));
        uiInstance->on_ui_toBeShow(UiPwdDetailShowType::eCreatePwd);
    });
}

UiMainWindow::~UiMainWindow()
{
    delete ui;
}

void UiMainWindow::showScrollAreaContext(QStringList &pwdLabels)
{
    std::unordered_set<QString> labelsSet;

    for ( auto &label : pwdLabels ) {
        labelsSet.insert(label);

        // 判断标签按钮是否已经存在
        if ( m_strLBMap.find(label) != m_strLBMap.end() ) {
            // 检查子页面是否存在并刷新
            QPushButton *button = m_strLBMap[label]->button;
            if ( m_buttonChildWidgetMap.find(button) != m_buttonChildWidgetMap.end() ) {
                // 关闭之前的子页面并重新加载
                closeLabelDetails(button);
                showLabelDetails(button);
            }
        }
        else {
            QPushButton *button = new QPushButton(ui->scrollAreaWidgetContents);
            // 创建水平布局
            QHBoxLayout *layout = new QHBoxLayout();
            // 创建主要文字标签
            QLabel *mainText = new QLabel(label, button);
            layout->addWidget(mainText);
            // 创建符号标签
            QLabel *symbol = new QLabel(SYMBOL_LABEL_1, button);
            layout->addWidget(symbol);
            // 设置布局间距和对齐方式
            layout->setContentsMargins(0, 0, 0, 0);
            layout->setAlignment(symbol, Qt::AlignRight);

            button->setLayout(layout);
            ui->scrollArea_VBLayout->addWidget(button);

            // 记录button及下属控件信息
            m_strLBMap[label] = std::make_shared<LabelButton>(button, mainText, symbol);
            m_ptrLBstrMap[button] = label;

            connect(button, &QPushButton::clicked, this, &UiMainWindow::on_lableButton_clicked);
        }
    }

    // 检查是否有失效的标签需要删除
    for ( auto it = m_strLBMap.begin(); it != m_strLBMap.end(); ) {
        QString label = it->first;
        if ( labelsSet.find(label) == labelsSet.end() ) {
            QPushButton *button = m_strLBMap[label]->button;
            closeLabelDetails(button);

            m_ptrLBstrMap.erase(button);
            delete button;
            it = m_strLBMap.erase(it);
        }
        else ++it;
    }
}

void UiMainWindow::showLabelDetails(QPushButton *button)
{
    int index = ui->scrollArea_VBLayout->indexOf(button);
    if ( index == -1 ) {
        QMessageBox::warning(this, "系统信息", "系统错误，请稍后重试!");
        return ;
    }

    // 获取目标标签下所有密码记录
    QString label = m_ptrLBstrMap[button];
    std::vector<QString> labelDetails = DataBase::getInstance().getPwdNamesByPwdType(label);
    if ( labelDetails.empty() ) {
        qDebug() << "labelDetails empty";
        return;
    }

    // 在目标按钮后面添加一个垂直布局器，用来显示标签对应的具体密码记录项
    QWidget *detailWidget = new QWidget();
    m_buttonChildWidgetMap[button] = detailWidget;
    ui->scrollArea_VBLayout->insertWidget(index+1, detailWidget);
    QHBoxLayout *hLayout = new QHBoxLayout(detailWidget);

    hLayout->setContentsMargins(0,0,0,0);
    QSpacerItem *spacer = new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);
    hLayout->addItem(spacer); // 添加水平间隔器

    QVBoxLayout *vLayout = new QVBoxLayout();
    hLayout->addLayout(vLayout);

    vLayout->setContentsMargins(0,0,0,0);
    vLayout->setSpacing(3);
    vLayout->setAlignment(Qt::AlignTop);

    // 在垂直布局器中添加具体的项
    for ( auto &pwdDetail : labelDetails ) {
        QPushButton *bt = new QPushButton(pwdDetail);
        bt->setStyleSheet("text-align: left;");     // 文字靠左显示
        vLayout->addWidget(bt);

        connect(bt, &QPushButton::clicked, [bt]() {
            UiManager::getInstance().showUi(UiName::eUiPwdDetail);
            UiPwdDetail *uiInstance = qobject_cast<UiPwdDetail *>(UiManager::getInstance().getUiInstance(UiName::eUiPwdDetail));
            uiInstance->on_ui_toBeShow(UiPwdDetailShowType::eShowPwd, bt->text());
        });
    }
}

void UiMainWindow::closeLabelDetails(QPushButton *button)
{
    if ( button == nullptr ) return ;

    if ( m_buttonChildWidgetMap.find(button)==m_buttonChildWidgetMap.end() ) {
        qDebug() << "detail widget is not found, label: " << m_strLBMap[m_ptrLBstrMap[button]]->mainLabel->text();
        return ;
    }
    QWidget *detailWidget = m_buttonChildWidgetMap[button];
    if ( detailWidget ) {
        ui->scrollArea_VBLayout->removeWidget(detailWidget);
        delete detailWidget;
        detailWidget = nullptr;
    }
    else qDebug() << "detailWidget is error null for label: " << m_strLBMap[m_ptrLBstrMap[button]]->mainLabel->text();

    m_buttonChildWidgetMap.erase(button);
}

void UiMainWindow::on_lableButton_clicked()
{
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    if ( !button ) {
        QMessageBox::warning(this, "系统信息", "系统错误，请稍后重试!");
        return ;
    }

    auto symbolLabel = m_strLBMap[m_ptrLBstrMap[button]]->symbolLabel;
    if ( !symbolLabel ) {
        QMessageBox::warning(this, "系统信息", "系统错误，请稍后重试!");
        return ;
    }

    if ( symbolLabel->text() == SYMBOL_LABEL_1 ) {
        symbolLabel->setText(SYMBOL_LABEL_2);
        showLabelDetails(button);
    }
    else {
        symbolLabel->setText(SYMBOL_LABEL_1);
        closeLabelDetails(button);
    }

    QTimer::singleShot(100, this, [=]() {
        UiManager::getInstance().updateUi();
    });
}

void UiMainWindow::on_button_addNewPwd_clicked()
{
    UiManager::getInstance().showUi(UiName::eUiPwdDetail);
}

void UiMainWindow::on_ui_toBeShow()
{
    QStringList lables = DataBase::getInstance().getAllPwdTypes();
    qDebug() << "all pwdTypes size: " << lables.size();
    showScrollAreaContext(lables);
}


void UiMainWindow::on_button_search_clicked()
{
    QMessageBox::information(nullptr, "系统信息", "功能不可用，请关注后续版本！");
}

