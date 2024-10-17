#include "database.h"
#include "public.h"
#include "jni_interface_mysql.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QCryptographicHash>
#include <QDebug>
#include <QUuid>
#include <QMessageBox>
#include <QJsonArray>
#include <QJsonObject>
#include <QThreadPool>

DataBase::DataBase(QObject *parent)
    : QObject{parent}, m_mysqlValid(false)
{}

DataBase::~DataBase()
{
    m_sqliteDb.close();
    QSqlDatabase::removeDatabase(DB_NAME_SQLITE);

    if ( m_mysqlValid ) {
        m_mysqlDb.close();
        QSqlDatabase::removeDatabase(DB_NAME_MYSQL);
    }
}

// 检查表是否存在
bool DataBase::checkTableExist(const QString &tbName, DatabaseType dbType)
{
    if ( dbType == DatabaseType::eDbType_null ) {
        if ( m_mysqlValid ) dbType = DatabaseType::eDbType_mysql;
        else dbType = DatabaseType::eDbType_sqlite;
    }

    QString cmd;
    QSqlDatabase db;

    if ( dbType == eDbType_sqlite ) {
        db = m_sqliteDb;
        cmd = QString("SELECT COUNT(*) AS count FROM sqlite_master WHERE type='table' AND name='%1'").arg(tbName);
    }
    else {
        db = m_mysqlDb;
        cmd = QString("SELECT COUNT(*) AS count FROM information_schema.tables WHERE table_schema='%1' AND table_name='%2'").arg(m_mysqlDbSet.databaseName, tbName);

#ifdef Q_OS_ANDROID
        // 安卓端通过调用java接口进行查询
        if ( !m_mysqlValid ) {
            qWarning() << "check table exist from mysql, but mysql is not available!!";
            return false;
        }
        // 调用Java接口并返回
        QJsonObject jsonObject = MysqlJniInterface::getInstance().queryMysql(cmd);
        if ( !jsonObject.contains(JSON_KEY_RESULT) ) return false;
        QJsonArray jsonArray = jsonObject[JSON_KEY_RESULT].toArray();
        if ( jsonArray.isEmpty() || jsonArray[0].toObject()["count"].toString().toInt()<=0 ) return false;

        return true;
#endif
    }

    QSqlQuery query(db);
    query.exec(cmd);
    if ( query.exec(cmd) && query.next() ) {
        int count = query.value(0).toInt();
        return count > 0; // 如果计数大于0，表存在
    }

    return false;
}

bool DataBase::createTable_tbPasswordRecorder()
{
    QString cmd = QString("CREATE TABLE %1 ("
                              "pwdName VARCHAR(30) PRIMARY KEY, "
                              "pwdType VARCHAR(30) NOT NULL, "
                              "username TEXT NOT NULL, "
                              "password TEXT NOT NULL, "
                              "pwdUrl TEXT, "
                              "pwdNotes TEXT)").arg(m_tbPwdRecorder);

    QSqlQuery query(getSqlDataBase());

    if ( OS_ANDROID && m_mysqlValid ) {
        QJsonObject jsonObject = MysqlJniInterface::getInstance().updateMysql(cmd);
        if ( jsonObject.isEmpty() || jsonObject[JSON_KEY_RETURN]!=RETURN_SUCCESS_STR ) {
            qDebug() << "Failed to create table: " << m_tbPwdRecorder;
            return false;
        }
    }
    else {
        if ( !query.exec(cmd) ) {
            qDebug() << "Failed to create table:" << query.lastError().text();
            return false;
        }
    }

    // 创建索引     TODO: 失败后的异常处理
    cmd = QString("CREATE INDEX idx_pwdType ON %1 (pwdType)").arg(m_tbPwdRecorder);
    if ( OS_ANDROID && m_mysqlValid ) {
        QJsonObject jsonObject = MysqlJniInterface::getInstance().updateMysql(cmd);
        if ( jsonObject.isEmpty() || jsonObject[JSON_KEY_RETURN]!=RETURN_SUCCESS_STR ) {
            qDebug() << "Failed to create index:" << m_tbPwdRecorder;
            return false;
        }
    }
    else {
        if ( !query.exec(cmd) ) {
            qDebug() << "Failed to create index:" << query.lastError().text();
            return false;
        }
    }

    return true;
}

bool DataBase::createTable_tbUsers()
{
    QString cmd = QString("CREATE TABLE IF NOT EXISTS %1 (username VARCHAR(25) PRIMARY KEY, pwdHash TEXT, salt TEXT)").arg(TB_NAME_USERS);

    if ( OS_ANDROID && m_mysqlValid ) {
        QJsonObject jsonObject = MysqlJniInterface::getInstance().updateMysql(cmd);
        if ( jsonObject.isEmpty() || jsonObject[JSON_KEY_RETURN]!=RETURN_SUCCESS_STR ) {
            qCritical() << "Failed to create table:" << jsonObject[JSON_KEY_RETURN];
            return false;
        }
    }
    else {
        QSqlQuery query(getSqlDataBase());
        if ( !query.exec(cmd) ) {
            qCritical() << "Failed to create table:" << query.lastError().text();
            return false;
        }
    }

    return true;
}

QString DataBase::connectToMysql(const DBTable_DatabaseSet &dbSet, const QString &connectName, bool needClose)
{
    QString ret = RETURN_SUCCESS_STR;

    // MySQL 连接
    QSqlDatabase mysqlDb = QSqlDatabase::addDatabase("QMYSQL", connectName);
    mysqlDb.setHostName(dbSet.hostName);
    mysqlDb.setPort(dbSet.hostPort);
    mysqlDb.setDatabaseName(dbSet.databaseName);
    mysqlDb.setUserName(dbSet.username);
    mysqlDb.setPassword(dbSet.password);
    if ( !mysqlDb.open() ) {
        ret = "Cannot connect to mysql: " + mysqlDb.lastError().text();
        qDebug() << ret;
        needClose = true;
    }

    if ( needClose ) {
        mysqlDb.close();
        QSqlDatabase::removeDatabase(connectName);
    }
    else if ( connectName == DB_NAME_MYSQL ) {
        m_mysqlDb = mysqlDb;
    }

    return ret;
}

void DataBase::recordUserLoginHistory(const QString &username)
{
    QString cmd = QString("INSERT INTO %1 (username, lastLoginTime)"
                          "VALUES ('%2', datetime('now')) "
                          "ON CONFLICT(username) DO UPDATE SET lastLoginTime = excluded.lastLoginTime")
                      .arg(TB_NAME_LOGIN_HISTORY, username);
    QSqlQuery query(m_sqliteDb);
    if ( !query.exec(cmd) ) {
        qWarning() << "record new user login info failed: " << query.lastError().text();
        return ;
    }

    // max record 5 users
    cmd = QString("DELETE FROM %1 WHERE username NOT IN ("
                  "SELECT username FROM %1 "
                  "ORDER BY lastLoginTime DESC LIMIT 5)").arg(TB_NAME_LOGIN_HISTORY);
    if ( !query.exec(cmd) ) {
        qWarning() << "record new user login info failed: " << query.lastError().text();
    }
}

int DataBase::dataBaseInit()
{
    // SQLite 连接
    m_sqliteDb = QSqlDatabase::addDatabase("QSQLITE", DB_NAME_SQLITE);
    m_sqliteDb.setDatabaseName("sqlite_local.db");  // 数据库文件名
    if ( !m_sqliteDb.open() ) {
        qFatal() << "Cannot connect to sqlite: " << m_sqliteDb.lastError().text();
        return -1;
    }

    // 创建数据表用来保存远程数据库信息
    QString cmd = QString("CREATE TABLE IF NOT EXISTS %1 ("
                          "type TEXT PRIMARY KEY, "
                          "host TEXT NOT NULL, "
                          "port INTEGER NOT NULL, "
                          "dbName TEXT NOT NULL, "
                          "username TEXT NOT NULL, "
                          "password TEXT NOT NULL)").arg(TB_NAME_MYSQL_INFO);
    QSqlQuery sqliteQuery(m_sqliteDb);
    if ( !sqliteQuery.exec(cmd) ) {
        qFatal() << QString("Create table %1 failed: ").arg(TB_NAME_MYSQL_INFO) << sqliteQuery.lastError().text();
        return false;
    }
    /*
    if ( !checkTableExist(TB_NAME_MYSQL_INFO, DatabaseType::eDbType_sqlite) ) {
        QString cmd = QString("CREATE TABLE %1 ("
                              "type TEXT PRIMARY KEY, "
                              "host TEXT NOT NULL, "
                              "port INTEGER NOT NULL, "
                              "dbName TEXT NOT NULL, "
                              "username TEXT NOT NULL, "
                              "password TEXT NOT NULL"
                              ")").arg(TB_NAME_MYSQL_INFO);
        QSqlQuery query(m_sqliteDb);
        if ( !query.exec(cmd) ) {
            qDebug() << "Failed to create table:" << query.lastError().text();
            return false;
        }
    }
    */
    // 检查远程数据库配置并连接
    cmd = QString("SELECT * FROM %1 WHERE type = '%2'").arg(TB_NAME_MYSQL_INFO, "mysql");
    if ( sqliteQuery.exec(cmd) && sqliteQuery.next() ) {
        DBTable_DatabaseSet dbSet(sqliteQuery.value("host").toString(), sqliteQuery.value("port").toInt(), sqliteQuery.value("dbName").toString(),
                                  sqliteQuery.value("username").toString(), sqliteQuery.value("password").toString());
        mysqlDatabaseSet(dbSet);
    }

    // create table for record history login users
    cmd = QString("CREATE TABLE IF NOT EXISTS %1 ("
                  "username TEXT PRIMARY KEY,"
                  "lastLoginTime DATETIME)").arg(TB_NAME_LOGIN_HISTORY);
    if ( !sqliteQuery.exec(cmd) ) {
        qFatal() << QString("Create table %1 failed: ").arg(TB_NAME_LOGIN_HISTORY) << sqliteQuery.lastError().text();
        return false;
    }

    return 0;
}

QString DataBase::remoteDatabaseConnectTest(const DBTable_DatabaseSet &dbSet)
{
#ifdef Q_OS_WIN
    return connectToMysql(dbSet, "test", true);
#elif defined(Q_OS_ANDROID)
    return MysqlJniInterface::getInstance().testConnect(dbSet.hostName, dbSet.hostPort, dbSet.databaseName, dbSet.username, dbSet.password);
#endif
}

int DataBase::mysqlDatabaseSet(const DBTable_DatabaseSet &dbSet)
{
    // 检查是否已连接，且参数无变更
    if ( m_mysqlValid ) {
        if ( dbSet == m_mysqlDbSet ) {
            qDebug() << "remote mysql already connected, and new params no change, return directly";
            return 0;
        }
        // 释放旧连接
        else {
#ifdef Q_OS_WIN
            m_mysqlDb.close();
            QSqlDatabase::removeDatabase(DB_NAME_MYSQL);
#elif defined(Q_OS_ANDROID)
            // 调用JAVA接口释放连接池
            MysqlJniInterface::getInstance().closeMysqlConnect();
#endif
        }
    }

#ifdef Q_OS_WIN
    int ret = connectToMysql(dbSet, DB_NAME_MYSQL) == RETURN_SUCCESS_STR ? 0 : -1;
#elif defined(Q_OS_ANDROID)
    int ret = MysqlJniInterface::getInstance().connectToMysql(dbSet.hostName, dbSet.hostPort, dbSet.databaseName, dbSet.username, dbSet.password);
#endif
    // 如果连接成功，保存相关mysql信息到本地
    if ( ret == 0 ) {
        m_mysqlValid = true;
        m_mysqlDbSet = dbSet;

        QSqlQuery query(m_sqliteDb);
        QString cmd_select = QString("SELECT type FROM %1 WHERE type = '%2'").arg(TB_NAME_MYSQL_INFO, "mysql");
        QString cmd_new;
        if ( query.exec(cmd_select) && query.next() ) {
            cmd_new = QString("UPDATE %1 SET host = '%2', port = %3, dbName = '%4', username = '%5', password = '%6' WHERE type = '%7'")
                          .arg(TB_NAME_MYSQL_INFO).arg(dbSet.hostName).arg(dbSet.hostPort).arg(dbSet.databaseName).arg(dbSet.username).arg(dbSet.password)
                          .arg("mysql");
        }
        else {
            cmd_new = QString("INSERT INTO %1 (type, host, port, dbName, username, password) VALUES ('%2', '%3', %4, '%5', '%6', '%7')").arg(TB_NAME_MYSQL_INFO)
                          .arg("mysql").arg(dbSet.hostName).arg(dbSet.hostPort).arg(dbSet.databaseName).arg(dbSet.username).arg(dbSet.password);
        }

        if ( !query.exec(cmd_new) ) {
            qWarning() << "Set new mysql DB info to local db failed: " << query.lastError().text() << ", cmd: " << cmd_new;
            // return -1;
        }
    }
    else {
        m_mysqlValid = false;
        m_mysqlDbSet = DBTable_DatabaseSet();

        QString cmd = QString("DELETE FROM %1 WHERE type = '%2'").arg(TB_NAME_MYSQL_INFO, "mysql");
        QSqlQuery query(m_sqliteDb);
        query.exec(cmd);
    }

    return ret;
}

QString DataBase::generateRandomSalt() {
    // 生成一个随机的 UUID，并转换为字符串
    QString salt = QUuid::createUuid().toString();
    // 移除 UUID 字符串中的大括号
    salt.remove(0, 1).remove(salt.length() - 1, 1);
    return salt;
}

int DataBase::updatePwdRecord(const DBTable_PwdRecorder &record)
{
    QString cmd = QString("UPDATE %1 SET pwdType = '%2', username = '%3', password = '%4', pwdUrl = '%5', pwdNotes = '%6' WHERE pwdName = '%7'")
                        .arg(m_tbPwdRecorder).arg(record.pwdType).arg(record.username).arg(record.password).arg(record.pwdUrl).arg(record.pwdNotes)
                        .arg(record.pwdName);
    if ( OS_ANDROID && m_mysqlValid ) {
        QJsonObject jsonObject = MysqlJniInterface::getInstance().updateMysql(cmd);
        if ( jsonObject.isEmpty() || jsonObject[JSON_KEY_RETURN]!=RETURN_SUCCESS_STR ) {
            qWarning() << "Update failed: " << jsonObject[JSON_KEY_RETURN].toString();
            return -1;
        }
    }
    else {
        QSqlQuery query(getSqlDataBase());
        if ( !query.exec(cmd) ) {
            qWarning() << "Update failed: " << query.lastError().text();
            return -1;
        }
    }

    return 0;
}

int DataBase::deletePwdRecord(const QString &pwdName)
{
    QString cmd = QString("DELETE FROM %1 WHERE pwdName = '%2'").arg(m_tbPwdRecorder, pwdName);

    if ( OS_ANDROID && m_mysqlValid ) {
        QJsonObject jsonObject = MysqlJniInterface::getInstance().updateMysql(cmd);
        if ( jsonObject.isEmpty() || jsonObject[JSON_KEY_RETURN]!=RETURN_SUCCESS_STR ) {
            qWarning() << "Delete failed: " << jsonObject[JSON_KEY_RETURN].toString();
            return -1;
        }
    }
    else {
        QSqlQuery query(getSqlDataBase());
        if ( !query.exec(cmd) ) {
            qWarning() << "Delete failed: " << query.lastError().text();
            return -1;
        }
    }

    return 0;
}

/*
 * return:
 *      0: success
 *     -1: data already exist
 *     -2: system error
 */
int DataBase::addNewPwdRecord(const DBTable_PwdRecorder &record)
{
    QString cmd = QString("INSERT INTO %1 (pwdName, pwdType, username, password, pwdUrl, pwdNotes) VALUES ('%2', '%3', '%4', '%5', '%6', '%7')")
                      .arg(m_tbPwdRecorder).arg(record.pwdName).arg(record.pwdType).arg(record.username).arg(record.password).arg(record.pwdUrl)
                      .arg(record.pwdNotes);

    if ( OS_ANDROID && m_mysqlValid ) {
        QJsonObject jsonObject = MysqlJniInterface::getInstance().updateMysql(cmd);
        if ( jsonObject.isEmpty() || jsonObject[JSON_KEY_RETURN]!=RETURN_SUCCESS_STR ) {
            qDebug() << "Insert failed: " << jsonObject[JSON_KEY_RETURN].toString();
            if ( jsonObject[JSON_KEY_RETURN].toString().contains("Duplicate entry") ) {
                qDebug() << "Primary key conflict occurred.";
                return -1;
            }
            return -2;
        }
    }
    else {
        QSqlQuery query(getSqlDataBase());
        if ( !query.exec(cmd) ) {
            qDebug() << "Insert failed: " << query.lastError().text();
            // 检查是否是主键冲突，检查错误文本
            if ( m_mysqlValid ) {
                if ( query.lastError().text().contains("Duplicate entry") ) {
                    qDebug() << "Primary key conflict occurred.";
                    return -1;
                }
            }
            else {
                if ( query.lastError().text().contains("UNIQUE constraint failed") ) {
                    qDebug() << "Primary key conflict occurred.";
                    return -1;
                }
            }

            return -2;
        }
    }

    return 0;
}

std::vector<QString> DataBase::getAllPwdTypes() const
{
    std::vector<QString> types;

    QString cmd = QString("SELECT DISTINCT pwdType FROM %1").arg(m_tbPwdRecorder);

    if ( OS_ANDROID && m_mysqlValid ) {
        QJsonObject jsonObject = MysqlJniInterface::getInstance().queryMysql(cmd);
        if ( !jsonObject[JSON_KEY_RESULT].isArray() ) {
            qWarning() << "query failed: " << jsonObject[JSON_KEY_RETURN];
            return types;
        }

        QJsonArray jsonArray = jsonObject[JSON_KEY_RESULT].toArray();
        for ( const auto &value : jsonArray ) {
            auto object = value.toObject();
            if ( !object["pwdType"].toString().isEmpty() ) {
                types.push_back(object["pwdType"].toString());
            }
        }
    }
    else {
        QSqlQuery query(getSqlDataBase());
        if ( !query.exec(cmd) ) {
            qWarning() << "query failed: " << query.lastError().text();
            return types;
        }

        while ( query.next() ) {
            types.emplace_back(query.value(0).toString());
        }
    }

    return types;
}

std::vector<QString> DataBase::getPwdNamesByPwdType(const QString &pwdType) const
{
    std::vector<QString> pwdNames;

    QString cmd = QString("SELECT pwdName FROM %1 WHERE pwdType = '%2'").arg(m_tbPwdRecorder, pwdType);

    if ( OS_ANDROID && m_mysqlValid ) {
        QJsonObject jsonObject = MysqlJniInterface::getInstance().queryMysql(cmd);
        if ( !jsonObject[JSON_KEY_RESULT].isArray() ) {
            qWarning() << "query failed: " << jsonObject[JSON_KEY_RETURN];
            return pwdNames;
        }

        QJsonArray jsonArray = jsonObject[JSON_KEY_RESULT].toArray();
        for ( const auto &value : jsonArray ) {
            auto object = value.toObject();
            if ( !object["pwdName"].toString().isEmpty() ) {
                pwdNames.push_back(object["pwdName"].toString());
            }
        }
    }
    else {
        QSqlQuery query(getSqlDataBase());
        query.prepare(cmd);

        if ( !query.exec() ) {
            qDebug() << "Select failed: " << query.lastError().text();
            return pwdNames;
        }

        while ( query.next() ) {
            pwdNames.emplace_back(query.value(0).toString());
        }
    }

    return pwdNames;
}

int DataBase::getPasswordRecord(const QString &pwdName, DBTable_PwdRecorder &record) const
{
    QString cmd = QString("SELECT * FROM %1 WHERE pwdName = '%2'").arg(m_tbPwdRecorder, pwdName);

    if ( OS_ANDROID && m_mysqlValid ) {
        QJsonObject jsonObject = MysqlJniInterface::getInstance().queryMysql(cmd);
        if ( jsonObject[JSON_KEY_RETURN] != RETURN_SUCCESS_STR ) {
            qWarning() << "Select failed: " << jsonObject[JSON_KEY_RETURN];
            return -1;
        }
        QJsonArray jsonArray = jsonObject[JSON_KEY_RESULT].toArray();
        QJsonObject result = jsonArray[0].toObject();
        if ( result.isEmpty() ) {
            qWarning() << "Select failed: " << jsonObject[JSON_KEY_RETURN];
            return -1;
        }

        record.pwdName = result["pwdName"].toString();
        record.pwdType = result["pwdType"].toString();
        record.username = result["username"].toString();
        record.password = result["password"].toString();
        record.pwdUrl = result["pwdUrl"].toString();
        record.pwdNotes = result["pwdNotes"].toString();
    }
    else {
        QSqlQuery query(getSqlDataBase());
        if ( !query.exec(cmd) || !query.next() ) {
            qWarning() << "Select failed: " << query.lastError().text();
            return -1;
        }

        record.pwdName = query.value("pwdName").toString();
        record.pwdType = query.value("pwdType").toString();
        record.username = query.value("username").toString();
        record.password = query.value("password").toString();
        record.pwdUrl = query.value("pwdUrl").toString();
        record.pwdNotes = query.value("pwdNotes").toString();
    }

    return 0;
}

QString DataBase::hashFunction(const QString &input) const
{
    // 将输入转换为字节数组
    QByteArray byteArray = input.toUtf8();
    // 计算 SHA-256 哈希
    QByteArray hash = QCryptographicHash::hash(byteArray, QCryptographicHash::Sha256);

    // 将哈希值转换为十六进制字符串
    return hash.toHex();
}

/*
 * return:
 *      0：成功
 *     -1：用户不存在或密码错误
 *     -2：系统错误
 */
int DataBase::userLogin(const QString &username, const QString &password)
{
    // create table for users
    if ( !createTable_tbUsers() ) return -2;

    QString cmd = QString("SELECT pwdHash, salt FROM %1 WHERE username = '%2'").arg(TB_NAME_USERS, username);
    QString storedHash, storedSalt;

    if ( m_mysqlValid && OS_ANDROID) {
        QJsonObject query = MysqlJniInterface::getInstance().queryMysql(cmd);
        if ( query.isEmpty() || query[JSON_KEY_RETURN]!=RETURN_SUCCESS_STR ) {
            qCritical() << "Query failed! " << query[JSON_KEY_RETURN];
            return -2;
        }
        QJsonArray jsonArray = query[JSON_KEY_RESULT].toArray();
        if ( jsonArray.isEmpty() ) {
            qDebug() << "user not found, sername: " << username;
            return -1;
        }

        QJsonObject result = jsonArray[0].toObject();
        storedHash = result["pwdHash"].toString();
        storedSalt = result["salt"].toString();
    }
    else {
        // 查询特定用户的值
        QSqlQuery query(getSqlDataBase());
        query.prepare(cmd);
        if ( !query.exec() ) {
            qCritical() << "Query failed:" << query.lastError().text();
            return -2;
        }
        if ( !query.next() ) {
            qDebug() << "user not found, sername: " << username;
            return -1;
        }

        storedHash = query.value(0).toString();      // 获取第一列
        storedSalt = query.value(1).toString(); // 获取第二列
    }

    if ( storedHash != hashFunction(password+storedSalt) ) {
        qDebug() << "user login failed, because pwd check failed";
        return -1;
    }

    constructTbNamePwdRecorder(username);
    // 检查密码记录表是否存在
    if ( !checkTableExist(m_tbPwdRecorder) ) {
        qDebug() << "user(" << username << ")'s table for save password not exist, create it.";
        if ( !createTable_tbPasswordRecorder() ) {
            return -2;
        }
    }

    // record user login history
    QThreadPool::globalInstance()->start([=]() {    // 注意捕获方式，使用&捕获全部，在debug模式下正常，release模式下会出现异常。
        this->recordUserLoginHistory(username);
    });

    return 0;
}

/*
 * return:
 *       0： 成功
 *      -1: 系统错误
 *      -2: 账户已存在
 */
int DataBase::userSignIn(const QString &username, const QString &password)
{
    // create table for users
    if ( !createTable_tbUsers() ) return -2;

    QSqlQuery query(getSqlDataBase());
    // 检查用户是否存在
    QString cmd = QString("SELECT COUNT(*) AS count FROM %1 WHERE username = '%2'").arg(TB_NAME_USERS, username);

    if ( OS_ANDROID && m_mysqlValid ) {
        QJsonObject jsonObject = MysqlJniInterface::getInstance().queryMysql(cmd);
        if ( !jsonObject.contains(JSON_KEY_RESULT) ) {
            qDebug() << "Query failed";
            return -1;
        }
        QJsonArray jsonArray = jsonObject[JSON_KEY_RESULT].toArray();
        if ( !jsonArray.isEmpty() && jsonArray[0].toObject()["count"].toString().toInt()>0 ) {
            qDebug() << "User already exists.";
            return -2;
        }
    }
    else {
        query.prepare(cmd);
        if (query.exec() && query.next()) {
            int userCount = query.value(0).toInt();
            if ( userCount != 0 ) {
                qDebug() << "User already exists.";
                return -2;
            }
        }
        else {
            qDebug() << "Query failed:" << query.lastError().text();
            return -1;
        }
    }

    // add new user
    QString salt = generateRandomSalt();
    QString pwdHash = hashFunction(password+salt);
    // 用户不存在，插入新用户
    cmd = QString("INSERT INTO %1 (username, pwdHash, salt) VALUES ('%2', '%3', '%4')").arg(TB_NAME_USERS, username, pwdHash, salt);

    if ( OS_ANDROID && m_mysqlValid ) {
        QJsonObject jsonObject = MysqlJniInterface::getInstance().updateMysql(cmd);
        if ( jsonObject.isEmpty() || jsonObject[JSON_KEY_RETURN]!=RETURN_SUCCESS_STR ) {
            qDebug() << "Failed to add new user: " << username;
            return -1;
        }
    }
    else {
        query.prepare(cmd);
        if ( query.exec() ) {
            qDebug() << "User inserted successfully.";
            // return 0;
        } else {
            qDebug() << "Insert failed:" << query.lastError().text();
            return -1;
        }
    }


    // 创建成功后同时创建保存密码记录的表项
    constructTbNamePwdRecorder(username);
    if ( checkTableExist(m_tbPwdRecorder) ) {
        qDebug() << "error: table already exist on user signIn: " << m_tbPwdRecorder;
        // 清空表中的全部数据
        cmd = QString("DELETE FROM %1").arg(m_tbPwdRecorder);

        if ( OS_ANDROID && m_mysqlValid ) {
            QJsonObject jsonObject = MysqlJniInterface::getInstance().updateMysql(cmd);
            if ( jsonObject.isEmpty() || jsonObject[JSON_KEY_RETURN]!=RETURN_SUCCESS_STR ) {
                qWarning() << "clear data for table: " << m_tbPwdRecorder << " failed";
            }
        }
        else {
            if ( !query.exec(cmd) ) {
                qWarning() << "clear data for table: " << m_tbPwdRecorder << " failed";
            }
        }
    }
    else {
        if ( !createTable_tbPasswordRecorder() ) {
            // 删除创建的用户
            cmd = QString("DELETE FROM %1 WHERE username = '%2'").arg(TB_NAME_USERS, username);

            if ( OS_ANDROID && m_mysqlValid ) {
                QJsonObject jsonObject = MysqlJniInterface::getInstance().updateMysql(cmd);
                if ( jsonObject.isEmpty() || jsonObject[JSON_KEY_RETURN]!=RETURN_SUCCESS_STR ) {
                    qWarning() << "delete user failed: " << username;
                }
            }
            else {
                if ( !query.exec(cmd) ) {
                    qWarning() << "delete user failed: " << username;
                }
            }

            return -1;
        }
    }

    return 0;
}

QStringList DataBase::getUserLoginHistory()
{
    QStringList historyUsers;

    QString cmd = QString("SELECT username FROM %1 ORDER BY lastLoginTime DESC").arg(TB_NAME_LOGIN_HISTORY);
    QSqlQuery query(m_sqliteDb);
    if ( !query.exec(cmd) ) {
        qWarning() << "get user login history failed: " << query.lastError().text();
        return historyUsers;
    }

    while ( query.next() ) {
        historyUsers.push_back(query.value(0).toString());
    }

    return historyUsers;
}
