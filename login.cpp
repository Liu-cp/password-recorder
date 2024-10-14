#include "login.h"
#include "ui_login.h"
#include "uimanager.h"
#include "database.h"
#include "uidatabaseset.h"
#include <QMessageBox>
#include <QSqlQuery>
#include <QDebug>

Login::Login(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Login)
{
    ui->setupUi(this);

    connect(ui->button_databaseSet, &QPushButton::clicked, []() {
        UiDatabaseSet *uiInstance = qobject_cast<UiDatabaseSet *>(UiManager::getInstance().getUiInstance(UiName::eUiDatabaseSet));
        uiInstance->on_ui_toBeShow();
    });
}

Login::~Login()
{
    delete ui;
}

void Login::on_checkBox_showPwd_stateChanged(int arg1)
{
    if ( arg1 == Qt::Checked ) {
        ui->lineEdit_password->setEchoMode(QLineEdit::Normal);
    }
    else {
        ui->lineEdit_password->setEchoMode(QLineEdit::Password);
    }
}


void Login::on_button_signIn_clicked()
{
    UiManager::getInstance().showUi(UiName::eUiSignIn);
}


void Login::on_button_login_clicked()
{
    if ( ui->lineEdit_username->text().isEmpty() || ui->lineEdit_password->text().isEmpty() ) {
        QMessageBox::warning(this, "登录异常", "请输入用户名和密码！");
        return ;
    }

    int ret = DataBase::getInstance().userLogin(ui->lineEdit_username->text(), ui->lineEdit_password->text());
    if ( ret != 0 ) {
        QString errorMsg;
        if ( ret == -1 ) errorMsg = "用户名不存在或密码错误，请重新输入！";
        else if ( ret == -2 ) errorMsg = "系统错误，请稍后重试！";
        QMessageBox::warning(this, "登录失败", errorMsg);
        return ;
    }

    // 切换到主界面
    qDebug() << "用户登录成功！！！";
    UiManager::getInstance().showUi(UiName::eUiMainWindow);
}


void Login::on_button_lossPwd_clicked()
{
    QMessageBox::information(nullptr, "系统信息", "功能不可用，请关注后续版本！");
}


void Login::on_button_databaseSet_clicked()
{
    UiManager::getInstance().showUi(UiName::eUiDatabaseSet);
}

void Login::on_ui_toBeShow()
{
    DBTable_DatabaseSet dbSet;
    if ( DataBase::getInstance().getRemoteDatabaseInfo(dbSet) ) {
        ui->label_showInfo->setText("已连接到远程数据库");
        ui->label_showInfo->setStyleSheet("");
    }
    else {
        ui->label_showInfo->setText("远程数据库未连接，将使用本地模式！");
        ui->label_showInfo->setStyleSheet("color: red;");
    }
}

