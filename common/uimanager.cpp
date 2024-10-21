#include "uimanager.h"
#include <QDebug>

UiManager::UiManager(QObject *parent)
    : QObject{parent}
{
    m_stackedWidget = new QStackedWidget();
}

void UiManager::addUi(UiName name, QWidget *ui) {
    m_stackedWidget->addWidget(ui);
    m_uiNameMap[name] = m_stackedWidget->indexOf(ui);
}
