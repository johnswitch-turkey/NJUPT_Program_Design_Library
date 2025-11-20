// librarymanager.h
#ifndef LIBRARYMANAGER_H
#define LIBRARYMANAGER_H

#include <QObject>
#include <QVector>
#include <QHash>
#include <QString>
#include "book.h"
#include "./databasemanager.h"
#include "./bookcopymanager.h"

class LibraryManager : public QObject
{
    Q_OBJECT

public:
    static LibraryManager& instance();

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
    QVector<Book> searchBooks(const QString &keyword) const;
    QVector<Book> getWarn(int days) const;

    // --- 副本管理 ---
    bool addBookCopies(const QString &indexId, int count, QString *error = nullptr);
    bool removeBookCopy(const QString &copyId, QString *error = nullptr);
    QVector<BookCopy> getBookCopies(const QString &indexId) const;
    QVector<BookCopy> getAvailableCopies(const QString &indexId) const;
    BookCopy getFirstAvailableCopy(const QString &indexId) const;
    int getTotalCopyCount(const QString &indexId) const;
    int getAvailableCopyCount(const QString &indexId) const;

    // --- 借阅相关 ---
    bool borrowBook(const QString &indexId, const QString &username, const QDate &dueDate, QString *error = nullptr);
    bool returnBook(const QString &copyId, const QString &username, QString *error = nullptr);
    QVector<BookCopy> getUserBorrowedCopies(const QString &username) const;
    QVector<BookCopy> getDueSoonCopies(int days) const;

    // --- 统计信息 ---
    int getTotalBooks() const;
    int getTotalCopies() const;
    int getAvailableCopies() const;
    int getBorrowedCopies() const;
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

    // --- 数据库操作 ---
    bool loadFromDatabase();
    bool saveToDatabase();
    bool importSampleData();
    bool exportToJson(const QString& filePath);
    bool importFromJson(const QString& filePath);

    signals:
        void dataChanged();  // 确保有这个信号声明

private:
    void refreshFromDatabase();
    QVector<Book> books_;
    DatabaseManager& dbManager_;
    BookCopyManager& copyManager_;
};

#endif // LIBRARYMANAGER_H

