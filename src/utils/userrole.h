//
// 用户与角色相关的简单结构体定义
//

#ifndef USERROLE_H
#define USERROLE_H

#include <QString>
#include <QStringList>

struct UserInfo {
    QString username;
    QString password;
    QString role;                 // "admin" 或 "student"
    QStringList allowedCategories; // 学生可借的图书类别列表，空表示不限制
};

#endif // USERROLE_H


