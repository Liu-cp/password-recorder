#include "ui/login.h"
#include "ui/signin.h"
#include "ui/uidatabaseset.h"
#include "ui/uimainwindow.h"
#include "ui/uipwddetail.h"
#include "database/database.h"
#include "common/uimanager.h"
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

    // 获取当前时间
    QString currentDateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");

    // 格式化日志输出，添加文件、函数、行号、日期等信息
    QString logEntry = QString("[%1] [%2:%3] [%4] ").arg(currentDateTime)
                           .arg(context.file ? context.file : "Unknown file")
                           .arg(context.line)
                           .arg(context.function ? context.function : "Unknown function");

    switch (type) {
    case QtDebugMsg:
        out << logEntry << "Debug: " << message << "\n";
        break;
    case QtInfoMsg:
        out << logEntry << "Info: " << message << "\n";
        break;
    case QtWarningMsg:
        out << logEntry << "Warning: " << message << "\n";
        break;
    case QtCriticalMsg:
        out << logEntry << "Critical: " << message << "\n";
        break;
    case QtFatalMsg:
        out << logEntry << "Fatal: " << message << "\n";
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
    UiManager::getInstance().addUi(UiName::eUiLogin, new Login());
    UiManager::getInstance().addUi(UiName::eUiDatabaseSet, new UiDatabaseSet());    // eUiDatabaseSet must add after eUiLogin
    UiManager::getInstance().addUi(UiName::eUiSignIn, new SignIn());
    UiManager::getInstance().addUi(UiName::eUiMainWindow, new UiMainWindow());
    UiManager::getInstance().addUi(UiName::eUiPwdDetail, new UiPwdDetail());        // eUiPwdDetail must add after eUiMainWindow
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


