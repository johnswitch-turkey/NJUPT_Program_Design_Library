// book.h
#ifndef BOOK_H
#define BOOK_H

#include <QString>
#include <QDate>
#include <QJsonObject>

// 图书信息数据结构
struct Book {
    QString indexId;          // 索引号
    QString name;             // 名称
    QString author;           // 作者
    QString publisher;        // 出版社
    QString location;         // 馆藏地址：三牌楼/仙林
    QString category;         // 类别：人文/科技/外语等
    double price = 0.0;       // 价格
    QDate inDate;             // 入库日期
    int borrowCount = 0;      // 借阅次数

    // 移除以下字段，由BookCopy管理
    // int quantity = 0;         // 数量
    // QDate returnDate;         // 归还日期（若未借出可为空）
    // bool available = true;    // 借阅状态：可借/不可借
};

inline void toJson(QJsonObject &obj, const Book &b)
{
    obj["indexId"] = b.indexId;
    obj["name"] = b.name;
    obj["author"] = b.author;
    obj["publisher"] = b.publisher;
    obj["location"] = b.location;
    obj["category"] = b.category;
    obj["price"] = b.price;
    obj["inDate"] = b.inDate.toString(Qt::ISODate);
    obj["borrowCount"] = b.borrowCount;
}

inline Book fromJson(const QJsonObject &obj)
{
    Book b;
    b.indexId = obj.value("indexId").toString();
    b.name = obj.value("name").toString();
    b.author = obj.value("author").toString();
    b.publisher = obj.value("publisher").toString();
    b.location = obj.value("location").toString();
    b.category = obj.value("category").toString();
    b.price = obj.value("price").toDouble();
    b.inDate = QDate::fromString(obj.value("inDate").toString(), Qt::ISODate);
    b.borrowCount = obj.value("borrowCount").toInt();
    return b;
}

#endif // BOOK_H


