#include "jni_interface_mysql.h"
#include <QJniObject>
#include <QJniEnvironment>
#include <jni.h>
#include <QDebug>
#include <QMessageBox>
#include <QtConcurrent>
#include <QEventLoop>
#include <android/native_activity.h>
#include <QJsonDocument>

MysqlJniInterface::MysqlJniInterface(QObject *parent)
    : QObject{parent}
{
}

void checkNetworkStatus() {
    // 获取 Android Context
    QJniObject context = QJniObject::callStaticObjectMethod("org/qtproject/qt/android/QtNative",
                                                            "activity",
                                                            "()Landroid/app/Activity;");

    if (context.isValid()) {
        // 调用 Java 静态方法 isNetworkAvailable
        QJniObject result = QJniObject::callStaticObjectMethod(
            "com/mysql/MysqlConnector",  // Java 类的完整路径
            "isNetworkAvailable",        // Java 静态方法名
            "(Landroid/content/Context;)Z",  // 方法签名，表示传入一个 Context，返回 boolean
            context.object<jobject>()   // 传入的参数 (Context)
            );

        bool isNetworkAvailable = result.callMethod<jboolean>("booleanValue");
        if (isNetworkAvailable) {
            qDebug() << "网络可用";
        } else {
            qDebug() << "网络不可用";
        }
        QMessageBox::warning(nullptr, "系统信息", QString("网络可用性检查结果：%1").arg(isNetworkAvailable));
    } else {
        qDebug() << "无法获取 Context 对象";
        QMessageBox::warning(nullptr, "系统信息","无法获取 Context 对象");
    }
}

QString MysqlJniInterface::testConnect(const QString &hostName, int hostPort, const QString &databaseName, const QString &username, const QString &password)
{
    QJniObject result = QJniObject::callStaticObjectMethod(
        "com/mysql/MysqlConnector",    // Java类的包路径
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
        return "Fail: call JNI method failed!";
    }

    // 检查返回值是否有效
    if ( !result.isValid() ) {
        // QMessageBox::warning(nullptr, "系统异常", "Failed to call Java method");
        return "Failed to call Java method";
    }
    // QMessageBox::information(nullptr, "系统信息", QString("result: %1").arg(result.toString()));

    return result.toString(); // 大于0表示成功
}

int MysqlJniInterface::connectToMysql(const QString &hostName, int hostPort, const QString &databaseName, const QString &username, const QString &password)
{
    jint result = QJniObject::callStaticMethod<jint>(
        "com/mysql/MysqlConnector",    // Java类的包路径
        "initMysqlConnector",                // Java 方法名
        "(Ljava/lang/String;ILjava/lang/String;Ljava/lang/String;Ljava/lang/String;)I",  // 方法签名
        QJniObject::fromString(hostName).object<jstring>(),  // 参数：host
        hostPort,                                            // 参数：port
        QJniObject::fromString(databaseName).object<jstring>(),// 参数：dbName
        QJniObject::fromString(username).object<jstring>(),  // 参数：user
        QJniObject::fromString(password).object<jstring>()   // 参数：password
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

    return result==1 ? 0 : result; // java返回1表示成功
}

int MysqlJniInterface::closeMysqlConnect()
{
    jint result = QJniObject::callStaticMethod<jint>(
        "com/mysql/MysqlConnector",    // Java类的包路径
        "closeMysqlConntor",            // Java 方法名
        "()I"  // 方法签名
    );

    // 检查 JNI 调用是否有异常
    QJniEnvironment env;
    if (env->ExceptionCheck()) {
        env->ExceptionClear();
        qDebug() << "JNI 调用失败";
        return -2;
    }

    if ( result == 0 ) {
        qDebug() << "JNI 调用异常或未调用到JAVA方法";
        return -3;
    }

    qDebug() << "result: " << result;
    return result==1 ? 0 : result; // java返回1表示成功
}

QJsonObject MysqlJniInterface::executeSql(const QString &sqlCmd, const char *javaMethod)
{
    QJniObject result = QJniObject::callStaticObjectMethod(
        "com/mysql/MysqlConnector",    // Java类的包路径
        javaMethod,                // Java 方法名
        "(Ljava/lang/String;)Ljava/lang/String;",  // 方法签名
        QJniObject::fromString(sqlCmd).object<jstring>()  // 参数
        );

    // 检查 JNI 调用是否有异常
    QJniEnvironment env;
    if (env->ExceptionCheck() ) {
        env->ExceptionClear();
        qDebug() << "JNI 调用失败";
    }

    // 检查返回值是否有效
    if ( !result.isValid() ) {
        qDebug() << "Failed to call Java method";
    }

    qDebug() << "result: " << result.toString();
    // QMessageBox::information(nullptr, "调试信息", "result: " + result.toString());
    // 将 JSON 字符串解析为 QJsonDocument
    QJsonDocument jsonDoc = QJsonDocument::fromJson(result.toString().toUtf8());

    QJsonObject jsonObject;
    if ( !jsonDoc.isNull() && jsonDoc.isObject() ) {
        jsonObject = jsonDoc.object();
    }

    return jsonObject;
}

QJsonObject MysqlJniInterface::queryMysql(const QString &sqlCmd)
{
    return executeSql(sqlCmd, "executeQuerySql");
}

QJsonObject MysqlJniInterface::updateMysql(const QString &sqlCmd)
{
    return executeSql(sqlCmd, "executeUpdateSql");
}
