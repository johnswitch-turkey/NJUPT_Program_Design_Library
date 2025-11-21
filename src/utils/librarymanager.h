/**
 * @file librarymanager.h
 * @brief 图书馆管理器头文件
 *
 * 本文件定义了图书馆管理器类LibraryManager，作为图书管理系统的核心业务逻辑层。
 * 提供图书信息的增删改查、分类管理、位置管理、搜索功能等核心业务功能。
 * 采用单例模式设计，确保全局只有一个图书馆管理器实例。
 *
 * @author 开发团队
 * @date 2023-2024
 * @version 1.0
 */

#ifndef LIBRARYMANAGER_H
#define LIBRARYMANAGER_H

#include <QObject>
#include <QVector>
#include <QHash>
#include <QString>
#include "book.h"
#include "./databasemanager.h"
#include "./bookcopymanager.h"

/**
 * @class LibraryManager
 * @brief 图书馆管理器类
 *
 * 该类是图书管理系统的核心业务逻辑层，负责管理所有图书相关操作。
 * 采用单例模式设计，提供统一的图书管理接口。
 * 集成了数据持久化、副本管理、搜索过滤等功能。
 *
 * 主要功能模块：
 * - 图书信息管理：添加、更新、删除、查询图书信息
 * - 分类管理：按类别组织和查询图书
 * - 位置管理：支持多校区的馆藏位置管理
 * - 搜索功能：支持多关键词的模糊搜索
 * - 副本管理：管理图书的借阅副本
 * - 数据统计：提供各类统计数据和报表
 * - 数据导入导出：支持示例数据的导入
 *
 * 设计模式：
 * - 单例模式：确保全局唯一的图书馆管理器实例
 * - 事务模式：确保数据操作的原子性和一致性
 * - 缓存模式：内存缓存提高查询性能
 *
 * 数据流程：
 * 应用层 -> LibraryManager -> DatabaseManager -> 本地存储
 *
 * @note 所有写操作都会触发自动保存
 * @note 提供详细的错误信息用于用户反馈
 * @note 支持事务操作确保数据一致性
 */
class LibraryManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 获取图书馆管理器单例实例
     *
     * 返回全局唯一的图书馆管理器实例，采用懒加载模式。
     * 第一次调用时创建实例，后续调用返回同一个实例。
     *
     * @return LibraryManager& 全局唯一的图书馆管理器引用
     * @note 线程安全，可在多线程环境中使用
     */
    static LibraryManager& instance();

    /**
     * @brief 构造函数
     *
     * 初始化图书馆管理器，建立与数据库管理器的连接，
     * 加载现有的图书数据到内存缓存中。
     *
     * @param parent 父QObject指针，用于Qt对象树管理
     * @note 构造函数通常由instance()方法自动调用
     */
    explicit LibraryManager(QObject *parent = nullptr);

    /**
     * @brief 清空所有图书数据
     *
     * 清空内存缓存和数据库中的所有图书信息。
     * 这是一个危险操作，通常仅在系统重置或测试时使用。
     *
     * @note 该操作不可撤销，请谨慎使用
     * @note 会同时清空副本数据
     */
    void clear();

    // ==================== 数据操作模块 ====================

    /**
     * @brief 添加新图书
     *
     * 向图书馆系统中添加一本新的图书。该方法会验证图书信息的完整性，
     * 检查索引号是否重复，然后将图书信息保存到数据库。
     *
     * @param book 要添加的图书对象常量引用
     * @param error 可选的错误信息输出参数，用于返回详细的错误描述
     * @return bool 添加成功返回true，失败返回false
     *
     * @note 索引号必须唯一，重复添加会返回失败
     * @note 书名和作者不能为空
     * @note 成功添加后会自动保存到数据库
     */
    bool addBook(const Book &book, QString *error = nullptr);

    /**
     * @brief 更新图书信息
     *
     * 更新指定索引号的图书信息。该方法会验证图书索引号的存在性，
     * 更新除索引号外的所有字段信息。
     *
     * @param indexId 要更新的图书索引号
     * @param updatedBook 包含更新信息的图书对象
     * @param error 可选的错误信息输出参数
     * @return bool 更新成功返回true，失败返回false
     *
     * @note 索引号字段不可修改，以确保数据一致性
     * @note 如果指定的图书不存在，操作会失败
     */
    bool updateBook(const QString &indexId, const Book &updatedBook, QString *error = nullptr);

    /**
     * @brief 根据索引号删除图书
     *
     * 删除指定索引号的图书及其所有副本信息。
     * 这是一个级联删除操作，会同时清理相关的副本数据。
     *
     * @param indexId 要删除的图书索引号
     * @return bool 删除成功返回true，失败返回false
     *
     * @note 删除操作不可撤销，请谨慎使用
     * @note 如果该图书有借出中的副本，删除操作会失败
     */
    bool removeBookByIndexId(const QString &indexId);

    // ==================== 数据查询模块 ====================

    /**
     * @brief 根据索引号查找图书
     *
     * 在图书数据库中查找指定索引号的图书。
     * 查找操作在内存缓存中进行，性能较好。
     *
     * @param indexId 要查找的图书索引号
     * @return const Book* 查找到的图书对象指针，未找到返回nullptr
     *
     * @note 返回的指针指向内部数据，不应修改或删除
     * @note 查找操作时间复杂度为O(1)
     */
    const Book* findByIndexId(const QString &indexId) const;

    /**
     * @brief 根据书名查找图书
     *
     * 在图书数据库中查找指定书名的图书。
     * 支持精确匹配和大小写不敏感查找。
     *
     * @param name 要查找的图书书名
     * @return const Book* 查找到的图书对象指针，未找到返回nullptr
     *
     * @note 如果有多本同名图书，返回找到的第一本
     * @note 查找操作时间复杂度为O(n)
     */
    const Book* findByName(const QString &name) const;

    // ==================== 数据获取模块 ====================

    /**
     * @brief 获取所有图书信息
     *
     * 返回系统中所有图书的信息列表。
     * 返回的数据按照添加时间排序。
     *
     * @return const QVector<Book>& 所有图书的常量引用
     *
     * @note 返回的引用指向内部数据，不应修改
     * @note 数据量较大时建议使用分页或过滤功能
     */
    const QVector<Book>& getAll() const;

    /**
     * @brief 根据分类获取图书列表
     *
     * 返回指定分类下的所有图书信息。
     * 分类匹配支持大小写不敏感的比较。
     *
     * @param category 图书分类名称
     * @return QVector<Book> 指定分类的图书列表
     *
     * @note 如果分类不存在，返回空列表
     * @note 分类名称支持部分匹配
     */
    QVector<Book> getByCategory(const QString &category) const;

    /**
     * @brief 根据馆藏位置获取图书列表
     *
     * 返回指定馆藏位置的所有图书信息。
     * 支持多校区的位置管理。
     *
     * @param location 馆藏位置名称
     * @return QVector<Book> 指定位置的图书列表
     *
     * @note 常见位置：三牌楼、仙林等
     * @note 位置匹配支持大小写不敏感
     */
    QVector<Book> getByLocation(const QString &location) const;

    /**
     * @brief 搜索图书
     *
     * 根据关键词在书名、作者、出版社、索引号等字段中进行模糊搜索。
     * 支持中文分词和多关键词搜索。
     *
     * @param keyword 搜索关键词
     * @return QVector<Book> 匹配的图书列表
     *
     * @note 搜索是大小写不敏感的
     * @note 支持空格分隔的多关键词搜索
     * @note 搜索结果按相关性排序
     */
    QVector<Book> searchBooks(const QString &keyword) const;

    /**
     * @brief 获取借阅预警图书列表
     *
     * 获取指定天数内即将到期或逾期的图书列表。
     * 用于借阅管理和催还提醒。
     *
     * @param days 预警天数，从当前日期开始计算
     * @return QVector<Book> 需要预警的图书列表
     *
     * @note 包含已经逾期的副本
     * @note 按到期时间排序，最紧急的在前
     */
    QVector<Book> getWarn(int days) const;

    // ==================== 副本管理模块 ====================

    /**
     * @brief 添加图书副本
     *
     * 为指定索引号的图书添加指定数量的副本。
     * 副本编号会自动生成，确保唯一性。
     *
     * @param indexId 图书索引号
     * @param count 要添加的副本数量
     * @param error 可选的错误信息输出参数
     * @return bool 添加成功返回true，失败返回false
     *
     * @note 指定的图书必须存在
     * @note 副本编号格式：索引号-COPY-数字
     */
    bool addBookCopies(const QString &indexId, int count, QString *error = nullptr);

    /**
     * @brief 删除图书副本
     *
     * 删除指定编号的图书副本。
     * 只能删除未被借出的副本。
     *
     * @param copyId 副本编号
     * @param error 可选的错误信息输出参数
     * @return bool 删除成功返回true，失败返回false
     *
     * @note 借出中的副本不能删除
     * @note 删除操作不可撤销
     */
    bool removeBookCopy(const QString &copyId, QString *error = nullptr);

    /**
     * @brief 获取图书副本列表
     *
     * 返回指定图书的所有副本信息。
     * 包括副本的借阅状态、借阅者、应还日期等信息。
     *
     * @param indexId 图书索引号
     * @return QVector<BookCopy> 副本信息列表
     *
     * @note 包含所有状态的副本：可借、已借出、维护中等
     * @note 副本列表按副本编号排序
     */
    QVector<BookCopy> getBookCopies(const QString &indexId) const;
    QVector<BookCopy> getAvailableCopies(const QString &indexId) const;
    BookCopy getFirstAvailableCopy(const QString &indexId) const;
    int getTotalCopyCount(const QString &indexId) const;
    int getAvailableCopyCount(const QString &indexId) const;

    // --- 借阅相关 ---
    bool borrowBook(const QString &indexId, const QString &username, const QDate &dueDate, QString *error = nullptr);
    bool returnBook(const QString &copyId, const QString &username, QString *error = nullptr);
    bool renewBook(const QString &copyId, const QString &username, int extendDays = 30, QString *error = nullptr);  // 新增：续借功能，默认续借一个月
    QVector<BookCopy> getUserBorrowedCopies(const QString &username) const;
    QVector<BookCopy> getDueSoonCopies(int days) const;

    // --- 统计信息 ---
    int getTotalBooks() const;
    int getTotalCopies() const;
    int getAvailableCopies() const;
    int getBorrowedCopies() const;
    double getTotalValue() const;
    // QString getMostPopularCategory() const;
    QString getMostPopularLocation() const;

    // --- 排序 ---
    // void sortByName();
    // void sortByCategory();
    // void sortByLocation();
    // void sortByPrice(); // 高到低
    // void sortByDate(); // 新到旧
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

