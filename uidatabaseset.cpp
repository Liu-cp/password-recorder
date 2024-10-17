#include "uidatabaseset.h"
#include "ui_uidatabaseset.h"
#include "uimanager.h"
#include "database.h"
// #ifdef Q_OS_ANDROID
// #include "jni_interface_mysql.h"
// #endif
#include <QMessageBox>

UiDatabaseSet::UiDatabaseSet(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::UiDatabaseSet)
{
    ui->setupUi(this);
}

UiDatabaseSet::~UiDatabaseSet()
{
    delete ui;
}

void UiDatabaseSet::on_checkBox_showPwd_stateChanged(int arg1)
{
    if ( arg1 == Qt::Checked ) ui->lineEdit_password->setEchoMode(QLineEdit::Normal);
    else ui->lineEdit_password->setEchoMode(QLineEdit::Password);
}


void UiDatabaseSet::on_button_return_clicked()
{
    UiManager::getInstance().showUi(UiName::eUiLogin);
}


void UiDatabaseSet::on_button_dbTest_clicked()
{
    DBTable_DatabaseSet dbSet(ui->lineEdit_hostName->text(), ui->lineEdit_hostPort->text().toUInt(), ui->lineEdit_dbName->text(), ui->lineEdit_username->text(), ui->lineEdit_password->text());
    QString info = DataBase::getInstance().remoteDatabaseConnectTest(dbSet);

    QMessageBox::information(nullptr, "系统信息", "远程数据库连接测试结果: " + info);
}


void UiDatabaseSet::on_button_save_clicked()
{
// #ifdef Q_OS_ANDROID
//     QMessageBox::information(nullptr, "系统信息", "功能不可用，请关注后续版本！");
//     return ;
// #endif

    DBTable_DatabaseSet dbSet(ui->lineEdit_hostName->text(), ui->lineEdit_hostPort->text().toUInt(), ui->lineEdit_dbName->text(), ui->lineEdit_username->text(), ui->lineEdit_password->text());
    QString info;

    int ret = DataBase::getInstance().mysqlDatabaseSet(dbSet);
    if ( ret == 0 ) {
        info = "远程数据库连接成功";
        ui->label_showInfo->setText("已连接到远程数据库" + dbSet.databaseName);
    }
    else {
        info = QString("远程数据库连接失败: %1").arg(ret);
        ui->label_showInfo->setText("远程数据库未连接");
    }

    QMessageBox::information(nullptr, "系统信息", info);
}

void UiDatabaseSet::on_ui_toBeShow()
{
    DBTable_DatabaseSet dbSet;
    if ( DataBase::getInstance().getRemoteDatabaseInfo(dbSet) ) {
        ui->label_showInfo->setText("已连接到远程数据库" + dbSet.databaseName);
        ui->lineEdit_hostPort->setText(QString::number(dbSet.hostPort));
    }
    else {
        ui->label_showInfo->setText("远程数据库未连接");
        ui->lineEdit_hostPort->setText("3306");
    }

    ui->lineEdit_hostName->setText(dbSet.hostName);
    ui->lineEdit_dbName->setText(dbSet.databaseName);
    // 数据库的账户和密码不显示
    ui->lineEdit_username->clear();
    ui->lineEdit_password->clear();
    ui->checkBox_showPwd->setCheckState(Qt::Unchecked);
}

