// bookcopymanager.h
#ifndef BOOKCOPYMANAGER_H
#define BOOKCOPYMANAGER_H

#include <QObject>
#include <QVector>
#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include "bookcopy.h"

class BookCopyManager : public QObject
{
    Q_OBJECT

public:
    static BookCopyManager& instance();

    // 初始化数据库
    bool initializeDatabase();
    bool isDatabaseReady() const;
    QString getDatabasePath() const;

    // 副本操作
    bool addCopy(const BookCopy &copy);
    bool removeCopy(const QString &copyId);
    bool updateCopy(const BookCopy &copy);
    QVector<BookCopy> getAllCopies() const;
    QVector<BookCopy> getCopiesByIndexId(const QString &indexId) const;
    BookCopy getCopyById(const QString &copyId) const;

    // 获取可用的副本（从小到大编号）
    QVector<BookCopy> getAvailableCopies(const QString &indexId) const;
    BookCopy getFirstAvailableCopy(const QString &indexId) const;

    // 借阅相关
    bool borrowCopy(const QString &copyId, const QString &username, const QDate &dueDate);
    bool returnCopy(const QString &copyId);
    bool renewCopy(const QString &copyId, int extendDays = 30);  // 新增：续借功能，默认续借30天
    QVector<BookCopy> getBorrowedCopies(const QString &username) const;
    QVector<BookCopy> getDueSoonCopies(int days) const;

    // 统计信息
    int getTotalCopyCount(const QString &indexId) const;
    int getAvailableCopyCount(const QString &indexId) const;
    int getBorrowedCopyCount(const QString &indexId) const;

    // 获取下一个编号
    int getNextCopyNumber(const QString &indexId) const;

private:
    explicit BookCopyManager(QObject *parent = nullptr);
    ~BookCopyManager();
    BookCopyManager(const BookCopyManager&) = delete;
    BookCopyManager& operator=(const BookCopyManager&) = delete;

    bool loadFromFile();
    bool saveToFile();

    QVector<BookCopy> copies_;
    QString dbFilePath_;
    bool isInitialized_;
};

#endif // BOOKCOPYMANAGER_H
//
// Created by 13913 on 2025/11/19.
//

#ifndef CLION_TEST_BOOKCOPYMANAGER_H
#define CLION_TEST_BOOKCOPYMANAGER_H


class bookcopymanager
{
};


#endif //CLION_TEST_BOOKCOPYMANAGER_H