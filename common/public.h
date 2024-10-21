#ifndef PUBLIC_H
#define PUBLIC_H

#include <QObject>
#include <QOperatingSystemVersion>

// 使用 C++17 或更高版本，可以使用 inline 关键字来定义全局常量。这种方法可以将常量的定义放在头文件中而不会引发重复定义的错误
inline const QString RETURN_SUCCESS_STR = "Success";
const bool OS_ANDROID = (QOperatingSystemVersion::currentType() == QOperatingSystemVersion::Android);

inline const QString JSON_KEY_RETURN = "JavaResult";
inline const QString JSON_KEY_RESULT = "MysqlValue";

#endif // PUBLIC_H
