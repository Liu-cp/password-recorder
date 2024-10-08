#include "uimanager.h"
#include "uimainwindow.h"
#include "login.h"
#include <QDebug>

UiManager::UiManager(QObject *parent)
    : QObject{parent}
{
    m_stackedWidget = new QStackedWidget();
    // m_stackedWidget->setAttribute(Qt::WA_OpaquePaintEvent, true);  // 禁用不透明事件
    // m_stackedWidget->setAttribute(Qt::WA_PaintOnScreen, true);     // 强制直接绘制到屏幕
}

void UiManager::addUi(UiName name, QWidget *ui) {
    m_stackedWidget->addWidget(ui);
    m_uiNameMap[name] = m_stackedWidget->indexOf(ui);

    switch ( name ) {
    case UiName::eUiLogin:
        connect(m_stackedWidget, &QStackedWidget::currentChanged, [&](int index) {
            if ( index == m_uiNameMap[UiName::eUiLogin] ) {
                Login *uiLogin = qobject_cast<Login *>(getUiInstance(UiName::eUiLogin));
                uiLogin->on_ui_toBeShow();
            }
        });
        break;
    case UiName::eUiMainWindow:
        // exchange to mainWindow trigger to show
        connect(m_stackedWidget, &QStackedWidget::currentChanged, [&](int index) {
            if ( index == m_uiNameMap[UiName::eUiMainWindow] ) {
                UiMainWindow *uiMain = qobject_cast<UiMainWindow *>(getUiInstance(UiName::eUiMainWindow));
                uiMain->on_ui_toBeShow();
            }
        });
        break;
    default:
        break;
    }
}
