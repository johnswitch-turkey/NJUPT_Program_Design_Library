// bookcopy.h
#ifndef BOOKCOPY_H
#define BOOKCOPY_H

#include <QString>
#include <QDate>

struct BookCopy {
    QString copyId;           // 副本唯一标识 (格式: indexId_copyNumber)
    QString indexId;          // 图书索引号
    int copyNumber;           // 编号 (从1开始)
    QString borrowedBy;       // 借阅者用户名 (空表示未被借阅)
    QDate borrowDate;         // 借阅日期
    QDate dueDate;           // 归还日期
    bool isAvailable() const { return borrowedBy.isEmpty(); }

    QJsonObject toJson() const;
    static BookCopy fromJson(const QJsonObject &obj);
};

inline QJsonObject BookCopy::toJson() const {
    QJsonObject obj;
    obj["copyId"] = copyId;
    obj["indexId"] = indexId;
    obj["copyNumber"] = copyNumber;
    obj["borrowedBy"] = borrowedBy;
    obj["borrowDate"] = borrowDate.isValid() ? borrowDate.toString(Qt::ISODate) : QString();
    obj["dueDate"] = dueDate.isValid() ? dueDate.toString(Qt::ISODate) : QString();
    return obj;
}

inline BookCopy BookCopy::fromJson(const QJsonObject &obj) {
    BookCopy copy;
    copy.copyId = obj.value("copyId").toString();
    copy.indexId = obj.value("indexId").toString();
    copy.copyNumber = obj.value("copyNumber").toInt();
    copy.borrowedBy = obj.value("borrowedBy").toString();
    copy.borrowDate = obj.value("borrowDate").toString().isEmpty() ?
                      QDate() : QDate::fromString(obj.value("borrowDate").toString(), Qt::ISODate);
    copy.dueDate = obj.value("dueDate").toString().isEmpty() ?
                   QDate() : QDate::fromString(obj.value("dueDate").toString(), Qt::ISODate);
    return copy;
}

#endif // BOOKCOPY_H
//
// Created by 13913 on 2025/11/19.
//

#ifndef CLION_TEST_BOOKCOPY_H
#define CLION_TEST_BOOKCOPY_H

#endif //CLION_TEST_BOOKCOPY_H