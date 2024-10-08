#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QSqlDatabase>
#ifdef Q_OS_ANDROID
#include "jni_interface_mysql.h"
#endif

// enum DBTable {
//     eTableUsers = 0,
// };

// const QString DBTableName[] = {"users", ""};

enum DatabaseType {
    eDbType_null,
    eDbType_sqlite,
    eDbType_mysql,
};

struct DBTable_DatabaseSet
{
    DBTable_DatabaseSet() {}
    DBTable_DatabaseSet(const QString &hn, uint16_t p, const QString &dn, const QString &un, const QString &pwd)
        : hostName(hn), hostPort(p), databaseName(dn), username(un), password(pwd) {}
    // 重载 == 运算符
    bool operator==(const DBTable_DatabaseSet &other) const {
        return (hostName == other.hostName &&
               hostPort == other.hostPort &&
               databaseName == other.databaseName &&
               username == other.username &&
                password == other.password);
    }

    QString hostName;
    uint16_t hostPort;
    QString databaseName;
    QString username;
    QString password;
};

struct DBTable_PwdRecorder
{
    DBTable_PwdRecorder() {}
    DBTable_PwdRecorder(const QString &name, const QString &type, const QString &user, const QString &pwd, const QString &url, const QString &notes)
        : pwdName(name), pwdType(type), username(user), password(pwd), pwdUrl(url), pwdNotes(notes) {}

    QString pwdName;
    QString pwdType;
    QString username;
    QString password;
    QString pwdUrl;
    QString pwdNotes;
};

class DataBase : public QObject
{
    Q_OBJECT
private:
    explicit DataBase(QObject *parent = nullptr);
    DataBase(const DataBase&) = delete;
    DataBase& operator=(const DataBase&) = delete;

    const QSqlDatabase& getSqlDataBase() const {
        return m_mysqlValid ? m_mysqlDb : m_sqliteDb;
    }
    void constructTbNamePwdRecorder(const QString &username) {
        m_tbPwdRecorder = "tb_" + username + TB_NAME_PWD_RECORDER;
    }
    bool checkTableExist(const QString& tbName, DatabaseType dbType = DatabaseType::eDbType_null);
    bool createTable4PasswordRecorder();
    int connectToMysql(const DBTable_DatabaseSet &dbSet, const QString &connectName, bool needClose = false);

public:
    ~DataBase();

    // 单例模式
    static DataBase& getInstance() {
        static DataBase database;
        return database;
    }

    // database set
    int dataBaseInit();
    int remoteDatabaseConnectTest(const DBTable_DatabaseSet &dbSet);
    int mysqlDatabaseSet(const DBTable_DatabaseSet &dbSet);
    bool getRemoteDatabaseInfo(DBTable_DatabaseSet &dbSet) {
        if ( !m_mysqlValid ) return false;

        dbSet = m_mysqlDbSet;
        return true;
    }

    QString generateRandomSalt();
    QString hashFunction(const QString &input) const;

    // users
    int userLogin(const QString &username, const QString &password);
    int userSignIn(const QString &username, const QString &password);

    // password records
    int addNewPwdRecord(const DBTable_PwdRecorder &record);
    int updatePwdRecord(const DBTable_PwdRecorder &record);
    int deletePwdRecord(const QString &pwdName);
    std::vector<QString> getAllPwdTypes() const;
    std::vector<QString> getPwdNamesByPwdType(const QString &pwdType) const;
    int getPasswordRecord(const QString &pwdName, DBTable_PwdRecorder &record) const;


private:
    const QString DB_NAME_SQLITE = "sqliteDb";
    const QString DB_NAME_MYSQL = "mysqlDb";
    const QString TB_NAME_MYSQL_INFO = "tb_database_info";
    const QString TB_NAME_USERS = "tb_users";
    const QString TB_NAME_PWD_RECORDER = "_password_recorder";
    QSqlDatabase m_sqliteDb;
    QSqlDatabase m_mysqlDb;
    QString m_tbPwdRecorder;
    bool m_mysqlValid;
    DBTable_DatabaseSet m_mysqlDbSet;
};

#endif // DATABASE_H
