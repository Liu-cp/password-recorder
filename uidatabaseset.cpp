#include "uidatabaseset.h"
#include "ui_uidatabaseset.h"
#include "uimanager.h"
#include "database.h"
#ifdef Q_OS_ANDROID
#include "jni_interface_mysql.h"
#endif
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
    QString info;

    int ret = DataBase::getInstance().remoteDatabaseConnectTest(dbSet);
    if ( ret == 0 ) info = "远程数据库连接测试成功";
    else info = QString("远程数据库连接测试失败: %1").arg(ret);

    QMessageBox::information(nullptr, "系统信息", info);
}


void UiDatabaseSet::on_button_save_clicked()
{
    DBTable_DatabaseSet dbSet(ui->lineEdit_hostName->text(), ui->lineEdit_hostPort->text().toUInt(), ui->lineEdit_dbName->text(), ui->lineEdit_username->text(), ui->lineEdit_password->text());
    QString info;

    int ret = DataBase::getInstance().mysqlDatabaseSet(dbSet);
    if ( ret == 0 ) {
        info = "远程数据库连接成功";
        ui->label_showInfo->setText("已连接到远程数据库");
    }
    else {
        info = QString("远程数据库连接失败: %1").arg(ret);
        ui->label_showInfo->setText("远程数据库未连接");
    }

    QMessageBox::information(nullptr, "系统信息", info);
}

void UiDatabaseSet::on_ui_toBeShow()
{
#ifdef Q_OS_ANDROID
    if ( !MysqlJniInterface::getInstance().isValid() ) {
        QMessageBox::warning(nullptr, "系统异常", "系统数据库必要模块加载失败，请稍后重试！");
        UiManager::getInstance().showUi(UiName::eUiLogin);
        return ;
    }
#endif

    DBTable_DatabaseSet dbSet;
    if ( DataBase::getInstance().getRemoteDatabaseInfo(dbSet) ) {
        ui->label_showInfo->setText("远程数据库已连接");
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

