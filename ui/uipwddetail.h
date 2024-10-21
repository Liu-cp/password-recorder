#ifndef UIPWDDETAIL_H
#define UIPWDDETAIL_H

#include "database/database.h"
#include "custom_widget/customcombobox.h"
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

private slots:
    void on_button_return_clicked();
    void on_checkBox_showPwd_stateChanged(int arg1);
    void on_button_genPwd_clicked();
    void on_button_copyPwd_clicked();
    void on_button_pwdDelete_clicked();
    void on_button_pwdSave_clicked();

    void handleShowPwdDetailsSignal(UiPwdDetailShowType type, const QString pwdName = "");

private:
    Ui::UiPwdDetail *ui;
    UiPwdDetailShowType m_showType;
    DBTable_PwdRecorder m_originRecord;
    CustomComboBox *m_cComboBox_pwdType;
};

#endif // UIPWDDETAIL_H
