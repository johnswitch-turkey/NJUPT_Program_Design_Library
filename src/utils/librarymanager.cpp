// librarymanager.cpp
#include "librarymanager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDate>
#include <QDir>
#include <algorithm>

#include "./databasemanager.h"
#include "./bookcopymanager.h"
LibraryManager& LibraryManager::instance()
{
    static LibraryManager instance;
    return instance;
}
LibraryManager::LibraryManager(QObject *parent)
    : QObject(parent)
    , dbManager_(DatabaseManager::instance())
    , copyManager_(BookCopyManager::instance())
{
    loadFromDatabase();

    // 如果数据库为空，自动导入示例数据
    if (books_.isEmpty()) {
        importSampleData();
    }
};


void LibraryManager::clear()
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

    if (!dbManager_.addBook(book)) {
        if (error) *error = QStringLiteral("数据库添加失败");
        return false;
    }

    books_.append(book);

    // 添加一个默认副本
    BookCopy copy;
    copy.copyId = book.indexId + "_1";
    copy.indexId = book.indexId;
    copy.copyNumber = 1;
    copyManager_.addCopy(copy);

    emit dataChanged();
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

            if (!dbManager_.updateBook(updatedBook)) {
                if (error) *error = QStringLiteral("数据库更新失败");
                return false;
            }

            books_[i] = updatedBook;
            emit dataChanged();
            return true;
        }
    }

    if (error) *error = QStringLiteral("未找到索引号为 '%1' 的图书").arg(indexId);
    return false;
}

bool LibraryManager::removeBookByIndexId(const QString &indexId)
{
    // 检查是否有副本被借阅
    QVector<BookCopy> copies = copyManager_.getCopiesByIndexId(indexId);
    for (const BookCopy &copy : copies) {
        if (!copy.isAvailable()) {
            return false; // 有副本被借阅，不能删除
        }
    }

    // 删除所有副本
    for (const BookCopy &copy : copies) {
        copyManager_.removeCopy(copy.copyId);
    }

    if (!dbManager_.removeBook(indexId)) {
        return false;
    }

    for (int i = 0; i < books_.size(); ++i) {
        if (books_[i].indexId == indexId) {
            books_.removeAt(i);
            emit dataChanged();
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
    return dbManager_.getBooksByCategory(category);
}

QVector<Book> LibraryManager::getByLocation(const QString &location) const
{
    return dbManager_.getBooksByLocation(location);
}

QVector<Book> LibraryManager::searchBooks(const QString &keyword) const
{
    return dbManager_.searchBooks(keyword);
}

QVector<Book> LibraryManager::getWarn(int days) const
{
    QVector<BookCopy> dueSoonCopies = copyManager_.getDueSoonCopies(days);
    QSet<QString> indexIds;
    for (const BookCopy &copy : dueSoonCopies) {
        indexIds.insert(copy.indexId);
    }

    QVector<Book> result;
    for (const QString &indexId : indexIds) {
        const Book *book = findByIndexId(indexId);
        if (book) {
            result.append(*book);
        }
    }
    return result;
}

// --- 副本管理 ---
bool LibraryManager::addBookCopies(const QString &indexId, int count, QString *error)
{
    if (!findByIndexId(indexId)) {
        if (error) *error = QStringLiteral("未找到索引号为 '%1' 的图书").arg(indexId);
        return false;
    }

    QVector<BookCopy> existingCopies = copyManager_.getCopiesByIndexId(indexId);
    int nextNumber = copyManager_.getNextCopyNumber(indexId);

    for (int i = 0; i < count; ++i) {
        BookCopy copy;
        copy.copyId = indexId + "_" + QString::number(nextNumber + i);
        copy.indexId = indexId;
        copy.copyNumber = nextNumber + i;
        if (!copyManager_.addCopy(copy)) {
            if (error) *error = QStringLiteral("添加副本失败");
            return false;
        }
    }

    emit dataChanged();
    return true;
}

bool LibraryManager::removeBookCopy(const QString &copyId, QString *error)
{
    BookCopy copy = copyManager_.getCopyById(copyId);
    if (copy.copyId.isEmpty()) {
        if (error) *error = QStringLiteral("未找到副本 '%1'").arg(copyId);
        return false;
    }

    if (!copy.isAvailable()) {
        if (error) *error = QStringLiteral("副本 '%1' 正在被借阅，无法删除").arg(copyId);
        return false;
    }

    if (copyManager_.removeCopy(copyId)) {
        emit dataChanged();
        return true;
    }

    if (error) *error = QStringLiteral("删除副本失败");
    return false;
}

QVector<BookCopy> LibraryManager::getBookCopies(const QString &indexId) const
{
    return copyManager_.getCopiesByIndexId(indexId);
}

QVector<BookCopy> LibraryManager::getAvailableCopies(const QString &indexId) const
{
    return copyManager_.getAvailableCopies(indexId);
}

BookCopy LibraryManager::getFirstAvailableCopy(const QString &indexId) const
{
    return copyManager_.getFirstAvailableCopy(indexId);
}

int LibraryManager::getTotalCopyCount(const QString &indexId) const
{
    return copyManager_.getTotalCopyCount(indexId);
}

int LibraryManager::getAvailableCopyCount(const QString &indexId) const
{
    return copyManager_.getAvailableCopyCount(indexId);
}

// --- 借阅相关 ---
bool LibraryManager::borrowBook(const QString &indexId, const QString &username, const QDate &dueDate, QString *error)
{
    BookCopy copy = getFirstAvailableCopy(indexId);
    if (copy.copyId.isEmpty()) {
        if (error) *error = QStringLiteral("没有可用的副本");
        return false;
    }

    if (copyManager_.borrowCopy(copy.copyId, username, dueDate)) {
        // 更新借阅次数
        Book *book = const_cast<Book*>(findByIndexId(indexId));
        if (book) {
            book->borrowCount++;
            dbManager_.updateBook(*book);
        }
        emit dataChanged();
        return true;
    }

    if (error) *error = QStringLiteral("借阅失败");
    return false;
}

bool LibraryManager::returnBook(const QString &copyId, const QString &username, QString *error)
{
    BookCopy copy = copyManager_.getCopyById(copyId);
    if (copy.copyId.isEmpty()) {
        if (error) *error = QStringLiteral("未找到副本 '%1'").arg(copyId);
        return false;
    }

    if (copy.borrowedBy != username) {
        if (error) *error = QStringLiteral("该副本不是由您借阅的");
        return false;
    }

    if (copyManager_.returnCopy(copyId)) {
        emit dataChanged();
        return true;
    }

    if (error) *error = QStringLiteral("归还失败");
    return false;
}

QVector<BookCopy> LibraryManager::getUserBorrowedCopies(const QString &username) const
{
    return copyManager_.getBorrowedCopies(username);
}

QVector<BookCopy> LibraryManager::getDueSoonCopies(int days) const
{
    return copyManager_.getDueSoonCopies(days);
}

// --- 统计信息 ---
int LibraryManager::getTotalBooks() const
{
    return dbManager_.getTotalBookCount();
}

int LibraryManager::getTotalCopies() const
{
    int total = 0;
    for (const Book &book : books_) {
        total += getTotalCopyCount(book.indexId);
    }
    return total;
}

int LibraryManager::getAvailableCopies() const
{
    int total = 0;
    for (const Book &book : books_) {
        total += getAvailableCopyCount(book.indexId);
    }
    return total;
}

int LibraryManager::getBorrowedCopies() const
{
    return getTotalCopies() - getAvailableCopies();
}

double LibraryManager::getTotalValue() const
{
    return dbManager_.getTotalInventoryValue();
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
void LibraryManager::sortByName()
{
    std::sort(books_.begin(), books_.end(), [](const Book&a, const Book&b){ return a.name < b.name; });
}

void LibraryManager::sortByCategory()
{
    std::sort(books_.begin(), books_.end(), [](const Book&a, const Book&b){ return a.category < b.category; });
}

void LibraryManager::sortByLocation()
{
    std::sort(books_.begin(), books_.end(), [](const Book&a, const Book&b){ return a.location < b.location; });
}

void LibraryManager::sortByPrice()
{
    std::sort(books_.begin(), books_.end(), [](const Book&a, const Book&b){ return a.price > b.price; });
}

void LibraryManager::sortByDate()
{
    std::sort(books_.begin(), books_.end(), [](const Book&a, const Book&b){ return a.inDate > b.inDate; });
}

void LibraryManager::sortByBorrowCount()
{
    std::sort(books_.begin(), books_.end(), [](const Book&a, const Book&b){ return a.borrowCount > b.borrowCount; });
}

bool LibraryManager::loadFromDatabase()
{
    books_ = dbManager_.getAllBooks();
    emit dataChanged();
    return true;
}

bool LibraryManager::saveToDatabase()
{
    // 数据已经通过各个操作实时保存到数据库
    // 这个方法主要用于确保数据已保存
    return true;
}

bool LibraryManager::importSampleData()
{
    // 创建示例图书数据
    QVector<Book> sampleBooks = {
        // 计算机类图书
        Book{"CS001", "C++程序设计教程", "谭浩强", "清华大学出版社", "仙林图书馆", "计算机科学", 45.80, QDate(2023, 1, 15), 12},
        Book{"CS002", "数据结构与算法分析", "Mark Allen Weiss", "机械工业出版社", "三牌楼图书馆", "计算机科学", 68.50, QDate(2023, 2, 20), 8},
        Book{"CS003", "操作系统概念", "Abraham Silberschatz", "机械工业出版社", "仙林图书馆", "计算机科学", 89.00, QDate(2023, 3, 10), 15},
        Book{"CS004", "计算机网络", "谢希仁", "电子工业出版社", "三牌楼图书馆", "计算机科学", 76.20, QDate(2023, 1, 25), 9},
        Book{"CS005", "数据库系统概论", "王珊", "高等教育出版社", "仙林图书馆", "计算机科学", 92.50, QDate(2023, 4, 5), 6},

        // 文学类图书
        Book{"LIT001", "红楼梦", "曹雪芹", "人民文学出版社", "三牌楼图书馆", "文学", 35.60, QDate(2023, 1, 10), 25},
        Book{"LIT002", "百年孤独", "加西亚·马尔克斯", "南海出版公司", "三牌楼图书馆", "文学", 42.80, QDate(2023, 2, 15), 18},
        Book{"LIT003", "活着", "余华", "作家出版社", "三牌楼图书馆", "文学", 28.90, QDate(2023, 3, 1), 22},
        Book{"LIT004", "平凡的世界", "路遥", "北京十月文艺出版社", "三牌楼图书馆", "文学", 55.00, QDate(2023, 1, 20), 16},
        Book{"LIT005", "围城", "钱钟书", "人民文学出版社", "三牌楼图书馆", "文学", 38.50, QDate(2023, 2, 28), 14},

        // 历史类图书
        Book{"HIS001", "中国通史", "范文澜", "人民出版社", "仙林图书馆", "历史", 78.00, QDate(2023, 1, 5), 11},
        Book{"HIS002", "世界文明史", "陈晓律", "商务印书馆", "三牌楼图书馆", "历史", 85.50, QDate(2023, 3, 15), 7},
        Book{"HIS003", "明朝那些事儿", "当年明月", "北京联合出版公司", "仙林图书馆", "历史", 48.80, QDate(2023, 2, 10), 20},
        Book{"HIS004", "人类简史", "尤瓦尔·赫拉利", "中信出版社", "三牌楼图书馆", "历史", 65.20, QDate(2023, 4, 1), 13},

        // 科学类图书
        Book{"SCI001", "时间简史", "史蒂芬·霍金", "湖南科学技术出版社", "仙林图书馆", "科学", 52.00, QDate(2023, 1, 30), 9},
        Book{"SCI002", "物种起源", "查尔斯·达尔文", "商务印书馆", "三牌楼图书馆", "科学", 68.80, QDate(2023, 3, 20), 5},
        Book{"SCI003", "相对论", "爱因斯坦", "科学出版社", "仙林图书馆", "科学", 75.50, QDate(2023, 2, 25), 3},
        Book{"SCI004", "量子力学原理", "狄拉克", "科学出版社", "仙林图书馆", "科学", 88.00, QDate(2023, 4, 10), 4},

        // 外语类图书
        Book{"ENG001", "新概念英语", "L. G. Alexander", "外语教学与研究出版社", "仙林图书馆", "外语", 32.50, QDate(2023, 1, 12), 35},
        Book{"ENG002", "托福词汇精选", "Zhao", "上海外语教育出版社", "三牌楼图书馆", "外语", 45.80, QDate(2023, 2, 18), 28},
        Book{"ENG003", "雅思考试指南", "Cambridge", "剑桥大学出版社", "仙林图书馆", "外语", 58.20, QDate(2023, 3, 8), 19},
        Book{"ENG004", "商务英语", "王志强", "外语教学与研究出版社", "三牌楼图书馆", "外语", 42.00, QDate(2023, 1, 28), 12},

        // 艺术类图书
        Book{"ART001", "西方美术史", "贡布里希", "广西美术出版社", "仙林图书馆", "艺术", 72.50, QDate(2023, 2, 5), 8},
        Book{"ART002", "中国书法艺术", "启功", "荣宝斋出版社", "仙林图书馆", "艺术", 55.80, QDate(2023, 3, 12), 6},
        Book{"ART003", "音乐理论基础", "郑珉", "人民音乐出版社", "仙林图书馆", "艺术", 48.00, QDate(2023, 1, 18), 10},

        // 哲学类图书
        Book{"PHI001", "论语", "孔子", "中华书局", "三牌楼图书馆", "哲学", 25.80, QDate(2023, 1, 8), 17},
        Book{"PHI002", "道德经", "老子", "中华书局", "三牌楼图书馆", "哲学", 22.50, QDate(2023, 2, 22), 14},
        Book{"PHI003", "苏菲的世界", "乔斯坦·贾德", "作家出版社", "三牌楼图书馆", "哲学", 38.80, QDate(2023, 3, 25), 11}
    };

    // 批量导入示例数据
    int addedCount = 0;
    for (const Book& book : sampleBooks) {
        if (addBook(book)) {
            addedCount++;
        }
    }

    qDebug() << "Imported" << addedCount << "sample books";
    return addedCount > 0;
}

bool LibraryManager::exportToJson(const QString& filePath)
{
    return dbManager_.exportToJson(filePath);
}

bool LibraryManager::importFromJson(const QString& filePath)
{
    if (dbManager_.importFromJson(filePath)) {
        return loadFromDatabase();
    }
    return false;
}


