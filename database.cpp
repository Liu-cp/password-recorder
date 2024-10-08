#include "database.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QCryptographicHash>
#include <QDebug>
#include <QUuid>
#include <QMessageBox>

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
        cmd = QString("SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='%1'").arg(tbName);
    }
    else {
#ifdef Q_OS_ANDROID
        // 安卓端通过调用java接口进行查询
        if ( !m_mysqlValid ) {
            qWarning() << "check table exist from mysql, but mysql is not available!!";
            return false;
        }
        // TODO: 调用Java接口并返回
        // return xxxx
#endif
        db = m_mysqlDb;
        cmd = QString("SELECT COUNT(*) FROM information_schema.tables WHERE table_schema='%1' AND table_name='%2'").arg(m_mysqlDbSet.databaseName).arg(tbName);
    }

    QSqlQuery query(db);
    query.exec(cmd);
    if ( query.exec(cmd) && query.next() ) {
        int count = query.value(0).toInt();
        return count > 0; // 如果计数大于0，表存在
    }

    return false;
}

bool DataBase::createTable4PasswordRecorder()
{
    QString cmd = QString("CREATE TABLE %1 ("
                              "pwdName VARCHAR(30) PRIMARY KEY, "
                              "pwdType VARCHAR(30) NOT NULL, "
                              "username TEXT NOT NULL, "
                              "password TEXT NOT NULL, "
                              "pwdUrl TEXT, "
                              "pwdNotes TEXT)").arg(m_tbPwdRecorder);
    // qDebug() << "cmd: "<<command << ", tb: " << m_tbPwdRecorder;
    QSqlQuery query(getSqlDataBase());
    if ( !query.exec(cmd) ) {
        qDebug() << "Failed to create table:" << query.lastError().text();
        return false;
    }

    // 创建索引
    cmd = QString("CREATE INDEX idx_pwdType ON %1 (pwdType)").arg(m_tbPwdRecorder);
    if ( !query.exec(cmd) ) {
        qDebug() << "Failed to create index:" << query.lastError().text();
        return false;
    }

    return true;
}

int DataBase::connectToMysql(const DBTable_DatabaseSet &dbSet, const QString &connectName, bool needClose)
{
    int ret = 0;

    // MySQL 连接
    QSqlDatabase mysqlDb = QSqlDatabase::addDatabase("QMYSQL", connectName);
    mysqlDb.setHostName(dbSet.hostName);
    mysqlDb.setPort(dbSet.hostPort);
    mysqlDb.setDatabaseName(dbSet.databaseName);
    mysqlDb.setUserName(dbSet.username);
    mysqlDb.setPassword(dbSet.password);
    if ( !mysqlDb.open() ) {
        qDebug() << "Cannot connect to mysql: " << mysqlDb.lastError().text();
        needClose = true;
        ret = -1;
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

int DataBase::dataBaseInit()
{
    // SQLite 连接
    m_sqliteDb = QSqlDatabase::addDatabase("QSQLITE", DB_NAME_SQLITE);
    m_sqliteDb.setDatabaseName("sqlite_local.db");  // 数据库文件名
    if ( !m_sqliteDb.open() ) {
        qDebug() << "Cannot connect to sqlite: " << m_sqliteDb.lastError().text();
        return -1;
    }

    // 创建数据表用来保存远程数据库信息
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
    // TODO:检查远程数据库配置并连接
    else {
        QString cmd = QString("SELECT * FROM %1 WHERE type = '%2'").arg(TB_NAME_MYSQL_INFO, "mysql");
        QSqlQuery query(m_sqliteDb);
        if ( query.exec(cmd) && query.next() ) {
            DBTable_DatabaseSet dbSet(query.value("host").toString(), query.value("port").toInt(), query.value("dbName").toString(), query.value("username").toString(), query.value("password").toString());
            mysqlDatabaseSet(dbSet);
        }
    }

    return 0;
}

int DataBase::remoteDatabaseConnectTest(const DBTable_DatabaseSet &dbSet)
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
            // TODO：调用JAVA接口释放连接池
#endif
        }
    }

#ifdef Q_OS_WIN
    int ret = connectToMysql(dbSet, DB_NAME_MYSQL);
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
    QString cmd = QString("UPDATE %1 SET pwdType = :pwdType, username = :username, password = :password, pwdUrl = :pwdUrl, pwdNotes = :pwdNotes WHERE pwdName = :pwdName").arg(m_tbPwdRecorder);
    QSqlQuery query(getSqlDataBase());
    query.prepare(cmd);
    query.bindValue(":pwdName", record.pwdName);
    query.bindValue(":pwdType", record.pwdType);
    query.bindValue(":username", record.username);
    query.bindValue(":password", record.password);
    query.bindValue(":pwdUrl", record.pwdUrl);
    query.bindValue(":pwdNotes", record.pwdNotes);

    if ( !query.exec() ) {
        qDebug() << "Update failed: " << query.lastError().text();
        return -1;
    }

    return 0;
}

int DataBase::deletePwdRecord(const QString &pwdName)
{
    QString cmd = QString("DELETE FROM %1 WHERE pwdName = :pwdName").arg(m_tbPwdRecorder);
    QSqlQuery query(getSqlDataBase());
    query.prepare(cmd);
    query.bindValue(":pwdName", pwdName);

    if ( !query.exec() ) {
        qDebug() << "Delete failed: " << query.lastError().text();
        return -1;
    }

    return 0;
}

std::vector<QString> DataBase::getAllPwdTypes() const
{
    std::vector<QString> types;

    QString command = QString("SELECT DISTINCT pwdType FROM %1").arg(m_tbPwdRecorder);
    QSqlQuery query(getSqlDataBase());
    if ( !query.exec(command) ) {
        qDebug() << "query failed: " << query.lastError().text();
        return types;
    }

    while ( query.next() ) {
        types.emplace_back(query.value(0).toString());
    }

    return types;
}

/*
 * return:
 *      0: success
 *     -1: data already exist
 *     -2: system error
 */
int DataBase::addNewPwdRecord(const DBTable_PwdRecorder &record)
{
    QString cmd = QString("INSERT INTO %1 (pwdName, pwdType, username, password, pwdUrl, pwdNotes) VALUES (:pwdName, :pwdType, :username, :password, :pwdUrl, :pwdNotes)").arg(m_tbPwdRecorder);
    QSqlQuery query(getSqlDataBase());
    query.prepare(cmd);
    query.bindValue(":pwdName", record.pwdName);
    query.bindValue(":pwdType", record.pwdType);
    query.bindValue(":username", record.username);
    query.bindValue(":password", record.password);
    query.bindValue(":pwdUrl", record.pwdUrl);
    query.bindValue(":pwdNotes", record.pwdNotes);

    if ( !query.exec() ) {
        qDebug() << "Insert failed: " << query.lastError().text();
        // 检查是否是主键冲突，检查错误文本
        if (query.lastError().text().contains("UNIQUE constraint failed")) {
            qDebug() << "Primary key conflict occurred.";
            return -1;
        }
        return -2;
    }

    return 0;
}

std::vector<QString> DataBase::getPwdNamesByPwdType(const QString &pwdType) const
{
    std::vector<QString> pwdNames;

    QString cmd = QString("SELECT pwdName FROM %1 WHERE pwdType = :pwdType").arg(m_tbPwdRecorder);
    QSqlQuery query(getSqlDataBase());
    query.prepare(cmd);
    query.bindValue(":pwdType", pwdType);

    if ( !query.exec() ) {
        qDebug() << "Select failed: " << query.lastError().text();
        return pwdNames;
    }

    while ( query.next() ) {
        pwdNames.emplace_back(query.value(0).toString());
    }

    return pwdNames;
}

int DataBase::getPasswordRecord(const QString &pwdName, DBTable_PwdRecorder &record) const
{
    QString cmd = QString("SELECT * FROM %1 WHERE pwdName = :pwdName").arg(m_tbPwdRecorder);
    QSqlQuery query(getSqlDataBase());
    query.prepare(cmd);
    query.bindValue(":pwdName", pwdName);

    if ( !query.exec() || !query.next() ) {
        qDebug() << "Select failed: " << query.lastError().text();
        return -1;
    }

    record.pwdName = query.value("pwdName").toString();
    record.pwdType = query.value("pwdType").toString();
    record.username = query.value("username").toString();
    record.password = query.value("password").toString();
    record.pwdUrl = query.value("pwdUrl").toString();
    record.pwdNotes = query.value("pwdNotes").toString();

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
    QSqlQuery query(getSqlDataBase());

    // 查询特定用户的值
    QString cmd = QString("SELECT pwdHash, salt FROM %1 WHERE username = :username").arg(TB_NAME_USERS);
    query.prepare(cmd);
    query.bindValue(":username", username);
    if ( !query.exec() ) {
        qDebug() << "Query failed:" << query.lastError().text();
        return -2;
    }
    if ( !query.next() ) {
        qDebug() << "user not found, sername: " << username;
        return -1;
    }


    QString storedHash = query.value(0).toString();      // 获取第一列
    QString storedSalt = query.value(1).toString(); // 获取第二列
    if ( storedHash != hashFunction(password+storedSalt) ) {
        qDebug() << "user login failed, because pwd check failed";
        return -1;
    }

    constructTbNamePwdRecorder(username);
    // TODO:检查密码记录表是否存在
    if ( !checkTableExist(m_tbPwdRecorder) ) {
        qDebug() << "user(" << username << ")'s table for save password not exist, create it.";
        if ( !createTable4PasswordRecorder() ) {
            return -2;
        }
    }

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
    QSqlQuery query(getSqlDataBase());

    // 检查表是否存在
    if ( !checkTableExist(TB_NAME_USERS) ) {
        // 表不存在，执行创建表的语句
        QString command = QString("CREATE TABLE %1 (username VARCHAR(25) PRIMARY KEY, pwdHash TEXT, salt TEXT)").arg(TB_NAME_USERS);
        if ( !query.exec(command) ) {
            qDebug() << "Failed to create table:" << query.lastError().text();
            return -1;
        }
    }

    // 检查用户是否存在
    QString cmd = QString("SELECT COUNT(*) FROM %1 WHERE username = :username").arg(TB_NAME_USERS);
    query.prepare(cmd);
    query.bindValue(":username", username);

    if (query.exec() && query.next()) {
        int userCount = query.value(0).toInt();

        if (userCount == 0) {
            QString salt = generateRandomSalt();
            QString pwdHash = hashFunction(password+salt);
            // 用户不存在，插入新用户
            cmd = QString("INSERT INTO %1 (username, pwdHash, salt) VALUES (:username, :pwdHash, :salt)").arg(TB_NAME_USERS);
            query.prepare(cmd);
            query.bindValue(":username", username);
            query.bindValue(":pwdHash", pwdHash);
            query.bindValue(":salt", salt);

            if (query.exec()) {
                qDebug() << "User inserted successfully.";
                // return 0;
            } else {
                qDebug() << "Insert failed:" << query.lastError().text();
                return -1;
            }
        } else {
            qDebug() << "User already exists.";
            return -2;
        }
    } else {
        qDebug() << "Query failed:" << query.lastError().text();
        return -1;
    }

    // 创建成功后同时创建保存密码记录的表项
    constructTbNamePwdRecorder(username);
    if ( checkTableExist(m_tbPwdRecorder) ) {
        qDebug() << "error: table already exist on user signIn: " << m_tbPwdRecorder;
        // 清空表中的全部数据
        QString command = QString("DELETE FROM %1").arg(m_tbPwdRecorder);
        if ( !query.exec(command) ) {
            qDebug() << "clear data for table: " << m_tbPwdRecorder << " failed";
        }
    }
    else {
        if ( !createTable4PasswordRecorder() ) {
            // 删除创建的用户
            QString cmd = QString("DELETE FROM %1 WHERE username = :username").arg(TB_NAME_USERS);
            query.prepare(cmd);
            query.bindValue(":username", username);
            if ( !query.exec() ) {
                qDebug() << "delete user failed: " << username;
            }
            return -1;
        }
    }

    return 0;
}
