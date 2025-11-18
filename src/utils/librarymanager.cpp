#include "librarymanager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDate>
#include <QDir>
#include <algorithm>

#include "./databasemanager.h"


LibraryManager::LibraryManager(QObject *parent)
    : QObject(parent)
    , dbManager_(DatabaseManager::instance())
{
    loadFromDatabase();

    // 如果数据库为空，自动导入示例数据
    if (books_.isEmpty()) {
        importSampleData();
    }
}

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

QVector<Book> LibraryManager::getAvailable() const
{
    return dbManager_.getAvailableBooks();
}

QVector<Book> LibraryManager::getBorrowed() const
{
    return dbManager_.getBorrowedBooks();
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
    return dbManager_.searchBooks(keyword);
}

// --- 统计信息 ---
int LibraryManager::getTotalBooks() const
{
    return dbManager_.getTotalBookCount();
}

int LibraryManager::getAvailableBooks() const
{
    return dbManager_.getAvailableBookCount();
}

int LibraryManager::getBorrowedBooks() const
{
    return dbManager_.getBorrowedBookCount();
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
        Book{"CS001", "C++程序设计教程", "谭浩强", "清华大学出版社", "仙林图书馆", "计算机科学", 5, 45.80, QDate(2023, 1, 15), QDate(), 12, true},
        Book{"CS002", "数据结构与算法分析", "Mark Allen Weiss", "机械工业出版社", "三牌楼图书馆", "计算机科学", 3, 68.50, QDate(2023, 2, 20), QDate(), 8, true},
        Book{"CS003", "操作系统概念", "Abraham Silberschatz", "机械工业出版社", "仙林图书馆", "计算机科学", 4, 89.00, QDate(2023, 3, 10), QDate(), 15, true},
        Book{"CS004", "计算机网络", "谢希仁", "电子工业出版社", "三牌楼图书馆", "计算机科学", 6, 76.20, QDate(2023, 1, 25), QDate(), 9, true},
        Book{"CS005", "数据库系统概论", "王珊", "高等教育出版社", "仙林图书馆", "计算机科学", 2, 92.50, QDate(2023, 4, 5), QDate(), 6, true},

        // 文学类图书
        Book{"LIT001", "红楼梦", "曹雪芹", "人民文学出版社", "三牌楼图书馆", "文学", 8, 35.60, QDate(2023, 1, 10), QDate(), 25, true},
        Book{"LIT002", "百年孤独", "加西亚·马尔克斯", "南海出版公司", "三牌楼图书馆", "文学", 4, 42.80, QDate(2023, 2, 15), QDate(), 18, true},
        Book{"LIT003", "活着", "余华", "作家出版社", "三牌楼图书馆", "文学", 6, 28.90, QDate(2023, 3, 1), QDate(), 22, true},
        Book{"LIT004", "平凡的世界", "路遥", "北京十月文艺出版社", "三牌楼图书馆", "文学", 5, 55.00, QDate(2023, 1, 20), QDate(), 16, true},
        Book{"LIT005", "围城", "钱钟书", "人民文学出版社", "三牌楼图书馆", "文学", 3, 38.50, QDate(2023, 2, 28), QDate(), 14, true},

        // 历史类图书
        Book{"HIS001", "中国通史", "范文澜", "人民出版社", "仙林图书馆", "历史", 4, 78.00, QDate(2023, 1, 5), QDate(), 11, true},
        Book{"HIS002", "世界文明史", "陈晓律", "商务印书馆", "三牌楼图书馆", "历史", 3, 85.50, QDate(2023, 3, 15), QDate(), 7, true},
        Book{"HIS003", "明朝那些事儿", "当年明月", "北京联合出版公司", "仙林图书馆", "历史", 6, 48.80, QDate(2023, 2, 10), QDate(), 20, true},
        Book{"HIS004", "人类简史", "尤瓦尔·赫拉利", "中信出版社", "三牌楼图书馆", "历史", 5, 65.20, QDate(2023, 4, 1), QDate(), 13, true},

        // 科学类图书
        Book{"SCI001", "时间简史", "史蒂芬·霍金", "湖南科学技术出版社", "仙林图书馆", "科学", 3, 52.00, QDate(2023, 1, 30), QDate(), 9, true},
        Book{"SCI002", "物种起源", "查尔斯·达尔文", "商务印书馆", "三牌楼图书馆", "科学", 2, 68.80, QDate(2023, 3, 20), QDate(), 5, true},
        Book{"SCI003", "相对论", "爱因斯坦", "科学出版社", "仙林图书馆", "科学", 1, 75.50, QDate(2023, 2, 25), QDate(), 3, true},
        Book{"SCI004", "量子力学原理", "狄拉克", "科学出版社", "仙林图书馆", "科学", 2, 88.00, QDate(2023, 4, 10), QDate(), 4, true},

        // 外语类图书
        Book{"ENG001", "新概念英语", "L. G. Alexander", "外语教学与研究出版社", "仙林图书馆", "外语", 10, 32.50, QDate(2023, 1, 12), QDate(), 35, true},
        Book{"ENG002", "托福词汇精选", "Zhao", "上海外语教育出版社", "三牌楼图书馆", "外语", 8, 45.80, QDate(2023, 2, 18), QDate(), 28, true},
        Book{"ENG003", "雅思考试指南", "Cambridge", "剑桥大学出版社", "仙林图书馆", "外语", 6, 58.20, QDate(2023, 3, 8), QDate(), 19, true},
        Book{"ENG004", "商务英语", "王志强", "外语教学与研究出版社", "三牌楼图书馆", "外语", 4, 42.00, QDate(2023, 1, 28), QDate(), 12, true},

        // 艺术类图书
        Book{"ART001", "西方美术史", "贡布里希", "广西美术出版社", "仙林图书馆", "艺术", 3, 72.50, QDate(2023, 2, 5), QDate(), 8, true},
        Book{"ART002", "中国书法艺术", "启功", "荣宝斋出版社", "仙林图书馆", "艺术", 2, 55.80, QDate(2023, 3, 12), QDate(), 6, true},
        Book{"ART003", "音乐理论基础", "郑珉", "人民音乐出版社", "仙林图书馆", "艺术", 4, 48.00, QDate(2023, 1, 18), QDate(), 10, true},

        // 哲学类图书
        Book{"PHI001", "论语", "孔子", "中华书局", "三牌楼图书馆", "哲学", 5, 25.80, QDate(2023, 1, 8), QDate(), 17, true},
        Book{"PHI002", "道德经", "老子", "中华书局", "三牌楼图书馆", "哲学", 4, 22.50, QDate(2023, 2, 22), QDate(), 14, true},
        Book{"PHI003", "苏菲的世界", "乔斯坦·贾德", "作家出版社", "三牌楼图书馆", "哲学", 3, 38.80, QDate(2023, 3, 25), QDate(), 11, true},

        // 一些已借出的图书（剩余数量为 0）
        Book{"CS006", "人工智能导论", "Stuart Russell", "机械工业出版社", "仙林图书馆", "计算机科学", 0, 95.00, QDate(2023, 4, 15), QDate(2024, 1, 15), 3, false},
        Book{"LIT006", "1984", "乔治·奥威尔", "新星出版社", "三牌楼图书馆", "文学", 0, 36.50, QDate(2023, 2, 8), QDate(2024, 1, 20), 7, false},
        Book{"ENG005", "英语语法大全", "侯捷", "外语教学与研究出版社", "仙林图书馆", "外语", 0, 52.80, QDate(2023, 3, 18), QDate(2024, 1, 25), 9, false},
        Book{"SCI005", "宇宙的奥秘", "Carl Sagan", "科学出版社", "仙林图书馆", "科学", 0, 68.00, QDate(2023, 1, 22), QDate(2024, 1, 30), 5, false}
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

void LibraryManager::refreshFromDatabase()
{
    loadFromDatabase();
}

QVector<Book> LibraryManager::getWarn(int days) const
{
    QVector<Book> dueSoonBooks;
    QDate currentDate = QDate::currentDate();
    QDate thresholdDate = currentDate.addDays(days);

    for (const Book &book : books_) {
        // 只考虑已借出的图书
        if (!book.available && book.returnDate.isValid()) {
            // 检查归还日期是否在阈值之内（包括今天）
            if (book.returnDate <= thresholdDate) {
                dueSoonBooks.append(book);
            }
        }
    }

    return dueSoonBooks;
}
