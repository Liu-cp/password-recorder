#include "uimainwindow.h"
#include "ui_uimainwindow.h"
#include "uipwddetail.h"
#include "common/uimanager.h"
#include "database/database.h"
#include <QLabel>
#include <QMessageBox>
#include <unordered_set>
#include <QTimer>
#include <QScroller>

UiMainWindow::UiMainWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::UiMainWindow)
    , m_searchDialogHided(false)
{
    ui->setupUi(this);
    ui->scrollArea_VBLayout->setAlignment(Qt::AlignTop);
    // 启用 QScroller 手势滚动
    QScroller::grabGesture(ui->scrollArea->viewport(), QScroller::TouchGesture);

    // 创建QDialog作为下拉列表
    m_searchDialog = new QDialog(this);
    m_searchListWidget = new QListWidget(m_searchDialog);
    // 设置QDialog的属性
    m_searchDialog->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
    m_searchDialog->setModal(false);           // 确保它不会成为模态窗口
    m_searchDialog->setLayout(new QVBoxLayout);
    m_searchDialog->layout()->addWidget(m_searchListWidget);
    m_searchDialog->layout()->setContentsMargins(0, 0, 0, 0);

    connect(m_searchListWidget, &QListWidget::itemClicked, this, &UiMainWindow::handleSearchListWidgetSelected);
    connect(this, &UiMainWindow::triggerHideSearchDialog, m_searchDialog, &QDialog::hide);
    connect(UiManager::getInstance().getStackedWidget(), &QStackedWidget::currentChanged, this, &UiMainWindow::handleStackWidgetCurrentChanged);
    // 滚动界面滚动后立即刷新（主要针对安卓端显示异常的问题）
    connect(QScroller::scroller(ui->scrollArea->viewport()), &QScroller::stateChanged, this, [](QScroller::State state) {
        if (state == QScroller::Inactive) {
            UiManager::getInstance().updateUi();
        }
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
            else if ( !m_searchText.isEmpty() ) {   // show details for search condition
                button->click();
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

            connect(button, &QPushButton::clicked, this, &UiMainWindow::handleLableButtonClicked);

            if ( !m_searchText.isEmpty() ) {    // show details for search condition
                button->click();
            }
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

    // 获取目标标签下密码记录名称
    QString label = m_ptrLBstrMap[button];
    QString filter;
    if ( !m_searchText.isEmpty() && !label.contains(m_searchText) ) {
        filter = m_searchText;
    }
    std::vector<QString> labelDetails = DataBase::getInstance().getPwdNamesByPwdType(label, filter);
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

        connect(bt, &QPushButton::clicked, this, [this, bt]() {
            emit this->showPwdDetailsSignal(UiPwdDetailShowType::eShowPwd, bt->text());

            UiManager::getInstance().showUi(UiName::eUiPwdDetail);
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

void UiMainWindow::handleLableButtonClicked()
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
    emit showPwdDetailsSignal(UiPwdDetailShowType::eCreatePwd);

    UiManager::getInstance().showUi(UiName::eUiPwdDetail);
}

void UiMainWindow::handleStackWidgetCurrentChanged(int index)
{
    if ( index != UiManager::getInstance().getUiIndex(UiName::eUiMainWindow) ) return ;

    // clear the search condition when ui to be showed
    m_searchText.clear();
    ui->lineEdit_search->clear();

    QStringList lables = DataBase::getInstance().getPwdTypes();
    qDebug() << "all pwdTypes size: " << lables.size();
    this->showScrollAreaContext(lables);
}


void UiMainWindow::on_button_search_clicked()
{
    m_searchDialog->hide();

    m_searchText = ui->lineEdit_search->text();
    QStringList lables = DataBase::getInstance().getPwdTypes(m_searchText);
    qDebug() << "all pwdTypes size: " << lables.size() << "filter: " << m_searchText;

    this->showScrollAreaContext(lables);

    UiManager::getInstance().updateUi();
}


void UiMainWindow::on_lineEdit_search_textEdited(const QString &arg1)
{
    if ( arg1.isEmpty() ) {
        m_searchDialog->hide();
    }
    else {
        QStringList lables = DataBase::getInstance().getSearchLabels(arg1);
        m_searchListWidget->clear();
        m_searchListWidget->addItems(lables);

        if ( lables.isEmpty() ) {
            m_searchDialog->hide();
        }
        else if ( !m_searchDialog->isVisible() || m_searchDialogHided ) {
            m_searchDialogHided = false;
            // 获取 QComboBox 在父窗口中的相对坐标
            QRect comboBoxGeometry = ui->lineEdit_search->geometry();
            QPoint comboBoxGlobalPos = this->mapToGlobal(comboBoxGeometry.topLeft()); // 转换为全局坐标

            // 计算 QDialog 应该显示的位置
            int dialogX = comboBoxGlobalPos.x(); // 水平位置与 QComboBox 左对齐
            int dialogY = comboBoxGlobalPos.y() + comboBoxGeometry.height(); // 垂直位置在 QComboBox 的下边缘

            // 设置 QDialog 的位置
            m_searchDialog->setFixedWidth(comboBoxGeometry.width()); // 将 QDialog 宽度设置为与 QComboBox 一致
            m_searchDialog->move(dialogX, dialogY);

            m_searchDialog->show();
            ui->lineEdit_search->setFocus();
        }
    }
}

void UiMainWindow::handleSearchListWidgetSelected(QListWidgetItem *item)
{
    ui->lineEdit_search->setText(item->text());

    // 移动 QDialog 到屏幕外代替隐藏      note: 其他方法如hide、close、done等相关操作都会导致安卓程序崩溃
    m_searchDialog->move(-5000, -5000);
    m_searchDialogHided = true;

    ui->lineEdit_search->setFocus();
}


void UiMainWindow::on_lineEdit_search_textChanged(const QString &arg1)
{
    UiManager::getInstance().updateUi();
}

