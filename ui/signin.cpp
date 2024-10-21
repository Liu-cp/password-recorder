#include "signin.h"
#include "ui_signin.h"
#include "common/uimanager.h"
#include "database/database.h"
#include <QMessageBox>

SignIn::SignIn(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SignIn)
{
    ui->setupUi(this);
}

SignIn::~SignIn()
{
    delete ui;
}

void SignIn::on_button_return_clicked()
{
    UiManager::getInstance().showUi(UiName::eUiLogin);
}


void SignIn::on_button_signIn_clicked()
{
    if ( ui->lineEdit_username->text().isEmpty() || ui->lineEdit_password->text().isEmpty() || ui->lineEdit_passwordRetry->text().isEmpty() ) {
        QMessageBox::warning(this, "注册失败", "请完整输入全部选项后进行注册！");
        return ;
    }
    if ( ui->lineEdit_password->text() != ui->lineEdit_passwordRetry->text() ) {
        QMessageBox::warning(this, "注册失败", "两次密码输入不一致，请检查后重试！");
        return ;
    }
    QString username = ui->lineEdit_username->text();
    if ( username[0].isDigit() ) {
        QMessageBox::warning(this, "注册失败", "用户名不能以数字开头，请修改后重试！");
        return ;
    }

    int ret = DataBase::getInstance().userSignIn(username, ui->lineEdit_password->text());
    if ( ret != 0 ) {
        QString errorMsg;
        if ( ret == -1 ) errorMsg = "系统错误，请重试！";
        else if ( ret == -2 ) errorMsg = "用户已存在，请返回登录或注册新用户！";
        QMessageBox::warning(this, "注册失败", errorMsg);
        return ;
    }

    // 自动返回登录界面
    QMessageBox::information(this, "注册成功", "注册成功，请登录！");
    UiManager::getInstance().showUi(UiName::eUiLogin);
}


void SignIn::on_checkBox_showPwd_stateChanged(int arg1)
{
    if ( arg1 == Qt::Checked ) {
        ui->lineEdit_password->setEchoMode(QLineEdit::Normal);
        ui->lineEdit_passwordRetry->setEchoMode(QLineEdit::Normal);
    }
    else {
        ui->lineEdit_password->setEchoMode(QLineEdit::Password);
        ui->lineEdit_passwordRetry->setEchoMode(QLineEdit::Password);
    }
}

