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
/**
 * @brief 添加图书到图书馆管理系统
 *
 * 功能流程：
 * 1. 重复检查：检查索引号是否已存在，确保唯一性
 * 2. 数据库操作：将图书信息保存到数据库
 * 3. 内存更新：将图书信息添加到内存中的books_向量
 * 4. 副本创建：为新添加的图书创建一个默认副本
 * 5. 信号发送：发送dataChanged信号，通知UI更新
 *
 * @param book 要添加的图书信息
 * @param error 错误信息输出参数（可选）
 * @return 添加成功返回true，失败返回false
 *
 * 涉及的函数：
 * - findByIndexId(): 查找图书是否已存在
 * - dbManager_.addBook(): 数据库添加操作
 * - copyManager_.addCopy(): 添加图书副本
 * - emit dataChanged(): 发送数据变更信号
 */
bool LibraryManager::addBook(const Book &book, QString *error)
{
    // 重复检查：确保索引号的唯一性
    if (findByIndexId(book.indexId)) {
        if (error) *error = QStringLiteral("索引号 '%1' 已存在").arg(book.indexId);
        return false;
    }

    // 数据库操作：将图书信息持久化保存
    if (!dbManager_.addBook(book)) {
        if (error) *error = QStringLiteral("数据库添加失败");
        return false;
    }

    // 内存更新：更新运行时数据
    books_.append(book);

    // 副本创建：为新图书创建默认副本（副本1）
    BookCopy copy;
    copy.copyId = book.indexId + "_1";  // 副本ID格式：索引号_编号
    copy.indexId = book.indexId;
    copy.copyNumber = 1;
    copyManager_.addCopy(copy);

    // 信号发送：通知UI界面更新
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
/**
 * @brief 借书功能实现
 *
 * 功能流程：
 * 1. 副本查找：获取该图书的第一个可用副本
 * 2. 可用性检查：确保有可借阅的副本
 * 3. 借阅操作：调用副本管理器执行具体的借阅操作
 * 4. 统计更新：更新图书的借阅次数统计
 * 5. 数据持久化：将更新的图书信息保存到数据库
 * 6. 信号发送：通知UI界面更新显示
 *
 * @param indexId 图书索引号
 * @param username 借阅用户名
 * @param dueDate 归还日期
 * @param error 错误信息输出参数（可选）
 * @return 借阅成功返回true，失败返回false
 *
 * 涉及的函数：
 * - getFirstAvailableCopy(): 获取第一个可用副本
 * - copyManager_.borrowCopy(): 执行副本借阅操作
 * - findByIndexId(): 查找图书信息
 * - dbManager_.updateBook(): 更新数据库中的图书信息
 */
bool LibraryManager::borrowBook(const QString &indexId, const QString &username, const QDate &dueDate, QString *error)
{
    // 副本查找：获取该图书的第一个可用副本
    BookCopy copy = getFirstAvailableCopy(indexId);
    if (copy.copyId.isEmpty()) {
        if (error) *error = QStringLiteral("没有可用的副本");
        return false;
    }

    // 借阅操作：调用副本管理器执行具体的借阅操作
    if (copyManager_.borrowCopy(copy.copyId, username, dueDate)) {
        // 统计更新：更新图书的借阅次数
        Book *book = const_cast<Book*>(findByIndexId(indexId));
        if (book) {
            book->borrowCount++;
            dbManager_.updateBook(*book);  // 数据持久化
        }
        emit dataChanged();  // 通知UI更新
        return true;
    }

    if (error) *error = QStringLiteral("借阅失败");
    return false;
}

/**
 * @brief 还书功能实现
 *
 * 功能流程：
 * 1. 副本验证：检查副本ID是否存在
 * 2. 权限验证：确保归还者与借阅者一致
 * 3. 还书操作：调用副本管理器执行具体的还书操作
 * 4. 状态更新：更新副本状态为可用
 * 5. 信号发送：通知UI界面更新显示
 *
 * @param copyId 副本ID
 * @param username 归还用户名
 * @param error 错误信息输出参数（可选）
 * @return 还书成功返回true，失败返回false
 *
 * 涉及的函数：
 * - copyManager_.getCopyById(): 根据副本ID查找副本信息
 * - copyManager_.returnCopy(): 执行副本归还操作
 * - emit dataChanged(): 发送数据变更信号
 */
bool LibraryManager::returnBook(const QString &copyId, const QString &username, QString *error)
{
    // 副本验证：检查副本ID是否存在
    BookCopy copy = copyManager_.getCopyById(copyId);
    if (copy.copyId.isEmpty()) {
        if (error) *error = QStringLiteral("未找到副本 '%1'").arg(copyId);
        return false;
    }

    // 权限验证：确保归还者与借阅者一致，防止误操作
    if (copy.borrowedBy != username) {
        if (error) *error = QStringLiteral("该副本不是由您借阅的");
        return false;
    }

    // 还书操作：调用副本管理器执行具体的还书操作
    if (copyManager_.returnCopy(copyId)) {
        emit dataChanged();  // 通知UI更新
        return true;
    }

    if (error) *error = QStringLiteral("归还失败");
    return false;
}

/**
 * @brief 续借功能实现
 *
 * 功能流程：
 * 1. 副本验证：检查副本ID是否存在
 * 2. 权限验证：确保续借者与借阅者一致
 * 3. 状态检查：确保副本当前处于借阅状态
 * 4. 续借操作：调用副本管理器执行具体的续借操作
 * 5. 信号发送：通知UI界面更新显示
 *
 * @param copyId 副本ID
 * @param username 续借用户名
 * @param extendDays 续借天数，默认30天
 * @param error 错误信息输出参数（可选）
 * @return 续借成功返回true，失败返回false
 */
bool LibraryManager::renewBook(const QString &copyId, const QString &username, int extendDays, QString *error)
{
    // 副本验证：检查副本ID是否存在
    BookCopy copy = copyManager_.getCopyById(copyId);
    if (copy.copyId.isEmpty()) {
        if (error) *error = QStringLiteral("未找到副本 '%1'").arg(copyId);
        return false;
    }

    // 权限验证：确保续借者与借阅者一致
    if (copy.borrowedBy != username) {
        if (error) *error = QStringLiteral("该副本不是由您借阅的，无法续借");
        return false;
    }

    // 状态检查：确保副本当前处于借阅状态
    if (copy.isAvailable()) {
        if (error) *error = QStringLiteral("该副本未被借阅，无需续借");
        return false;
    }

    // 续借操作：调用副本管理器执行具体的续借操作
    if (copyManager_.renewCopy(copyId, extendDays)) {
        emit dataChanged();  // 通知UI更新
        return true;
    }

    if (error) *error = QStringLiteral("续借失败");
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

// QString LibraryManager::getMostPopularCategory() const
// {
//     QHash<QString, int> categoryCount;
//     for(const auto& b : books_) {
//         categoryCount[b.category]++;
//     }
//
//     QString popular;
//     int maxCount = 0;
//     for(auto it = categoryCount.constBegin(); it != categoryCount.constEnd(); ++it) {
//         if (it.value() > maxCount) {
//             maxCount = it.value();
//             popular = it.key();
//         }
//     }
//     return popular;
// }

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

// // --- 排序 ---
// void LibraryManager::sortByName()
// {
//     std::sort(books_.begin(), books_.end(), [](const Book&a, const Book&b){ return a.name < b.name; });
// }
//
// void LibraryManager::sortByCategory()
// {
//     std::sort(books_.begin(), books_.end(), [](const Book&a, const Book&b){ return a.category < b.category; });
// }
//
// void LibraryManager::sortByLocation()
// {
//     std::sort(books_.begin(), books_.end(), [](const Book&a, const Book&b){ return a.location < b.location; });
// }
//
// void LibraryManager::sortByPrice()
// {
//     std::sort(books_.begin(), books_.end(), [](const Book&a, const Book&b){ return a.price > b.price; });
// }
//
// void LibraryManager::sortByDate()
// {
//     std::sort(books_.begin(), books_.end(), [](const Book&a, const Book&b){ return a.inDate > b.inDate; });
// }

void LibraryManager::sortByBorrowCount()
{
    std::sort(books_.begin(), books_.end(), [](const Book&a, const Book&b){ return a.borrowCount > b.borrowCount; });
}

bool LibraryManager::loadFromDatabase()
{
    books_ = dbManager_.getAllBooks();

    // 确保每本书都有副本，如果没有则创建默认副本
    for (const Book& book : books_) {
        QVector<BookCopy> existingCopies = copyManager_.getCopiesByIndexId(book.indexId);
        if (existingCopies.isEmpty()) {
            // 没有副本，创建默认副本
            BookCopy copy;
            copy.copyId = book.indexId + "_1";
            copy.indexId = book.indexId;
            copy.copyNumber = 1;
            copyManager_.addCopy(copy);

            // 根据借阅次数创建额外副本
            int extraCopies = qMax(0, (book.borrowCount - 1) / 5); // 每5次借阅增加一个副本
            if (extraCopies > 0) {
                addBookCopies(book.indexId, extraCopies);
            }
        }
    }

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
        Book{"CS001", "C++程序设计教程", "谭浩强", "清华大学出版社", "仙林图书馆", "计算机科学", 45.80, QDate(2023, 1, 15), 12, "本书详细介绍了C++程序设计的基础知识和高级特性，包括面向对象编程、模板、STL等内容，适合作为高等院校计算机专业教材使用。"},
        Book{"CS002", "数据结构与算法分析", "Mark Allen Weiss", "机械工业出版社", "三牌楼图书馆", "计算机科学", 68.50, QDate(2023, 2, 20), 8, "本书系统地介绍了数据结构和算法分析的基本概念和方法，内容包括数组、链表、栈、队列、树、图等基本数据结构，以及排序、查找等经典算法。"},
        Book{"CS003", "操作系统概念", "Abraham Silberschatz", "机械工业出版社", "仙林图书馆", "计算机科学", 89.00, QDate(2023, 3, 10), 15, "本书是操作系统领域的经典教材，全面介绍了操作系统的基本原理、概念和设计方法，包括进程管理、内存管理、文件系统和I/O系统等核心内容。"},
        Book{"CS004", "计算机网络", "谢希仁", "电子工业出版社", "三牌楼图书馆", "计算机科学", 76.20, QDate(2023, 1, 25), 9, "本书系统介绍了计算机网络的基本原理和技术，包括网络体系结构、数据通信、局域网、广域网、网络协议等内容，是计算机网络学习的优秀教材。"},
        Book{"CS005", "数据库系统概论", "王珊", "高等教育出版社", "仙林图书馆", "计算机科学", 92.50, QDate(2023, 4, 5), 6, "本书全面介绍了数据库系统的基本概念、原理和技术，包括关系数据库理论、SQL语言、数据库设计、事务管理、并发控制等重要内容。"},

        // 文学类图书
        Book{"LIT001", "红楼梦", "曹雪芹", "人民文学出版社", "三牌楼图书馆", "文学", 35.60, QDate(2023, 1, 10), 25, "中国古典文学四大名著之一，以贾宝玉、林黛玉的爱情悲剧为主线，展现了封建大家族的兴衰史，是中国古典小说的巅峰之作。"},
        Book{"LIT002", "百年孤独", "加西亚·马尔克斯", "南海出版公司", "三牌楼图书馆", "文学", 42.80, QDate(2023, 2, 15), 18, "魔幻现实主义文学的代表作，讲述了布恩迪亚家族七代人的传奇故事，展现了拉丁美洲的历史变迁和文化特色。"},
        Book{"LIT003", "活着", "余华", "作家出版社", "三牌楼图书馆", "文学", 28.90, QDate(2023, 3, 1), 22, "讲述了福贵悲惨而又充满韧性的一生，通过一个普通人的命运展现了大时代背景下的人生百态，是当代中国文学的经典之作。"},
        Book{"LIT004", "平凡的世界", "路遥", "北京十月文艺出版社", "三牌楼图书馆", "文学", 55.00, QDate(2023, 1, 20), 16, "以孙少安、孙少平两兄弟为中心，展现了改革开放前后中国农村的社会变迁和人们的奋斗历程，是一部现实主义文学杰作。"},
        Book{"LIT005", "围城", "钱钟书", "人民文学出版社", "三牌楼图书馆", "文学", 38.50, QDate(2023, 2, 28), 14, "以方鸿渐的人生经历为主线，深刻揭示了知识分子的精神困境和社会的'围城'现象，是中国现代文学的经典之作。"},

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

            // 根据借阅次数添加额外副本
            int extraCopies = qMax(0, (book.borrowCount - 1) / 5); // 每5次借阅增加一个副本
            if (extraCopies > 0) {
                addBookCopies(book.indexId, extraCopies);
            }
        }
    }

    qDebug() << "Imported" << addedCount << "sample books with appropriate copy counts";
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


