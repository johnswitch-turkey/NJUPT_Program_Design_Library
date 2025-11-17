#ifndef LIBRARYMANAGER_H
#define LIBRARYMANAGER_H

#include <QObject>
#include <QVector>
#include <QHash>
#include <QString>
#include "book.h"

class LibraryManager : public QObject
{
    Q_OBJECT

public:
    explicit LibraryManager(QObject *parent = nullptr);

    void clear();
    // --- 数据操作 ---
    bool addBook(const Book &book, QString *error = nullptr);
    bool updateBook(const QString &indexId, const Book &updatedBook, QString *error = nullptr);
    bool removeBookByIndexId(const QString &indexId);
    const Book* findByIndexId(const QString &indexId) const;
    const Book* findByName(const QString &name) const;

    // --- 数据获取 ---
    const QVector<Book>& getAll() const;
    QVector<Book> getByCategory(const QString &category) const;
    QVector<Book> getByLocation(const QString &location) const;
    QVector<Book> getAvailable() const;
    QVector<Book> getBorrowed() const;
    QVector<Book> getDueInDays(int days) const;
    QVector<Book> getTopBorrowed(int count) const;
    QVector<Book> getRecentlyAdded(int days) const;
    QVector<Book> getExpensiveBooks(double minPrice) const;
    QVector<Book> getCheapBooks(double maxPrice) const;
    QVector<Book> searchBooks(const QString &keyword) const;

    // --- 统计信息 ---
    int getTotalBooks() const;
    int getAvailableBooks() const;
    int getBorrowedBooks() const;
    double getTotalValue() const;
    QString getMostPopularCategory() const;
    QString getMostPopularLocation() const;

    // --- 排序 ---
    void sortByName();
    void sortByCategory();
    void sortByLocation();
    void sortByPrice(); // 高到低
    void sortByDate(); // 新到旧
    void sortByBorrowCount(); // 高到低

    // --- 文件操作 ---
    bool loadFromFile(const QString &filePath, QString *error = nullptr);
    bool saveToFile(const QString &filePath, QString *error = nullptr);

private:
    QVector<Book> books_;
};

#endif // LIBRARYMANAGER_H
