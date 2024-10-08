#ifndef JNI_INTERFACE_MYSQL_H
#define JNI_INTERFACE_MYSQL_H

#include <QObject>
#include <QJniObject>

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

public:
    static MysqlJniInterface& getInstance() {
        static MysqlJniInterface mysql;
        return mysql;
    }
    // check interface is valid
    bool isValid() {
        return m_javaClass.isValid();
    }
    int testConnect(const QString &hostName, int hostPort, const QString &databaseName, const QString &username, const QString &password);
    int connectToMysql(const QString &hostName, int hostPort, const QString &databaseName, const QString &username, const QString &password);

private:
    QJniObject m_javaClass;

signals:
};

#endif // JNI_INTERFACE_MYSQL_H
