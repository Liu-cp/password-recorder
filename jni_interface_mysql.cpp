#include "jni_interface_mysql.h"
#include <QJniEnvironment>
#include <jni.h>
#include <QDebug>
#include <QMessageBox>
#include <QtConcurrent>
#include <QEventLoop>
#include <android/native_activity.h>

MysqlJniInterface::MysqlJniInterface(QObject *parent)
    : QObject{parent}
{
    m_javaClass = QJniObject("com/MysqlConnector/MysqlConnector");
    if ( !m_javaClass.isValid() ) {
        qWarning() << "Java 类(MysqlConnector)加载失败";
        QMessageBox::warning(nullptr, "系统异常", "安卓模块“MysqlConnector”加载失败！");
    }
}

int MysqlJniInterface::testConnect(const QString &hostName, int hostPort, const QString &databaseName, const QString &username, const QString &password)
{
    if ( !isValid() ) return -3;

    // QEventLoop loop;
    // QJniObject result;
    // // 使用 QtConcurrent 异步调用 Java 方法
    // QtConcurrent::run([&]() {
    //     // 调用 Java 方法
    //     result = m_javaClass.callObjectMethod(
    //         "testConnect",                // Java 方法名
    //         "(Ljava/lang/String;ILjava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;",  // 方法签名
    //         QJniObject::fromString(hostName).object<jstring>(),  // 参数：host
    //         hostPort,                                            // 参数：port
    //         QJniObject::fromString(databaseName).object<jstring>(),// 参数：dbName
    //         QJniObject::fromString(username).object<jstring>(),  // 参数：user
    //         QJniObject::fromString(password).object<jstring>()// 参数：password
    //         );
    //     loop.quit();
    // });
    // loop.exec();
    QJniObject result = m_javaClass.callObjectMethod(
        "testConnect",                // Java 方法名
        "(Ljava/lang/String;ILjava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;",  // 方法签名
        QJniObject::fromString(hostName).object<jstring>(),  // 参数：host
        hostPort,                                            // 参数：port
        QJniObject::fromString(databaseName).object<jstring>(),// 参数：dbName
        QJniObject::fromString(username).object<jstring>(),  // 参数：user
        QJniObject::fromString(password).object<jstring>()   // 参数：password
    );

    // 检查 JNI 调用是否有异常
    QJniEnvironment env;
    if (env->ExceptionCheck() ) {
        env->ExceptionClear();
        qDebug() << "JNI 调用失败";
        return -4;
    }

    // 检查返回值是否有效
    if ( !result.isValid() ) {
        QMessageBox::warning(nullptr, "系统异常", "Failed to call Java method");
        return -5;
    }
    QMessageBox::information(nullptr, "系统信息", QString("resuilt: %1").arg(result.toString()));

    return result.toString()=="" ? 0 : 1; // 大于0表示成功
}

int MysqlJniInterface::connectToMysql(const QString &hostName, int hostPort, const QString &databaseName, const QString &username, const QString &password)
{
    if ( !isValid() ) return -3;

    // 将 QString 转换为 Java 的 String
    QJniObject jHost = QJniObject::fromString(hostName);
    QJniObject jDbName = QJniObject::fromString(databaseName);
    QJniObject jUser = QJniObject::fromString(username);
    QJniObject jPassword = QJniObject::fromString(password);

    // 调用 Java 方法
    jint result = m_javaClass.callMethod<jint>(
        "connectToMysql",                // Java 方法名
        "(Ljava/lang/String;ILjava/lang/String;Ljava/lang/String;Ljava/lang/String;)I",  // 方法签名
        jHost.object<jstring>(),      // 参数1: host (String)
        hostPort,                     // 参数2: port (int)
        jDbName.object<jstring>(),    // 参数3: dbName (String)
        jUser.object<jstring>(),      // 参数4: user (String)
        jPassword.object<jstring>()   // 参数5: password (String)
        );

    // 检查 JNI 调用是否有异常
    QJniEnvironment env;
    if (env->ExceptionCheck()) {
        env->ExceptionClear();
        qDebug() << "JNI 调用失败";
        return -4;
    }

    if ( result == 0 ) {
        qDebug() << "JNI 调用异常或未调用到JAVA方法";
        return -5;
    }

    return result>0 ? 0 : result; // 大于0表示成功
}
