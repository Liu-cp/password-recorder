#include "login.h"
#include "signin.h"
#include "uimainwindow.h"
#include "uipwddetail.h"
#include "uidatabaseset.h"
#include "database.h"
#include "uimanager.h"
#include <QApplication>
#include <QVBoxLayout>
#include <QDebug>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QStandardPaths>
#include <QtConcurrent>

void customMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &message)
{
    // 输出日志到文件
#ifdef Q_OS_WIN
    QFile logFile("password-recorder.log");
#elif defined(Q_OS_ANDROID)
    QString logPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString logFilePath = logPath + "/password-recorder.log";
    QFile logFile(logFilePath);
#endif
    // QMessageBox::information(nullptr, "系统信息", "申请日志写入");

    if (!logFile.open(QIODevice::Append | QIODevice::Text)) {
        qWarning() << "Open log file failed!";
        QMessageBox::information(nullptr, "系统信息", "日志文件打开失败");
        return; // 如果无法打开文件，则返回
    }

    QTextStream out(&logFile);

    switch (type) {
    case QtDebugMsg:
        out << "Debug: " << message << "\n";
        break;
    case QtInfoMsg:
        out << "Info: " << message << "\n";
        break;
    case QtWarningMsg:
        out << "Warning: " << message << "\n";
        break;
    case QtCriticalMsg:
        out << "Critical: " << message << "\n";
        break;
    case QtFatalMsg:
        out << "Fatal: " << message << "\n";
        abort();
    }
    out.flush();
}

void setupLogging()
{
    qInstallMessageHandler(customMessageHandler); // 设置自定义的日志处理器
    qInfo() << "Logging started. Log file path:" << QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/application.log"; // 记录日志文件的路径
}

void init_uiManager()
{
    UiManager::getInstance().addUi(UiName::eUiDatabaseSet, new UiDatabaseSet());
    UiManager::getInstance().addUi(UiName::eUiLogin, new Login());
    UiManager::getInstance().addUi(UiName::eUiSignIn, new SignIn());
    UiManager::getInstance().addUi(UiName::eUiMainWindow, new UiMainWindow());
    UiManager::getInstance().addUi(UiName::eUiPwdDetail, new UiPwdDetail());
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    setupLogging();

    // 设置应用程序图标
    a.setWindowIcon(QIcon(":/password-recorder.ico"));

    if (DataBase::getInstance().dataBaseInit() != 0 ) {
        // 添加错误弹窗，显示错误信息
        QMessageBox::warning(nullptr, "系统错误", "数据库异常，请稍后重试！");
        return -1;
    }
    // 添加全部界面并显示登录界面
    init_uiManager();
    UiManager::getInstance().showUi(UiName::eUiLogin);

    // 界面显示依赖
    QWidget *mainWidget = new QWidget;
    QVBoxLayout *mainLayout = new QVBoxLayout(mainWidget);
    mainLayout->addWidget(UiManager::getInstance().getStackedWidget());
    mainWidget->setLayout(mainLayout);
    mainWidget->show();


    return a.exec();
}


