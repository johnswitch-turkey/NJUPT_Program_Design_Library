//
// 用户与角色相关的简单结构体定义
//

#ifndef USERROLE_H
#define USERROLE_H

#include <QString>

struct UserInfo {
    QString username;
    QString password;
    QString role;                 // "admin" 或 "student"
};

#endif // USERROLE_H


