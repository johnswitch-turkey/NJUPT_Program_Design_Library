/**
 * @file bookdisplay.h
 * @brief 图书信息编辑对话框头文件
 *
 * 本文件定义了图书信息编辑对话框类BookDialog，用于添加新书和编辑现有图书信息。
 * 提供了完整的图书信息编辑界面，包括基本信息输入和内容简介编辑功能。
 *
 * @author 开发团队
 * @date 2023-2024
 * @version 1.0
 */

#ifndef BOOKDISPLAY_H
#define BOOKDISPLAY_H

#include <QDialog>
#include <QLineEdit>
#include <QTextEdit>
#include <QDateEdit>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QComboBox>
#include "book.h"

/**
 * @class BookDialog
 * @brief 图书信息编辑对话框类
 *
 * 该类提供了一个模态对话框，用于用户输入和编辑图书信息。
 * 支持两种模式：添加新书模式和编辑现有图书模式。
 * 对话框包含完整的图书信息输入字段，采用表单布局和现代化UI设计。
 *
 * 主要功能：
 * - 支持索引号的自动生成和手动输入
 * - 提供馆藏地址和图书类别的下拉选择
 * - 支持价格的数值验证
 * - 提供入库日期的日历选择
 * - 支持多行内容简介的编辑
 * - 根据索引号前缀自动推荐图书类别
 *
 * @note 对话框使用固定尺寸以确保UI布局的一致性
 * @note 所有输入字段都有适当的验证和默认值
 * @note 支持中文输入和UTF-8编码
 */
class BookDialog : public QDialog {
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     *
     * 创建图书信息编辑对话框并初始化所有UI组件。
     * 默认情况下对话框处于添加新书模式，字段为空或使用默认值。
     *
     * @param parent 父窗口指针，默认为nullptr，表示顶级窗口
     */
    explicit BookDialog(QWidget *parent = nullptr);

    /**
     * @brief 设置要编辑的图书信息
     *
     * 将图书对象的信息填充到对话框的各个输入字段中。
     * 调用此方法后，对话框将切换到编辑模式。
     *
     * @param book 要编辑的图书对象常量引用
     * @note 该方法会解析索引号并设置对应的前缀和数值
     * @note 日期字段会自动验证，无效日期使用当前日期
     */
    void setBook(const Book &book);

    /**
     * @brief 获取用户输入的图书信息
     *
     * 从对话框的各个输入字段中收集数据并构建Book对象。
     * 该方法通常在用户点击确认按钮后调用。
     *
     * @return 返回包含用户输入信息的Book对象
     * @note 返回的Book对象可能包含部分空字段，需要调用者进行验证
     * @note 索引号会自动格式化为3位数字编号
     */
    Book getBook() const;

private slots:
    /**
     * @brief 索引号前缀变更槽函数
     *
     * 当用户选择不同的索引号前缀时自动调用。
     * 根据选择的前缀自动设置相应的图书类别推荐。
     *
     * 支持的前缀到类别的映射：
     * - CS -> 计算机科学
     * - LIT -> 文学
     * - HIS -> 历史
     * - SCI -> 科学
     * - ENG -> 外语
     * - ART -> 艺术
     * - PHI -> 哲学
     *
     * @note 用户可以手动修改自动设置的类别
     * @note 该槽函数与indexPrefixCombo_的currentIndexChanged信号连接
     */
    void onIndexPrefixChanged();

private:
    // 索引号输入组件
    QComboBox *indexPrefixCombo_ = nullptr;    ///< 索引号前缀选择下拉框，支持不同类别的图书分类
    QLineEdit *indexNumberEdit_ = nullptr;     ///< 索引号数字输入框，支持1-999的数字输入

    // 基本信息输入组件
    QLineEdit *nameEdit_ = nullptr;            ///< 图书书名输入框，支持中英文输入
    QLineEdit *authorEdit_ = nullptr;          ///< 图书作者输入框，支持多作者输入
    QLineEdit *publisherEdit_ = nullptr;       ///< 图书出版社输入框，支持国内外出版社
    QComboBox *locationEdit_ = nullptr;        ///< 馆藏地址选择下拉框，支持多校区管理
    QComboBox *categoryEdit_ = nullptr;        ///< 图书类别选择下拉框，支持自定义输入
    QLineEdit *priceEdit_ = nullptr;           ///< 图书价格输入框，支持小数点后两位
    QDateEdit *inDateEdit_ = nullptr;          ///< 入库日期选择器，支持日历弹窗选择
    QTextEdit *descriptionEdit_ = nullptr;     ///< 内容简介多行文本编辑框，支持长文本输入
};

#endif //BOOKDISPLAY_H

