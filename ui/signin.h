#ifndef SIGNIN_H
#define SIGNIN_H

#include <QWidget>

namespace Ui {
class SignIn;
}

class SignIn : public QWidget
{
    Q_OBJECT

public:
    explicit SignIn(QWidget *parent = nullptr);
    ~SignIn();

private slots:
    void on_button_return_clicked();

    void on_button_signIn_clicked();

    void on_checkBox_showPwd_stateChanged(int arg1);

private:
    Ui::SignIn *ui;
};

#endif // SIGNIN_H
