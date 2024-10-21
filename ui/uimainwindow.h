#ifndef UIMAINWINDOW_H
#define UIMAINWINDOW_H

#include "uipwddetail.h"
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <memory>
#include <QHBoxLayout>

namespace Ui {
class UiMainWindow;
}

class UiMainWindow : public QWidget
{
    Q_OBJECT

private:
    struct LabelButton {
        QPushButton *button;
        QLabel *mainLabel;      // 主要显示的内容
        QLabel *symbolLabel;    // 符号标签“>>”、“v”
        LabelButton(QPushButton *b, QLabel *ml, QLabel *sl) : button(b), mainLabel(ml), symbolLabel(sl) {}
    };

public:
    explicit UiMainWindow(QWidget *parent = nullptr);
    ~UiMainWindow();

private:
    void showScrollAreaContext(QStringList &pwdLabels);
    void showLabelDetails(QPushButton *button);
    void closeLabelDetails(QPushButton *button);

private slots:
    void on_button_addNewPwd_clicked();
    void on_button_search_clicked();

    void handleLableButtonClicked();
    void handleStackWidgetCurrentChanged(int index);

signals:
    void showPwdDetailsSignal(UiPwdDetailShowType type, const QString pwdName = "");

private:
    const QString SYMBOL_LABEL_1 = ">> ";
    const QString SYMBOL_LABEL_2 = "v   ";

    Ui::UiMainWindow *ui;
    std::unordered_map<QString, std::shared_ptr<LabelButton>> m_strLBMap;
    std::unordered_map<QPushButton *, QString> m_ptrLBstrMap;
    std::unordered_map<QPushButton *, QWidget *> m_buttonChildWidgetMap;    // 主标签按钮对应的需要展开的子显示页，用来回收时释放
};

#endif // UIMAINWINDOW_H
