package com.MysqlConnector;

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.ResultSet;
import java.sql.Statement;
import java.sql.SQLException;
import java.util.LinkedList;

import android.os.Environment;

import java.io.File;
import java.io.IOException;
import java.sql.Connection;
import java.sql.DriverManager;
import java.util.logging.FileHandler;
import java.util.logging.Logger;
import java.util.logging.SimpleFormatter;

class ReturnValue {
    public static final String FAIL = "failed: ";
    public static final String SUCCESS = "success";
}

public class MysqlConnector {
    public static final Logger logger = Logger.getLogger(MysqlConnector.class.getName());

    static {
        initLogger();
    }

    // 初始化日志
    private static void initLogger() {
        try {
            // 使用外部存储的应用私有目录
            File logDir = new File(Environment.getExternalStorageDirectory(), "Android/data/org.qtproject.example.password_recorder/logs");
            // File logDir = new File(context.getExternalFilesDir(null), "logs"); // 直接存储在应用专属文件夹中
            if (!logDir.exists()) {
                logDir.mkdirs(); // 如果目录不存在，创建它
            }

            // 创建日志文件
            File logFile = new File(logDir, "password-recorder-java.log");

            // 创建文件处理器，指定日志文件，并设置追加模式
            FileHandler fileHandler = new FileHandler(logFile.getAbsolutePath(), true);

            // 设置日志格式
            fileHandler.setFormatter(new SimpleFormatter());
            logger.addHandler(fileHandler);

            // 禁用父处理器，避免日志输出到控制台
            logger.setUseParentHandlers(false);
        } catch (IOException e) {
            e.printStackTrace();  // 处理日志文件创建异常
        }
    }

    // 记录异常日志的方法
    private void logException(Exception e) {
        logger.severe("An exception occurred: " + e.getMessage()); // 记录异常消息
        for (StackTraceElement element : e.getStackTrace()) {
            logger.severe(element.toString()); // 记录堆栈元素
        }
    }

    public String testConnect(String host, int port, String dbName, String user, String password) {
        Connection conn = null;

        try {
            // 加载 MySQL JDBC 驱动
            // Class.forName("com.mysql.jdbc.Driver");  // mysql 5.x
            Class.forName("com.mysql.cj.jdbc.Driver");  // mysql 6.x

            // 构建连接 URL，推荐加上字符编码和时区参数
            // String url = String.format("jdbc:mysql://%s:%d/%s?useUnicode=true&characterEncoding=utf8mb4&serverTimezone=UTC", host, port, dbName);
            String url = String.format("jdbc:mysql://%s:%d/%s?useSSL=false", host, port, dbName);
            // 建立连接
            conn = DriverManager.getConnection(url, user, password);

            if ( conn == null ) {
                return ReturnValue.FAIL + "connect to MySQL failed";
            }
        } catch (Exception e) {
            e.printStackTrace();
            // 将异常信息记录到日志中
            logException(e);
            return ReturnValue.FAIL + e.getMessage();
        } finally {
            // 确保连接关闭
            if (conn != null) {
                try {
                    conn.close();
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        }

        return ReturnValue.SUCCESS;
    }

    public int connectToMysql(String host, int port, String dbName, String user, String password) {
        return MysqlConnectionPool.initConnectionPool(host, port, dbName, user, password);
    }
}

class MysqlConnectionPool {

    private static LinkedList<Connection> connectionPool = new LinkedList<>();
    private static final int INITIAL_POOL_SIZE = 10;
    private static final int MAX_POOL_SIZE = 20;
    private static String dbUrl;
    private static String dbUser;
    private static String dbPwd;

    // 创建一个新的数据库连接
    private static Connection createNewConnection() throws SQLException {
        return DriverManager.getConnection(dbUrl, dbUser, dbPwd);
    }

    // 初始化连接池
    public static int initConnectionPool(String host, int port, String dbName, String user, String password) {
        dbUrl = String.format("jdbc:mysql://%s:%d/%s", host, port, dbName);
        dbUser = user;
        dbPwd = password;

        try {
            for (int i = 0; i < INITIAL_POOL_SIZE; i++) {
                connectionPool.add(createNewConnection());
            }
        } catch (SQLException e) {
            e.printStackTrace();
            return -1;
        }

        return 1;
    }


    // 从连接池获取连接
    public static synchronized Connection getConnection() throws SQLException {
        if (connectionPool.isEmpty()) {
            // 如果池为空，创建新连接
            if (connectionPool.size() < MAX_POOL_SIZE) {
                connectionPool.add(createNewConnection());
            } else {
                throw new SQLException("Connection pool has reached its maximum size.");
            }
        }

        return connectionPool.removeFirst();
    }

    // 将连接归还到连接池
    public static synchronized void returnConnection(Connection connection) {
        if (connection != null) {
            try {
                if (!connection.isClosed()) {
                    connectionPool.addLast(connection);
                } else {
                    // 可以选择重新创建连接
                }
            } catch (SQLException e) {
                e.printStackTrace();
            }
        }
    }

    // 关闭连接池中的所有连接
    public static synchronized void closeAllConnections() {
        while (!connectionPool.isEmpty()) {
            try {
                Connection conn = connectionPool.removeFirst();
                if (conn != null && !conn.isClosed()) {
                    conn.close();
                }
            } catch (SQLException e) {
                e.printStackTrace();
            }
        }
    }
}
