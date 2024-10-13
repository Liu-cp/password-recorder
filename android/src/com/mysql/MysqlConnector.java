package com.mysql;

import com.zaxxer.hikari.HikariConfig;
import com.zaxxer.hikari.HikariDataSource;

import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.SQLException;

class ReturnMsg {
    public static final String SUCCESS = "Success";
    public static final String FAIL = "Fail: ";
    public static final String ERROR = "Error: ";

    public static String message(String key, String msg) {
        return key + msg;
    }
}

public class MysqlConnector {
    private static HikariDataSource dataSource = null;

    private static HikariDataSource initHikariDataSource(String host, int port, String dbName, String username, String password) {
        // 构建连接 URL
        String url = String.format("jdbc:mysql://%s:%d/%s", host, port, dbName);

        HikariConfig config = new HikariConfig();
        config.setJdbcUrl(url);
        config.setUsername(username);
        config.setPassword(password);
        config.setMaximumPoolSize(10);

        return new HikariDataSource(config);
    }

    // 测试数据库连接
    public static String testConnect(String host, int port, String dbName, String username, String password) {
        HikariDataSource dataSrc = null;

        try {
            dataSrc = initHikariDataSource(host, port, dbName, username, password);
            if ( dataSrc == null ) {
                return ReturnMsg.message(ReturnMsg.FAIL, "database connect failed!");
            }
        } catch (Exception e) {
            // e.printStackTrace();
            return e.getMessage();
        } catch (Throwable t) {
            return t.getMessage();
        } finally {
            if ( dataSrc != null ) {
                dataSrc.close();  // 关闭连接池
            }
        }

        return ReturnMsg.SUCCESS;
    }

    public static int initMysqlConnector(String host, int port, String dbName, String username, String password) {
        try {
            dataSource = initHikariDataSource(host, port, dbName, username, password);
            if ( dataSource == null ) {
                return -1;
            }
        } catch (Exception e) {
            // e.printStackTrace();
            return -2;
        } catch (Throwable t) {
            return -3;
        }

        return 1;
    }

    public static int closeMysqlConntor() {
        try {
            if ( dataSource != null ) {
                dataSource.close();
            }
        } catch (Exception e) {
            return -1;
        }

        return 1;
    }
}
