#include "librarymanager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDate>
#include <QDir>
#include <algorithm>


LibraryManager::LibraryManager(QObject *parent)
    : QObject(parent)
{
}

void LibraryManager::clear() // <--- 添加这个函数的实现
{
    books_.clear();
}

// --- 数据操作 ---
bool LibraryManager::addBook(const Book &book, QString *error)
{
    if (findByIndexId(book.indexId)) {
        if (error) *error = QStringLiteral("索引号 '%1' 已存在").arg(book.indexId);
        return false;
    }
    books_.append(book);
    return true;
}

bool LibraryManager::updateBook(const QString &indexId, const Book &updatedBook, QString *error)
{
    for (int i = 0; i < books_.size(); ++i) {
        if (books_[i].indexId == indexId) {
            if (books_[i].indexId != updatedBook.indexId && findByIndexId(updatedBook.indexId)) {
                if (error) *error = QStringLiteral("新索引号 '%1' 已存在").arg(updatedBook.indexId);
                return false;
            }
            books_[i] = updatedBook;
            return true;
        }
    }
    if (error) *error = QStringLiteral("未找到索引号为 '%1' 的图书").arg(indexId);
    return false;
}

bool LibraryManager::removeBookByIndexId(const QString &indexId)
{
    for (int i = 0; i < books_.size(); ++i) {
        if (books_[i].indexId == indexId) {
            books_.removeAt(i);
            return true;
        }
    }
    return false;
}

const Book* LibraryManager::findByIndexId(const QString &indexId) const
{
    auto it = std::find_if(books_.begin(), books_.end(), [&](const Book& b) {
        return b.indexId == indexId;
    });
    return (it != books_.end()) ? &(*it) : nullptr;
}

const Book* LibraryManager::findByName(const QString &name) const
{
    auto it = std::find_if(books_.begin(), books_.end(), [&](const Book& b) {
        return b.name == name;
    });
    return (it != books_.end()) ? &(*it) : nullptr;
}

// --- 数据获取 ---
const QVector<Book>& LibraryManager::getAll() const
{
    return books_;
}

QVector<Book> LibraryManager::getByCategory(const QString &category) const
{
    QVector<Book> result;
    for(const auto& b : books_) {
        if (b.category == category) result.append(b);
    }
    return result;
}

QVector<Book> LibraryManager::getByLocation(const QString &location) const
{
    QVector<Book> result;
    for(const auto& b : books_) {
        if (b.location == location) result.append(b);
    }
    return result;
}

QVector<Book> LibraryManager::getAvailable() const
{
    QVector<Book> result;
    for(const auto& b : books_) {
        if (b.available) result.append(b);
    }
    return result;
}

QVector<Book> LibraryManager::getBorrowed() const
{
    QVector<Book> result;
    for(const auto& b : books_) {
        if (!b.available) result.append(b);
    }
    return result;
}

QVector<Book> LibraryManager::getDueInDays(int days) const
{
    QVector<Book> result;
    QDate futureDate = QDate::currentDate().addDays(days);
    for(const auto& b : books_) {
        if (!b.available && b.returnDate.isValid() && b.returnDate <= futureDate) {
            result.append(b);
        }
    }
    return result;
}

QVector<Book> LibraryManager::getTopBorrowed(int count) const
{
    QVector<Book> sortedBooks = books_;
    std::sort(sortedBooks.begin(), sortedBooks.end(), [](const Book &a, const Book &b) {
        return a.borrowCount > b.borrowCount;
    });
    if (sortedBooks.size() > count) {
        sortedBooks.resize(count);
    }
    return sortedBooks;
}

QVector<Book> LibraryManager::getRecentlyAdded(int days) const
{
    QVector<Book> result;
    QDate cutoffDate = QDate::currentDate().addDays(-days);
    for(const auto& b : books_) {
        if (b.inDate >= cutoffDate) {
            result.append(b);
        }
    }
    std::sort(result.begin(), result.end(), [](const Book &a, const Book &b) {
        return a.inDate > b.inDate;
    });
    return result;
}

QVector<Book> LibraryManager::getExpensiveBooks(double minPrice) const
{
    QVector<Book> result;
    for(const auto& b : books_) {
        if (b.price >= minPrice) result.append(b);
    }
    return result;
}

QVector<Book> LibraryManager::getCheapBooks(double maxPrice) const
{
    QVector<Book> result;
    for(const auto& b : books_) {
        if (b.price <= maxPrice) result.append(b);
    }
    return result;
}

QVector<Book> LibraryManager::searchBooks(const QString &keyword) const
{
    QVector<Book> result;
    for(const auto& b : books_) {
        if (b.name.contains(keyword, Qt::CaseInsensitive) ||
            b.category.contains(keyword, Qt::CaseInsensitive) ||
            b.location.contains(keyword, Qt::CaseInsensitive) ||
            b.indexId.contains(keyword, Qt::CaseInsensitive)) {
            result.append(b);
        }
    }
    return result;
}

// --- 统计信息 ---
int LibraryManager::getTotalBooks() const { return books_.size(); }
int LibraryManager::getAvailableBooks() const { return getAvailable().size(); }
int LibraryManager::getBorrowedBooks() const { return getBorrowed().size(); }

double LibraryManager::getTotalValue() const
{
    double total = 0.0;
    for(const auto& b : books_) {
        total += b.price * b.quantity;
    }
    return total;
}

QString LibraryManager::getMostPopularCategory() const
{
    QHash<QString, int> categoryCount;
    for(const auto& b : books_) {
        categoryCount[b.category]++;
    }
    QString popular;
    int maxCount = 0;
    for(auto it = categoryCount.constBegin(); it != categoryCount.constEnd(); ++it) {
        if (it.value() > maxCount) {
            maxCount = it.value();
            popular = it.key();
        }
    }
    return popular;
}

QString LibraryManager::getMostPopularLocation() const
{
    QHash<QString, int> locationCount;
    for(const auto& b : books_) {
        locationCount[b.location]++;
    }
    QString popular;
    int maxCount = 0;
    for(auto it = locationCount.constBegin(); it != locationCount.constEnd(); ++it) {
        if (it.value() > maxCount) {
            maxCount = it.value();
            popular = it.key();
        }
    }
    return popular;
}

// --- 排序 ---
void LibraryManager::sortByName() { std::sort(books_.begin(), books_.end(), [](const Book&a, const Book&b){ return a.name < b.name; }); }
void LibraryManager::sortByCategory() { std::sort(books_.begin(), books_.end(), [](const Book&a, const Book&b){ return a.category < b.category; }); }
void LibraryManager::sortByLocation() { std::sort(books_.begin(), books_.end(), [](const Book&a, const Book&b){ return a.location < b.location; }); }
void LibraryManager::sortByPrice() { std::sort(books_.begin(), books_.end(), [](const Book&a, const Book&b){ return a.price > b.price; }); }
void LibraryManager::sortByDate() { std::sort(books_.begin(), books_.end(), [](const Book&a, const Book&b){ return a.inDate > b.inDate; }); }
void LibraryManager::sortByBorrowCount() { std::sort(books_.begin(), books_.end(), [](const Book&a, const Book&b){ return a.borrowCount > b.borrowCount; }); }

// --- 文件操作 ---
bool LibraryManager::loadFromFile(const QString &filePath, QString *error)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        if (error) *error = file.errorString();
        return false;
    }

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isArray()) {
        if (error) *error = "Invalid JSON format, expected an array.";
        return false;
    }

    books_.clear();
    QJsonArray jsonArray = doc.array();
    for (const QJsonValue &value : jsonArray) {
        if (value.isObject()) {
            books_.append(fromJson(value.toObject()));
        }
    }
    return true;
}

bool LibraryManager::saveToFile(const QString &filePath, QString *error)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        if (error) *error = file.errorString();
        return false;
    }

    QJsonArray jsonArray;
    for (const Book &b : books_) {
        QJsonObject obj;
        toJson(obj, b);
        jsonArray.append(obj);
    }

    QJsonDocument doc(jsonArray);
    file.write(doc.toJson());
    return true;
}
