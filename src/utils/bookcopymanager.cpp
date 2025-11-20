// bookcopymanager.cpp
#include "bookcopymanager.h"
#include <QDebug>
#include <QDir>
#include <QCoreApplication>

BookCopyManager& BookCopyManager::instance()
{
    static BookCopyManager instance;
    return instance;
}

BookCopyManager::BookCopyManager(QObject *parent)
    : QObject(parent), isInitialized_(false)
{
    initializeDatabase();
}

BookCopyManager::~BookCopyManager()
{
    saveToFile();
}

bool BookCopyManager::initializeDatabase()
{
    QString appDirPath = QCoreApplication::applicationDirPath();
    QDir targetDir(appDirPath);

    bool success = targetDir.cdUp();
    if (success) {
        targetDir.cd("src");
        targetDir.cd("resource");
    } else {
        targetDir = QDir(appDirPath);
        targetDir.cd("resource");
    }

    QString absoluteTargetPath = targetDir.absolutePath();
    if (!targetDir.exists()) {
        targetDir.mkpath(absoluteTargetPath);
    }

    dbFilePath_ = absoluteTargetPath + "/book_copies.json";
    qDebug() << "Book copies database file path:" << dbFilePath_;

    if (QFile::exists(dbFilePath_)) {
        if (loadFromFile()) {
            qDebug() << "Book copies database loaded successfully with" << copies_.size() << "copies";
            isInitialized_ = true;
            return true;
        }
    }

    copies_.clear();
    if (saveToFile()) {
        qDebug() << "New book copies database created successfully";
        isInitialized_ = true;
        return true;
    }

    qDebug() << "Failed to create book copies database";
    return false;
}

bool BookCopyManager::isDatabaseReady() const
{
    return isInitialized_;
}

QString BookCopyManager::getDatabasePath() const
{
    return dbFilePath_;
}

bool BookCopyManager::loadFromFile()
{
    QFile file(dbFilePath_);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Cannot open book copies database file for reading:" << file.errorString();
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isArray()) {
        qDebug() << "Invalid book copies database format: expected JSON array";
        return false;
    }

    copies_.clear();
    QJsonArray jsonArray = doc.array();
    for (const QJsonValue &value : jsonArray) {
        if (value.isObject()) {
            copies_.append(BookCopy::fromJson(value.toObject()));
        }
    }

    qDebug() << "Loaded" << copies_.size() << "book copies from database";
    return true;
}

bool BookCopyManager::saveToFile()
{
    QFile file(dbFilePath_);
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "Cannot open book copies database file for writing:" << file.errorString();
        return false;
    }

    QJsonArray jsonArray;
    for (const BookCopy &copy : copies_) {
        jsonArray.append(copy.toJson());
    }

    QJsonDocument doc(jsonArray);
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    qDebug() << "Saved" << copies_.size() << "book copies to database";
    return true;
}

bool BookCopyManager::addCopy(const BookCopy &copy)
{
    for (const BookCopy &existingCopy : copies_) {
        if (existingCopy.copyId == copy.copyId) {
            qDebug() << "Book copy with ID" << copy.copyId << "already exists";
            return false;
        }
    }

    copies_.append(copy);
    return saveToFile();
}

bool BookCopyManager::removeCopy(const QString &copyId)
{
    for (int i = 0; i < copies_.size(); ++i) {
        if (copies_[i].copyId == copyId) {
            copies_.removeAt(i);
            return saveToFile();
        }
    }

    qDebug() << "Book copy with ID" << copyId << "not found for removal";
    return false;
}

bool BookCopyManager::updateCopy(const BookCopy &copy)
{
    for (int i = 0; i < copies_.size(); ++i) {
        if (copies_[i].copyId == copy.copyId) {
            copies_[i] = copy;
            return saveToFile();
        }
    }

    qDebug() << "Book copy with ID" << copy.copyId << "not found for update";
    return false;
}

QVector<BookCopy> BookCopyManager::getAllCopies() const
{
    return copies_;
}

QVector<BookCopy> BookCopyManager::getCopiesByIndexId(const QString &indexId) const
{
    QVector<BookCopy> result;
    for (const BookCopy &copy : copies_) {
        if (copy.indexId == indexId) {
            result.append(copy);
        }
    }
    return result;
}

BookCopy BookCopyManager::getCopyById(const QString &copyId) const
{
    for (const BookCopy &copy : copies_) {
        if (copy.copyId == copyId) {
            return copy;
        }
    }
    return BookCopy();
}

QVector<BookCopy> BookCopyManager::getAvailableCopies(const QString &indexId) const
{
    QVector<BookCopy> result;
    QVector<BookCopy> allCopies = getCopiesByIndexId(indexId);

    std::sort(allCopies.begin(), allCopies.end(), [](const BookCopy &a, const BookCopy &b) {
        return a.copyNumber < b.copyNumber;
    });

    for (const BookCopy &copy : allCopies) {
        if (copy.isAvailable()) {
            result.append(copy);
        }
    }

    return result;
}

BookCopy BookCopyManager::getFirstAvailableCopy(const QString &indexId) const
{
    QVector<BookCopy> availableCopies = getAvailableCopies(indexId);
    return availableCopies.isEmpty() ? BookCopy() : availableCopies.first();
}

bool BookCopyManager::borrowCopy(const QString &copyId, const QString &username, const QDate &dueDate)
{
    for (int i = 0; i < copies_.size(); ++i) {
        if (copies_[i].copyId == copyId && copies_[i].isAvailable()) {
            copies_[i].borrowedBy = username;
            copies_[i].borrowDate = QDate::currentDate();
            copies_[i].dueDate = dueDate;
            return saveToFile();
        }
    }
    return false;
}

bool BookCopyManager::returnCopy(const QString &copyId)
{
    for (int i = 0; i < copies_.size(); ++i) {
        if (copies_[i].copyId == copyId) {
            copies_[i].borrowedBy = QString();
            copies_[i].borrowDate = QDate();
            copies_[i].dueDate = QDate();
            return saveToFile();
        }
    }
    return false;
}

QVector<BookCopy> BookCopyManager::getBorrowedCopies(const QString &username) const
{
    QVector<BookCopy> result;
    for (const BookCopy &copy : copies_) {
        if (copy.borrowedBy == username) {
            result.append(copy);
        }
    }
    return result;
}

QVector<BookCopy> BookCopyManager::getDueSoonCopies(int days) const
{
    QVector<BookCopy> result;
    QDate futureDate = QDate::currentDate().addDays(days);

    for (const BookCopy &copy : copies_) {
        if (!copy.isAvailable() && copy.dueDate.isValid() && copy.dueDate <= futureDate) {
            result.append(copy);
        }
    }
    return result;
}

int BookCopyManager::getTotalCopyCount(const QString &indexId) const
{
    return getCopiesByIndexId(indexId).size();
}

int BookCopyManager::getAvailableCopyCount(const QString &indexId) const
{
    return getAvailableCopies(indexId).size();
}

int BookCopyManager::getBorrowedCopyCount(const QString &indexId) const
{
    return getTotalCopyCount(indexId) - getAvailableCopyCount(indexId);
}

int BookCopyManager::getNextCopyNumber(const QString &indexId) const
{
    QVector<BookCopy> copies = getCopiesByIndexId(indexId);
    if (copies.isEmpty()) {
        return 1;
    }

    int maxNumber = 0;
    for (const BookCopy &copy : copies) {
        if (copy.copyNumber > maxNumber) {
            maxNumber = copy.copyNumber;
        }
    }

    return maxNumber + 1;
}
