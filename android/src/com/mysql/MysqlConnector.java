package com.mysql;

import com.zaxxer.hikari.HikariConfig;
import com.zaxxer.hikari.HikariDataSource;

import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.SQLException;

import java.sql.PreparedStatement;
import java.sql.ResultSet;

import org.json.JSONObject;
import org.json.JSONArray;

class ConstStr {
    public static final String SUCCESS = "Success";
    public static final String FAIL = "Fail";
    public static final String ERROR = "Error";
    public static final String JSON_KEY_RETURN = "JavaResult";
    public static final String JSON_KEY_RESULT = "MysqlValue";

    public static String message(String key, String msg) {
        return key + ": " + msg;
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
                return ConstStr.message(ConstStr.FAIL, "database connect failed!");
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

        return ConstStr.SUCCESS;
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

    public static String executeQuerySql(String sqlCmd) {
        JSONObject jsonResult = new JSONObject();

        try ( Connection conn = dataSource.getConnection();
              PreparedStatement stmt = conn.prepareStatement(sqlCmd);
              // 执行查询并获取结果
              ResultSet rs = stmt.executeQuery() ) {
            // 创建 JSON 数组存储多行数据
            JSONArray jsonArray = new JSONArray();

            // 获取结果集的元数据（可用于获取字段名）
            int columnCount = rs.getMetaData().getColumnCount();

            // 遍历查询结果
            while ( rs.next() ) {
                JSONObject jsonObject = new JSONObject();
                // 获取每一行的所有字段值
                for (int i = 1; i <= columnCount; i++) {
                    String columnName = rs.getMetaData().getColumnName(i);
                    String columnValue = rs.getString(i);
                    jsonObject.put(columnName, columnValue);
                }
                jsonArray.put(jsonObject);
            }

            jsonResult.put(ConstStr.JSON_KEY_RETURN, ConstStr.SUCCESS);
            jsonResult.put(ConstStr.JSON_KEY_RESULT, jsonArray);
        } catch (Exception e) {
            // e.printStackTrace();
            try {
                jsonResult.put(ConstStr.JSON_KEY_RETURN, e.getMessage());
            } catch (Exception e1) {
                e1.printStackTrace();
            }
        }

        return jsonResult.toString();
    }

    public static String executeUpdateSql(String sqlCmd) {
        JSONObject jsonResult = new JSONObject();

        try ( Connection conn = dataSource.getConnection();
              PreparedStatement stmt = conn.prepareStatement(sqlCmd) ) {

            int affectedRows = stmt.executeUpdate();

            jsonResult.put(ConstStr.JSON_KEY_RETURN, ConstStr.SUCCESS);
            jsonResult.put(ConstStr.JSON_KEY_RESULT, affectedRows);
        } catch (Exception e) {
            // e.printStackTrace();
            try {
                jsonResult.put(ConstStr.JSON_KEY_RETURN, e.getMessage());
            } catch (Exception e1) {
                e1.printStackTrace();
            }
        }

        return jsonResult.toString();
    }
}
