// databasemanager.h
#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QVector>
#include <QString>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include "book.h"

class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    static DatabaseManager& instance();

    // 数据库操作
    bool initializeDatabase();
    bool isDatabaseReady() const;
    QString getDatabasePath() const;

    // 图书相关操作
    bool addBook(const Book& book);
    bool updateBook(const Book& book);
    bool removeBook(const QString& indexId);
    QVector<Book> getAllBooks();
    Book getBookByIndexId(const QString& indexId);
    QVector<Book> searchBooks(const QString& keyword);
  QVector<Book> fuzzySearchByName(const QString& keyword);
  QVector<Book> fuzzySearchByIndexId(const QString& keyword);
    QVector<Book> getBooksByCategory(const QString& category);
    QVector<Book> getBooksByLocation(const QString& location);

    // 统计信息
    int getTotalBookCount();
    double getTotalInventoryValue();

    // 数据导入导出
    bool exportToJson(const QString& filePath);
    bool importFromJson(const QString& filePath);

private:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    bool loadFromFile();
    bool saveToFile();
    Book bookFromJson(const QJsonObject& obj);
    QJsonObject bookToJson(const Book& book);

private:
    QVector<Book> books_;
    QString dbFilePath_;
    bool isInitialized_;
};

#endif // DATABASEMANAGER_H
