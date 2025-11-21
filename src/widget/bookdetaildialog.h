/**
 * @file bookdetaildialog.h
 * @brief 图书详情对话框头文件
 *
 * 本文件定义了图书详情对话框类BookDetailDialog，用于显示图书的详细信息。
 * 该对话框以只读形式展示图书的基本信息、内容简介和副本状态等信息。
 *
 * @author 开发团队
 * @date 2023-2024
 * @version 1.0
 */

#ifndef BOOKDETAILDIALOG_H
#define BOOKDETAILDIALOG_H

#include <QDialog>
#include <QJsonObject>
#include "../utils/book.h"

// 前向声明，减少头文件依赖
class QLabel;
class QPushButton;
class QVBoxLayout;
class QHBoxLayout;
class QScrollArea;
class QGroupBox;

/**
 * @class BookDetailDialog
 * @brief 图书详情对话框类
 *
 * 该类提供了一个模态对话框，用于展示图书的完整详细信息。
 * 对话框采用分组布局，将信息分为三个主要部分：
 * 1. 内容简介 - 显示图书的详细内容介绍
 * 2. 基本信息 - 显示图书的书名、作者、出版社等属性
 * 3. 副本信息 - 显示该书所有副本的状态和借阅情况
 *
 * 对话框支持滚动显示，能够处理大量信息的展示。所有信息都以只读方式显示，
 * 确保数据安全性。采用现代化的UI设计，提供良好的用户体验。
 *
 * @note 该对话框为只读模式，不允许编辑图书信息
 * @note 支持长文本自动换行显示
 * @note 副本信息会实时显示当前状态
 */
class BookDetailDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     *
     * 创建一个图书详情对话框并初始化所有UI组件。
     * 对话框将显示传入图书对象的完整信息。
     *
     * @param book 要显示详情的图书对象常量引用
     * @param parent 父窗口指针，默认为nullptr，表示顶级窗口
     */
    explicit BookDetailDialog(const Book &book, QWidget *parent = nullptr);

private slots:
    // 注意：当前版本中没有自定义槽函数

private:
    /**
     * @brief 设置用户界面布局
     *
     * 初始化对话框的主要UI结构，包括：
     * - 设置对话框标题和大小
     * - 创建滚动区域以支持内容滚动
     * - 创建三个信息组的容器
     * - 设置对话框的基本样式
     *
     * 该函数在构造函数中被调用，是UI初始化的核心函数。
     */
    void setupUI();

    /**
     * @brief 设置内容简介显示区域
     *
     * 配置内容简介组的显示内容，包括：
     * - 清除现有内容
     * - 显示图书的内容简介文本
     * - 如果没有内容简介，显示"暂无内容简介"提示
     * - 应用合适的样式以增强可读性
     */
    void setupDescription();

    /**
     * @brief 设置图书基本信息显示区域
     *
     * 配置图书基本信息组的显示内容，包括：
     * - 清除现有内容
     * - 显示索引号、书名、作者等所有基本信息
     * - 采用标签-值的水平布局形式
     * - 为不同类型的信息应用不同的样式
     *
     * 显示的信息包括：索引号、书名、作者、出版社、馆藏地址、
     * 类别、价格、入库日期、借阅次数等。
     */
    void setupBookInfo();

    /**
     * @brief 设置图书副本信息显示区域
     *
     * 配置副本信息组的显示内容，包括：
     * - 清除现有内容
     * - 从副本管理器获取该书的所有副本信息
     * - 以表格形式显示副本编号、状态、借阅者、应还日期
     * - 显示副本统计信息（总数、可借数量、已借数量）
     *
     * 如果该书没有副本，会显示相应的提示信息。
     * 副本状态会根据是否可借显示不同的颜色标识。
     */
    void setupCopiesInfo();

    // 数据成员
    Book book_;    ///< 当前显示的图书对象

    // 主要UI组件
    QScrollArea *scrollArea_;    ///< 滚动区域，支持内容超出时滚动显示
    QWidget *contentWidget_;     ///< 滚动区域内的内容容器
    QVBoxLayout *mainLayout_;    ///< 对话框的主垂直布局

    // 内容简介组相关UI组件
    QGroupBox *descriptionGroup_;    ///< 内容简介的分组容器
    QVBoxLayout *descriptionLayout_; ///< 内容简介组的布局管理器
    QLabel *descriptionLabel_;       ///< 显示内容简介文本的标签

    // 图书信息组相关UI组件
    QGroupBox *bookInfoGroup_;        ///< 基本信息的分组容器
    QVBoxLayout *bookInfoLayout_;     ///< 基本信息组的布局管理器
    QLabel *indexIdLabel_;           ///< "索引号："标签
    QLabel *nameLabel_;              ///< "书名："标签
    QLabel *authorLabel_;            ///< "作者："标签
    QLabel *publisherLabel_;         ///< "出版社："标签
    QLabel *locationLabel_;          ///< "馆藏地址："标签
    QLabel *categoryLabel_;          ///< "类别："标签
    QLabel *priceLabel_;             ///< "价格："标签
    QLabel *inDateLabel_;            ///< "入库日期："标签
    QLabel *borrowCountLabel_;       ///< "借阅次数："标签

    // 副本信息组相关UI组件
    QGroupBox *copiesGroup_;      ///< 副本信息的分组容器
    QVBoxLayout *copiesLayout_;   ///< 副本信息组的布局管理器

    // 按钮组件
    QPushButton *closeButton_;   ///< 关闭对话框的按钮
};

#endif // BOOKDETAILDIALOG_H