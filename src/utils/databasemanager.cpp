// databasemanager.cpp
#include "databasemanager.h"
#include <QDebug>
#include <QDate>
#include <QDir>
#include <QStandardPaths>
#include <QApplication>

// 单例实例
DatabaseManager& DatabaseManager::instance()
{
    static DatabaseManager instance;
    return instance;
}

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent)
    , isInitialized_(false)
{
    initializeDatabase();
}

DatabaseManager::~DatabaseManager()
{
    saveToFile();
}

bool DatabaseManager::initializeDatabase()
{
    QString appDirPath = QCoreApplication::applicationDirPath();
    QDir targetDir(appDirPath);

    // 进入上一级目录的src/resource
    bool success = targetDir.cdUp();
    if (success) {
        targetDir.cd("src");
        targetDir.cd("resource");
    } else {
        // 如果cdUp()失败，使用可执行文件目录下的resource
        targetDir = QDir(appDirPath);
        targetDir.cd("resource");
    }

    // 确保resource目录存在
    QString absoluteTargetPath = targetDir.absolutePath();
    if (!targetDir.exists()) {
        targetDir.mkpath(absoluteTargetPath);
    }

    dbFilePath_ = absoluteTargetPath + "/library_data.json";
    qDebug() << "Database file path:" << dbFilePath_;
    
    // 尝试加载现有数据
    if (QFile::exists(dbFilePath_)) {
        if (loadFromFile()) {
            qDebug() << "Database loaded successfully with" << books_.size() << "books";
            isInitialized_ = true;
            return true;
        } else {
            qDebug() << "Failed to load existing database, creating new one";
        }
    }

    // 创建新的空数据库
    books_.clear();
    if (saveToFile()) {
        qDebug() << "New database created successfully";
        isInitialized_ = true;
        return true;
    }

    qDebug() << "Failed to create database";
    return false;
}

bool DatabaseManager::isDatabaseReady() const
{
    return isInitialized_;
}

QString DatabaseManager::getDatabasePath() const
{
    return dbFilePath_;
}

bool DatabaseManager::loadFromFile()
{
    QFile file(dbFilePath_);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Cannot open database file for reading:" << file.errorString();
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isArray()) {
        qDebug() << "Invalid database format: expected JSON array";
        return false;
    }

    books_.clear();
    QJsonArray jsonArray = doc.array();
    for (const QJsonValue &value : jsonArray) {
        if (value.isObject()) {
            books_.append(bookFromJson(value.toObject()));
        }
    }

    qDebug() << "Loaded" << books_.size() << "books from database";
    return true;
}

bool DatabaseManager::saveToFile()
{
    QFile file(dbFilePath_);
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "Cannot open database file for writing:" << file.errorString();
        return false;
    }

    QJsonArray jsonArray;
    for (const Book &book : books_) {
        jsonArray.append(bookToJson(book));
    }

    QJsonDocument doc(jsonArray);
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    qDebug() << "Saved" << books_.size() << "books to database";
    return true;
}

Book DatabaseManager::bookFromJson(const QJsonObject& obj)
{
    Book book;
    book.indexId = obj.value("indexId").toString();
    book.name = obj.value("name").toString();
    book.author = obj.value("author").toString();
    book.publisher = obj.value("publisher").toString();
    book.location = obj.value("location").toString();
    book.category = obj.value("category").toString();
    book.price = obj.value("price").toDouble();

    // 修改 inDate 的读取逻辑
    QString inDateStr = obj.value("inDate").toString();
    if (inDateStr.isEmpty()) {
        // 如果日期字符串为空，使用当前日期作为默认值
        book.inDate = QDate::currentDate();
    } else {
        QDate parsedDate = QDate::fromString(inDateStr, Qt::ISODate);
        book.inDate = parsedDate.isValid() ? parsedDate : QDate::currentDate();
    }

    book.borrowCount = obj.value("borrowCount").toInt();

    return book;
}

QJsonObject DatabaseManager::bookToJson(const Book& book)
{
    QJsonObject obj;
    obj["indexId"] = book.indexId;
    obj["name"] = book.name;
    obj["author"] = book.author;
    obj["publisher"] = book.publisher;
    obj["location"] = book.location;
    obj["category"] = book.category;
    obj["price"] = book.price;

    // 确保 inDate 总是被有效存储
    obj["inDate"] = book.inDate.isValid() ? book.inDate.toString(Qt::ISODate) : QDate::currentDate().toString(Qt::ISODate);

    obj["borrowCount"] = book.borrowCount;

    return obj;
}

bool DatabaseManager::addBook(const Book& book)
{
    // 检查索引号是否已存在
    for (const Book& existingBook : books_) {
        if (existingBook.indexId == book.indexId) {
            qDebug() << "Book with indexId" << book.indexId << "already exists";
            return false;
        }
    }

    // 确保日期有效
    Book validBook = book;
    if (!validBook.inDate.isValid()) {
        validBook.inDate = QDate::currentDate();
        qDebug() << "Invalid inDate for book" << book.indexId << ", using current date";
    }

    books_.append(validBook);
    return saveToFile();
}

bool DatabaseManager::updateBook(const Book& book)
{
    for (int i = 0; i < books_.size(); ++i) {
        if (books_[i].indexId == book.indexId) {
            // 确保日期有效
            Book validBook = book;
            if (!validBook.inDate.isValid()) {
                validBook.inDate = QDate::currentDate();
                qDebug() << "Invalid inDate for book" << book.indexId << ", using current date";
            }

            books_[i] = validBook;
            return saveToFile();
        }
    }

    qDebug() << "Book with indexId" << book.indexId << "not found for update";
    return false;
}

bool DatabaseManager::removeBook(const QString& indexId)
{
    for (int i = 0; i < books_.size(); ++i) {
        if (books_[i].indexId == indexId) {
            books_.removeAt(i);
            return saveToFile();
        }
    }

    qDebug() << "Book with indexId" << indexId << "not found for removal";
    return false;
}

QVector<Book> DatabaseManager::getAllBooks()
{
    return books_;
}

Book DatabaseManager::getBookByIndexId(const QString& indexId)
{
    for (const Book& book : books_) {
        if (book.indexId == indexId) {
            return book;
        }
    }

    return Book(); // 返回空的Book对象
}

QVector<Book> DatabaseManager::searchBooks(const QString& keyword)
{
    QVector<Book> result;
    QString lowerKeyword = keyword.toLower();

    for (const Book& book : books_) {
        if (book.name.toLower().contains(lowerKeyword) ||
            book.category.toLower().contains(lowerKeyword) ||
            book.location.toLower().contains(lowerKeyword) ||
            book.indexId.toLower().contains(lowerKeyword)) {
            result.append(book);
        }
    }

    return result;
}

QVector<Book> DatabaseManager::getBooksByCategory(const QString& category)
{
    QVector<Book> result;

    for (const Book& book : books_) {
        if (book.category == category) {
            result.append(book);
        }
    }

    return result;
}

QVector<Book> DatabaseManager::getBooksByLocation(const QString& location)
{
    QVector<Book> result;

    for (const Book& book : books_) {
        if (book.location == location) {
            result.append(book);
        }
    }

    return result;
}

int DatabaseManager::getTotalBookCount()
{
    return books_.size();
}

double DatabaseManager::getTotalInventoryValue()
{
    double total = 0.0;
    for (const Book& book : books_) {
        total += book.price;
    }
    return total;
}

bool DatabaseManager::exportToJson(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "Cannot open file for export:" << file.errorString();
        return false;
    }

    QJsonArray jsonArray;
    for (const Book &book : books_) {
        jsonArray.append(bookToJson(book));
    }

    QJsonDocument doc(jsonArray);
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    qDebug() << "Exported" << books_.size() << "books to" << filePath;
    return true;
}

bool DatabaseManager::importFromJson(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Cannot open file for import:" << file.errorString();
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isArray()) {
        qDebug() << "Invalid import file format: expected JSON array";
        return false;
    }

    QVector<Book> importedBooks;
    QJsonArray jsonArray = doc.array();
    for (const QJsonValue &value : jsonArray) {
        if (value.isObject()) {
            importedBooks.append(bookFromJson(value.toObject()));
        }
    }

    // 添加到现有数据库（跳过重复的indexId）
    int addedCount = 0;
    for (const Book& book : importedBooks) {
        bool exists = false;
        for (const Book& existingBook : books_) {
            if (existingBook.indexId == book.indexId) {
                exists = true;
                break;
            }
        }

        if (!exists) {
            books_.append(book);
            addedCount++;
        }
    }

    if (saveToFile()) {
        qDebug() << "Imported" << addedCount << "new books from" << filePath;
        return true;
    }

    return false;
}
