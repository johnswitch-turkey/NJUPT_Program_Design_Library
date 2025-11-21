/**
 * @file book.h
 * @brief 图书信息数据结构定义文件
 *
 * 本文件定义了图书管理系统中的核心数据结构Book，以及相关的JSON序列化/反序列化函数。
 * Book结构体包含了图书的基本信息，如索引号、书名、作者、出版社等。
 *
 * @author 开发团队
 * @date 2023-2024
 * @version 1.0
 */

#ifndef BOOK_H
#define BOOK_H

#include <QString>
#include <QDate>
#include <QJsonObject>

/**
 * @struct Book
 * @brief 图书信息数据结构
 *
 * 该结构体定义了图书管理系统中图书对象的完整信息模型。
 * 包含了图书的基本属性，如索引号、书名、作者、出版社、价格等。
 * 同时支持内容简介和借阅统计等功能。
 *
 * @note 所有字符串字段都使用UTF-8编码的QString类型
 * @note 价格字段使用double类型，保留两位小数
 * @note 借阅次数默认为0，每次借阅时递增
 */
struct Book {
    QString indexId;          ///< 图书唯一索引号，格式：类别前缀+数字编号（如：CS001）
    QString name;             ///< 图书书名，支持完整书名
    QString author;           ///< 图书作者，可以是个人作者或机构作者
    QString publisher;        ///< 图书出版社，记录出版机构名称
    QString location;         ///< 馆藏地址，支持多校区管理（可选值：三牌楼/仙林）
    QString category;         ///< 图书分类，如人文、科技、外语、艺术等
    double price = 0.0;       ///< 图书价格，单位：人民币元，默认值为0.0
    QDate inDate;             ///< 图书入库日期，记录图书添加到系统的日期
    int borrowCount = 0;      ///< 图书借阅次数，统计该图书被借阅的总次数，默认为0
    QString description;      ///< 图书内容简介，详细描述图书内容、特色和适用人群
};

/**
 * @brief 将Book对象序列化为JSON对象
 *
 * 该函数将Book结构体的所有字段转换为QJsonObject对象，
 * 用于数据持久化存储和网络传输。日期类型转换为ISO格式的字符串。
 *
 * @param obj 输出的JSON对象引用，用于存储序列化后的数据
 * @param b 要序列化的Book对象常量引用
 *
 * @note 日期使用Qt::ISODate格式进行转换，确保跨平台兼容性
 * @note 数值类型会自动转换为JSON对应的数值类型
 * @see fromJson() 反序列化函数
 */
inline void toJson(QJsonObject &obj, const Book &b)
{
    obj["indexId"] = b.indexId;        ///< 图书索引号，字符串类型
    obj["name"] = b.name;              ///< 图书书名，字符串类型
    obj["author"] = b.author;          ///< 图书作者，字符串类型
    obj["publisher"] = b.publisher;    ///< 图书出版社，字符串类型
    obj["location"] = b.location;      ///< 馆藏地址，字符串类型
    obj["category"] = b.category;      ///< 图书分类，字符串类型
    obj["price"] = b.price;            ///< 图书价格，数值类型
    obj["inDate"] = b.inDate.toString(Qt::ISODate);  ///< 入库日期，转换为ISO格式字符串
    obj["borrowCount"] = b.borrowCount;///< 借阅次数，整数类型
    obj["description"] = b.description;///< 内容简介，字符串类型
}

/**
 * @brief 从JSON对象反序列化为Book对象
 *
 * 该函数从QJsonObject对象中提取数据并构建Book结构体，
 * 用于从持久化存储或网络数据中恢复图书信息。ISO格式的日期字符串会转换回QDate对象。
 *
 * @param obj 输入的JSON对象常量引用，包含序列化的Book数据
 * @return 返回构建的Book对象
 *
 * @note 如果JSON对象中缺少某些字段，对应的Book字段将使用默认值
 * @note 日期字符串解析失败时，QDate会返回无效日期
 * @note 数值类型转换失败时，会返回0或默认值
 * @see toJson() 序列化函数
 */
inline Book fromJson(const QJsonObject &obj)
{
    Book b;
    b.indexId = obj.value("indexId").toString();        ///< 从JSON获取索引号
    b.name = obj.value("name").toString();              ///< 从JSON获取书名
    b.author = obj.value("author").toString();          ///< 从JSON获取作者
    b.publisher = obj.value("publisher").toString();    ///< 从JSON获取出版社
    b.location = obj.value("location").toString();      ///< 从JSON获取馆藏地址
    b.category = obj.value("category").toString();      ///< 从JSON获取分类
    b.price = obj.value("price").toDouble();            ///< 从JSON获取价格
    b.inDate = QDate::fromString(obj.value("inDate").toString(), Qt::ISODate);  ///< 从JSON解析日期
    b.borrowCount = obj.value("borrowCount").toInt();   ///< 从JSON获取借阅次数
    b.description = obj.value("description").toString();///< 从JSON获取内容简介
    return b;
}

#endif // BOOK_H


