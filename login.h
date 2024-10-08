#ifndef LOGIN_H
#define LOGIN_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class Login;
}
QT_END_NAMESPACE

class Login : public QWidget
{
    Q_OBJECT

public:
    Login(QWidget *parent = nullptr);
    ~Login();

private slots:
    void on_checkBox_showPwd_stateChanged(int arg1);
    void on_button_signIn_clicked();
    void on_button_login_clicked();
    void on_button_lossPwd_clicked();
    void on_button_databaseSet_clicked();

public slots:
    void on_ui_toBeShow();

private:
    Ui::Login *ui;
};
#endif // LOGIN_H
