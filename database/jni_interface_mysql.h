#ifndef JNI_INTERFACE_MYSQL_H
#define JNI_INTERFACE_MYSQL_H

#include <QObject>
#include <QJsonObject>

// struct MysqlConnectParams
// {
//     MysqlConnectParams() {}
//     MysqlConnectParams(const QString &hn, int p, const QString &dn, const QString &un, const QString &pwd)
//         : hostName(hn), hostPort(p), databaseName(dn), username(un), password(pwd) {}

//     QString hostName;
//     int hostPort;
//     QString databaseName;
//     QString username;
//     QString password;
// };

class MysqlJniInterface : public QObject
{
    Q_OBJECT
private:
    explicit MysqlJniInterface(QObject *parent = nullptr);
    MysqlJniInterface(const MysqlJniInterface&) = delete;
    MysqlJniInterface& operator=(const MysqlJniInterface&) = delete;
    QJsonObject executeSql(const QString &sqlCmd, const char *javaMethod);

public:
    static MysqlJniInterface& getInstance() {
        static MysqlJniInterface mysql;
        return mysql;
    }

    QString testConnect(const QString &hostName, int hostPort, const QString &databaseName, const QString &username, const QString &password);
    int connectToMysql(const QString &hostName, int hostPort, const QString &databaseName, const QString &username, const QString &password);
    int closeMysqlConnect();
    QJsonObject queryMysql(const QString &sqlCmd);
    QJsonObject updateMysql(const QString &sqlCmd);

private:
    const QString JAVA_RETURN_SUCCESS = "Success";

signals:
};

#endif // JNI_INTERFACE_MYSQL_H
