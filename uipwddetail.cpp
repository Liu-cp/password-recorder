#include "uipwddetail.h"
#include "ui_uipwddetail.h"
#include "uimanager.h"
#include <QDebug>
#include <QApplication>
#include <QClipboard>
#include <QMessageBox>
#include <QRandomGenerator>

UiPwdDetail::UiPwdDetail(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::UiPwdDetail)
    , m_showType(UiPwdDetailShowType::eTypeNone)
{
    ui->setupUi(this);
}

UiPwdDetail::~UiPwdDetail()
{
    delete ui;
}

void UiPwdDetail::on_ui_toBeShow(UiPwdDetailShowType type, const QString pwdName)
{
    qDebug() << "UiPwdDetailShowType: " << type << ", pwdName: " << pwdName;
    m_showType = type;

    if ( type == UiPwdDetailShowType::eCreatePwd ) {
        m_originRecord = DBTable_PwdRecorder();

        ui->lineEdit_pwdName->clear();  ui->lineEdit_pwdName->setReadOnly(false);
        ui->lineEdit_pwdType->clear();
        ui->lineEdit_username->clear();
        ui->lineEdit_password->clear(); ui->checkBox_showPwd->setCheckState(Qt::Unchecked);
        ui->lineEdit_pwdUrl->clear();
        ui->lineEdit_pwdNotes->clear();

        ui->button_genPwd->setText("生成密码");
    }
    else {
        // 检查数据库数据
        int ret = DataBase::getInstance().getPasswordRecord(pwdName, m_originRecord);
        if ( ret != 0 ) {
            QMessageBox::warning(nullptr, "警告", "名称为\""+pwdName+"\"的密码记录未找到，请重试！");
            UiManager::getInstance().showUi(UiName::eUiMainWindow);
            return ;
        }

        // 显示数据
        ui->lineEdit_pwdName->setText(pwdName); ui->lineEdit_pwdName->setReadOnly(true);
        ui->lineEdit_pwdType->setText(m_originRecord.pwdType);
        ui->lineEdit_username->setText(m_originRecord.username);
        ui->lineEdit_password->setText(m_originRecord.password);
        ui->lineEdit_pwdUrl->setText(m_originRecord.pwdUrl);
        ui->lineEdit_pwdNotes->setText(m_originRecord.pwdNotes);

        ui->button_genPwd->setText("复制用户名");
    }
}

void UiPwdDetail::on_button_return_clicked()
{
    if ( checkDataChanged() ) {
        QMessageBox msgBox;
        msgBox.setText("有数据更改未保存，请确认是否继续返回？");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);

        if ( msgBox.exec() == QMessageBox::No ) return ;
    }

    UiManager::getInstance().showUi(UiName::eUiMainWindow);
}


void UiPwdDetail::on_checkBox_showPwd_stateChanged(int arg1)
{
    if ( arg1 == Qt::Checked ) ui->lineEdit_password->setEchoMode(QLineEdit::Normal);
    else ui->lineEdit_password->setEchoMode(QLineEdit::Password);
}


void UiPwdDetail::on_button_genPwd_clicked()
{
    if ( m_showType == UiPwdDetailShowType::eCreatePwd ) {
        if ( !ui->lineEdit_password->text().isEmpty() ) {
            QMessageBox msgBox;
            msgBox.setText("密码已存在，请确认是否继续？");
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            msgBox.setDefaultButton(QMessageBox::No);

            if ( msgBox.exec() == QMessageBox::No ) return ;
        }

        ui->lineEdit_password->setText(generateRandomPassword());
    }
    else {
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(ui->lineEdit_username->text());
        QMessageBox::information(nullptr, "系统通知", "用户名已复制到剪贴板！");
    }
}


void UiPwdDetail::on_button_copyPwd_clicked()
{
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(ui->lineEdit_password->text());
    QMessageBox::warning(nullptr, "警告", "密码已复制到剪贴板，请注意密码安全！");
}


void UiPwdDetail::on_button_pwdDelete_clicked()
{
    QMessageBox msgBox;
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    if ( m_showType == UiPwdDetailShowType::eCreatePwd ) {
        msgBox.setText("您正在创建一个新的密码记录，确认取消吗？");
    }
    else msgBox.setText("该条密码记录将被删除，请确认？");

    if ( msgBox.exec() == QMessageBox::No ) return ;

    if ( m_showType == UiPwdDetailShowType::eShowPwd ) {
        // 从数据库中删除数据
        int ret = DataBase::getInstance().deletePwdRecord(ui->lineEdit_pwdName->text());
        if ( ret != 0 ) {
            QMessageBox::warning(nullptr, "密码项删除失败", "系统异常，请稍后重试！");
            return ;
        }
    }

    UiManager::getInstance().showUi(UiName::eUiMainWindow);
}

QString UiPwdDetail::generateRandomPassword() {
    const QString characters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                               "abcdefghijklmnopqrstuvwxyz"
                               "0123456789"
                               "!@#$%^&*()_-+=<>?";

    QRandomGenerator *generator = QRandomGenerator::global();
    int length = generator->bounded(8, 17);  // 随机生成长度，范围为 8 到 16

    QString password;
    for (int i = 0; i < length; ++i) {
        int index = generator->bounded(characters.length());
        password.append(characters[index]);
    }

    return password;
}

bool UiPwdDetail::checkDataChanged()
{
    DBTable_PwdRecorder record(ui->lineEdit_pwdName->text(), ui->lineEdit_pwdType->text(), ui->lineEdit_username->text(), ui->lineEdit_password->text(), ui->lineEdit_pwdUrl->text(), ui->lineEdit_pwdNotes->text());
    if ( m_showType == UiPwdDetailShowType::eCreatePwd ) {
        return ( !record.pwdName.isEmpty() || !record.pwdType.isEmpty() ||
                !record.username.isEmpty() || !record.password.isEmpty() ||
                !record.pwdUrl.isEmpty() || !record.pwdNotes.isEmpty() );
    }
    else {
        return ( m_originRecord.pwdName!=record.pwdName || m_originRecord.pwdType!=record.pwdType ||
                m_originRecord.username!=record.username || m_originRecord.password!=record.password ||
                m_originRecord.pwdUrl!=record.pwdUrl || m_originRecord.pwdNotes!=record.pwdNotes );
    }
}

void UiPwdDetail::on_button_pwdSave_clicked()
{
    DBTable_PwdRecorder record(ui->lineEdit_pwdName->text(), ui->lineEdit_pwdType->text(),  ui->lineEdit_username->text(), ui->lineEdit_password->text(), ui->lineEdit_pwdUrl->text(), ui->lineEdit_pwdNotes->text());
    if ( record.pwdName.isEmpty() || record.pwdType.isEmpty() || record.username.isEmpty() || record.password.isEmpty() ) {
        QMessageBox::warning(nullptr, "密码项保存失败", "\"名称\"、\"类别\"、\"用户名\"、\"密码\"为必填项，请完整输入后重试！");
        return ;
    }

    if ( m_showType == UiPwdDetailShowType::eCreatePwd ) {
        int ret = DataBase::getInstance().addNewPwdRecord(record);
        if ( ret != 0 ) {
            QString info = ret == -1 ? "密码项已存在，请输入新的密码名称后添加或从主页点击后进行更新！" : "系统错误，请稍后重试！";
            QMessageBox::warning(nullptr, "密码项添加失败", info);
            return ;
        }
        QMessageBox::information(nullptr, "添加密码项", "密码项添加成功！！！");
        UiManager::getInstance().showUi(UiName::eUiMainWindow);
    }
    else {
        if ( !checkDataChanged() ) {
            QMessageBox::warning(nullptr, "密码项更新失败", "数据未做修改，无需更新！");
            return ;
        }
        int ret = DataBase::getInstance().updatePwdRecord(record);
        if ( ret != 0 ) {
            QMessageBox::warning(nullptr, "密码项更新失败", "系统错误，请稍后重试！");
            return ;
        }
        QMessageBox::information(nullptr, "更新密码项", "密码项更新成功！！！");
        m_originRecord = record;
    }
}

