#ifndef UIPWDDETAIL_H
#define UIPWDDETAIL_H

#include "database.h"
#include <QWidget>

namespace Ui {
class UiPwdDetail;
}

enum UiPwdDetailShowType {
    eTypeNone,
    eCreatePwd,
    eShowPwd,
};

class UiPwdDetail : public QWidget
{
    Q_OBJECT

public:
    explicit UiPwdDetail(QWidget *parent = nullptr);
    ~UiPwdDetail();

private:
    QString generateRandomPassword();
    bool checkDataChanged();

public slots:
    void on_ui_toBeShow(UiPwdDetailShowType type, const QString pwdName = "");

private slots:
    void on_button_return_clicked();
    void on_checkBox_showPwd_stateChanged(int arg1);
    void on_button_genPwd_clicked();
    void on_button_copyPwd_clicked();
    void on_button_pwdDelete_clicked();
    void on_button_pwdSave_clicked();

private:
    Ui::UiPwdDetail *ui;
    UiPwdDetailShowType m_showType;
    DBTable_PwdRecorder m_originRecord;
};

#endif // UIPWDDETAIL_H
