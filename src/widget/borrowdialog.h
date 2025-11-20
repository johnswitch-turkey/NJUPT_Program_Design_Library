// borrowdialog.h
#ifndef BORROWDIALOG_H
#define BORROWDIALOG_H

#include <QDialog>
#include <QDate>
#include <QVector>
#include "../utils/book.h"
#include "../utils/bookcopy.h"

// 前向声明，避免包含完整的Qt头文件
class QVBoxLayout;
class QHBoxLayout;
class QLabel;
class QComboBox;
class QDateEdit;
class QPushButton;
class QLineEdit;

/**
 * @brief 借书对话框类
 *
 * 功能描述：
 * 提供一个友好的用户界面，让学生可以选择具体要借阅的图书副本和归还日期
 *
 * 主要特性：
 * 1. 显示图书详细信息（书名、索引号、作者、出版社）
 * 2. 列出所有可用的副本供用户选择
 * 3. 提供日期选择器设置归还日期（1-90天）
 * 4. 实时验证用户输入的有效性
 * 5. 动态更新状态提示信息
 * 6. 响应式界面设计，支持暗色/浅色主题
 */
class BorrowDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param book 要借阅的图书信息
     * @param availableCopies 该图书的可用副本列表
     * @param parent 父窗口指针
     */
    explicit BorrowDialog(const Book &book, const QVector<BookCopy> &availableCopies, QWidget *parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~BorrowDialog();

    /**
     * @brief 获取用户选择的副本
     * @return 选中的副本信息
     */
    BookCopy getSelectedCopy() const;

    /**
     * @brief 获取用户设置的归还日期
     * @return 归还日期
     */
    QDate getDueDate() const;

private slots:
    /**
     * @brief 确认按钮点击事件处理
     * 验证输入有效后关闭对话框，返回QDialog::Accepted
     */
    void onOkClicked();

    /**
     * @brief 取消按钮点击事件处理
     * 关闭对话框，返回QDialog::Rejected
     */
    void onCancelClicked();

    /**
     * @brief 输入验证槽函数
     * 验证用户选择的副本和归还日期是否有效，更新UI状态
     */
    void validateInput();

private:
    /**
     * @brief 设置用户界面
     * 创建和布局所有UI控件
     */
    void setupUI();

    /**
     * @brief 更新副本信息显示
     * 当用户选择不同副本时，更新状态标签显示选中的副本信息
     */
    void updateCopyInfo();

    // 数据成员
    Book book_;                    // 要借阅的图书信息
    QVector<BookCopy> availableCopies_;  // 可用副本列表
    BookCopy selectedCopy_;        // 用户选择的副本
    QDate dueDate_;               // 归还日期

    // UI组件成员
    QLabel *bookInfoLabel_;       // 图书信息显示标签
    QComboBox *copyComboBox_;     // 副本选择下拉框
    QDateEdit *dueDateEdit_;      // 归还日期选择器
    QPushButton *okButton_;       // 确认按钮
    QPushButton *cancelButton_;   // 取消按钮
    QLabel *statusLabel_;         // 状态提示标签
};

#endif // BORROWDIALOG_H