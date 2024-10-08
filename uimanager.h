#ifndef UIMANAGER_H
#define UIMANAGER_H

#include <QApplication>
#include <QObject>
#include <QStackedWidget>
#include <unordered_map>
#include <QDebug>

enum UiName {
    eUiLogin,
    eUiSignIn,
    eUiMainWindow,
    eUiPwdDetail,
    eUiDatabaseSet,
};

class UiManager : public QObject
{
    Q_OBJECT
private:
    QStackedWidget *m_stackedWidget;
    std::unordered_map<UiName, int> m_uiNameMap;

private:
    explicit UiManager(QObject *parent = nullptr);
    UiManager(const UiManager&) = delete;
    UiManager& operator=(const UiManager&) = delete;

public:
    // 单例模式
    static UiManager& getInstance() {
        static UiManager uiManager;
        return uiManager;
    }
    QStackedWidget *getStackedWidget() {
        return m_stackedWidget;
    }
    /*获取指定ui的ui实例*/
    QWidget *getUiInstance(UiName uiName) {
        return m_stackedWidget->widget(m_uiNameMap[uiName]);
    }
    void addUi(UiName name, QWidget* ui);
    void showUi(UiName name) {
        m_stackedWidget->setCurrentIndex(m_uiNameMap[name]);
        updateUi();
    }
    void updateUi() {
        m_stackedWidget->update();
        m_stackedWidget->window()->update();    // 强制刷新整个窗口
    }

// signals:
};

#endif // UIMANAGER_H
