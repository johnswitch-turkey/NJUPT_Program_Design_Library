
#include "bookdetaildialog.h"
#include "../utils/bookcopymanager.h"

// Qt核心UI组件
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QGroupBox>
#include <QFrame>
#include <QFont>
#include <QPixmap>
/**
 * @brief BookDetailDialog构造函数
 *
 * 创建图书详情对话框并初始化所有UI组件和数据。
 * 构造函数会按顺序调用各个设置函数来建立完整的用户界面。
 *
 * @param book 要显示详情的图书对象常量引用
 * @param parent 父窗口指针，默认为nullptr，表示顶级窗口
 *
 * @note 构造函数会自动调用setupUI()、setupDescription()、setupBookInfo()和setupCopiesInfo()
 * @note 对话框设置为模态对话框，用户必须关闭后才能操作其他窗口
 */
BookDetailDialog::BookDetailDialog(const Book &book, QWidget *parent)
    : QDialog(parent), book_(book)
{
    // 按顺序初始化各个UI组件
    setupUI();           // 设置主要UI结构和布局
    setupDescription();  // 设置内容简介显示
    setupBookInfo();     // 设置基本信息显示
    setupCopiesInfo();   // 设置副本信息显示
}

void BookDetailDialog::setupUI()
{
    // 设置对话框基本属性
    setWindowTitle(QStringLiteral("图书详情"));      ///< 设置窗口标题
    setMinimumSize(600, 400);                       ///< 设置最小尺寸，确保UI正常显示
    resize(650, 500);                               ///< 设置默认窗口大小

    // 创建主布局管理器
    mainLayout_ = new QVBoxLayout(this);

    // 创建滚动区域以支持内容超出窗口时的滚动显示
    scrollArea_ = new QScrollArea(this);
    scrollArea_->setWidgetResizable(true);                                            ///< 允许内容 widget 自动调整大小
    scrollArea_->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);                ///< 水平滚动条：需要时显示
    scrollArea_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);                  ///< 垂直滚动条：需要时显示

    // 创建内容容器 widget，包含所有信息组
    contentWidget_ = new QWidget();
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget_);

    // 创建内容简介组容器
    descriptionGroup_ = new QGroupBox(QStringLiteral("内容简介"), contentWidget_);
    descriptionLayout_ = new QVBoxLayout(descriptionGroup_);
    contentLayout->addWidget(descriptionGroup_);

    // 创建基本信息组容器
    bookInfoGroup_ = new QGroupBox(QStringLiteral("基本信息"), contentWidget_);
    bookInfoLayout_ = new QVBoxLayout(bookInfoGroup_);
    contentLayout->addWidget(bookInfoGroup_);

    // 创建副本信息组容器
    copiesGroup_ = new QGroupBox(QStringLiteral("副本信息"), contentWidget_);
    copiesLayout_ = new QVBoxLayout(copiesGroup_);
    contentLayout->addWidget(copiesGroup_);

    // 添加弹性空间，推动内容向上对齐
    contentLayout->addStretch();

    // 将内容 widget 设置为滚动区域的子控件
    scrollArea_->setWidget(contentWidget_);
    mainLayout_->addWidget(scrollArea_);

    // 创建并配置关闭按钮
    closeButton_ = new QPushButton(QStringLiteral("关闭"), this);
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();                        ///< 左侧添加弹性空间，使按钮右对齐
    buttonLayout->addWidget(closeButton_);             ///< 添加关闭按钮
    mainLayout_->addLayout(buttonLayout);             ///< 将按钮布局添加到主布局

    // 连接关闭按钮的点击信号到对话框的 accept 槽
    connect(closeButton_, &QPushButton::clicked, this, &QDialog::accept);

    // 设置样式
    descriptionGroup_->setStyleSheet(
        "QGroupBox {"
        "   font-weight: bold;"
        "   border: 2px solid #cccccc;"
        "   border-radius: 5px;"
        "   margin-top: 10px;"
        "   padding-top: 10px;"
        "}"
        "QGroupBox::title {"
        "   subcontrol-origin: margin;"
        "   left: 10px;"
        "   padding: 0 5px 0 5px;"
        "}"
    );

    bookInfoGroup_->setStyleSheet(
        "QGroupBox {"
        "   font-weight: bold;"
        "   border: 2px solid #cccccc;"
        "   border-radius: 5px;"
        "   margin-top: 10px;"
        "   padding-top: 10px;"
        "}"
        "QGroupBox::title {"
        "   subcontrol-origin: margin;"
        "   left: 10px;"
        "   padding: 0 5px 0 5px;"
        "}"
    );

    copiesGroup_->setStyleSheet(
        "QGroupBox {"
        "   font-weight: bold;"
        "   border: 2px solid #cccccc;"
        "   border-radius: 5px;"
        "   margin-top: 10px;"
        "   padding-top: 10px;"
        "}"
        "QGroupBox::title {"
        "   subcontrol-origin: margin;"
        "   left: 10px;"
        "   padding: 0 5px 0 5px;"
        "}"
    );
}

void BookDetailDialog::setupBookInfo()
{
    // 清除现有内容
    QLayoutItem *item;
    while ((item = bookInfoLayout_->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    // 创建标签样式
    QString labelStyle = "QLabel { color: #666666; font-weight: bold; }";
    QString valueStyle = "QLabel { color: #333333; padding: 5px; background-color: #f9f9f9; border-radius: 3px; }";

    // 索引号
    QHBoxLayout *indexLayout = new QHBoxLayout();
    indexIdLabel_ = new QLabel(QStringLiteral("索引号："));
    indexIdLabel_->setStyleSheet(labelStyle);
    QLabel *indexValue = new QLabel(book_.indexId);
    indexValue->setStyleSheet(valueStyle);
    indexLayout->addWidget(indexIdLabel_);
    indexLayout->addWidget(indexValue);
    indexLayout->addStretch();
    bookInfoLayout_->addLayout(indexLayout);

    // 书名
    QHBoxLayout *nameLayout = new QHBoxLayout();
    nameLabel_ = new QLabel(QStringLiteral("书名："));
    nameLabel_->setStyleSheet(labelStyle);
    QLabel *nameValue = new QLabel(book_.name);
    nameValue->setStyleSheet(valueStyle);
    nameValue->setWordWrap(true);
    nameLayout->addWidget(nameLabel_);
    nameLayout->addWidget(nameValue, 1);
    bookInfoLayout_->addLayout(nameLayout);

    // 作者
    QHBoxLayout *authorLayout = new QHBoxLayout();
    authorLabel_ = new QLabel(QStringLiteral("作者："));
    authorLabel_->setStyleSheet(labelStyle);
    QLabel *authorValue = new QLabel(book_.author.isEmpty() ? QStringLiteral("未知") : book_.author);
    authorValue->setStyleSheet(valueStyle);
    authorLayout->addWidget(authorLabel_);
    authorLayout->addWidget(authorValue, 1);
    bookInfoLayout_->addLayout(authorLayout);

    // 出版社
    QHBoxLayout *publisherLayout = new QHBoxLayout();
    publisherLabel_ = new QLabel(QStringLiteral("出版社："));
    publisherLabel_->setStyleSheet(labelStyle);
    QLabel *publisherValue = new QLabel(book_.publisher.isEmpty() ? QStringLiteral("未知") : book_.publisher);
    publisherValue->setStyleSheet(valueStyle);
    publisherLayout->addWidget(publisherLabel_);
    publisherLayout->addWidget(publisherValue, 1);
    bookInfoLayout_->addLayout(publisherLayout);

    // 馆藏地址
    QHBoxLayout *locationLayout = new QHBoxLayout();
    locationLabel_ = new QLabel(QStringLiteral("馆藏地址："));
    locationLabel_->setStyleSheet(labelStyle);
    QLabel *locationValue = new QLabel(book_.location);
    locationValue->setStyleSheet(valueStyle);
    locationLayout->addWidget(locationLabel_);
    locationLayout->addWidget(locationValue, 1);
    bookInfoLayout_->addLayout(locationLayout);

    // 类别
    QHBoxLayout *categoryLayout = new QHBoxLayout();
    categoryLabel_ = new QLabel(QStringLiteral("类别："));
    categoryLabel_->setStyleSheet(labelStyle);
    QLabel *categoryValue = new QLabel(book_.category);
    categoryValue->setStyleSheet(valueStyle);
    categoryLayout->addWidget(categoryLabel_);
    categoryLayout->addWidget(categoryValue, 1);
    bookInfoLayout_->addLayout(categoryLayout);

    // 价格
    QHBoxLayout *priceLayout = new QHBoxLayout();
    priceLabel_ = new QLabel(QStringLiteral("价格："));
    priceLabel_->setStyleSheet(labelStyle);
    QLabel *priceValue = new QLabel(QString("¥%1").arg(book_.price));
    priceValue->setStyleSheet(valueStyle);
    priceLayout->addWidget(priceLabel_);
    priceLayout->addWidget(priceValue, 1);
    bookInfoLayout_->addLayout(priceLayout);

    // 入库日期
    QHBoxLayout *inDateLayout = new QHBoxLayout();
    inDateLabel_ = new QLabel(QStringLiteral("入库日期："));
    inDateLabel_->setStyleSheet(labelStyle);
    QLabel *inDateValue = new QLabel(book_.inDate.toString("yyyy-MM-dd"));
    inDateValue->setStyleSheet(valueStyle);
    inDateLayout->addWidget(inDateLabel_);
    inDateLayout->addWidget(inDateValue, 1);
    bookInfoLayout_->addLayout(inDateLayout);

    // 借阅次数
    QHBoxLayout *borrowCountLayout = new QHBoxLayout();
    borrowCountLabel_ = new QLabel(QStringLiteral("借阅次数："));
    borrowCountLabel_->setStyleSheet(labelStyle);
    QLabel *borrowCountValue = new QLabel(QString::number(book_.borrowCount));
    borrowCountValue->setStyleSheet(valueStyle);
    borrowCountLayout->addWidget(borrowCountLabel_);
    borrowCountLayout->addWidget(borrowCountValue, 1);
    bookInfoLayout_->addLayout(borrowCountLayout);
}

void BookDetailDialog::setupCopiesInfo()
{
    // 清除现有内容
    QLayoutItem *item;
    while ((item = copiesLayout_->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    // 获取该书的所有副本信息
    BookCopyManager &copyManager = BookCopyManager::instance();
    QVector<BookCopy> copies = copyManager.getCopiesByIndexId(book_.indexId);

    if (copies.isEmpty()) {
        QLabel *noCopiesLabel = new QLabel(QStringLiteral("暂无副本信息"));
        noCopiesLabel->setStyleSheet("QLabel { color: #999999; font-style: italic; padding: 20px; }");
        copiesLayout_->addWidget(noCopiesLabel);
        return;
    }

    // 创建副本列表表头
    QHBoxLayout *headerLayout = new QHBoxLayout();
    QStringList headers = {QStringLiteral("副本编号"), QStringLiteral("状态"), QStringLiteral("借阅者"), QStringLiteral("应还日期")};
    QStringList headerStyles = {
        "QLabel { color: #666666; font-weight: bold; border-bottom: 2px solid #cccccc; padding: 5px; }"
    };

    for (const QString &header : headers) {
        QLabel *headerLabel = new QLabel(header);
        headerLabel->setStyleSheet(headerStyles.first());
        headerLabel->setMinimumWidth(120);
        headerLayout->addWidget(headerLabel);
    }
    copiesLayout_->addLayout(headerLayout);

    // 添加每个副本的信息
    for (const BookCopy &copy : copies) {
        QHBoxLayout *copyLayout = new QHBoxLayout();

        // 副本编号
        QLabel *copyIdLabel = new QLabel(copy.copyId);
        copyIdLabel->setStyleSheet("QLabel { padding: 5px; border-bottom: 1px solid #eeeeee; }");
        copyIdLabel->setMinimumWidth(120);

        // 状态
        QLabel *statusLabel = new QLabel(copy.isAvailable() ? QStringLiteral("可借") : QStringLiteral("已借出"));
        QString statusStyle = copy.isAvailable()
            ? "QLabel { padding: 5px; color: #388e3c; font-weight: bold; border-bottom: 1px solid #eeeeee; }"
            : "QLabel { padding: 5px; color: #d32f2f; font-weight: bold; border-bottom: 1px solid #eeeeee; }";
        statusLabel->setStyleSheet(statusStyle);
        statusLabel->setMinimumWidth(120);

        // 借阅者
        QLabel *borrowerLabel = new QLabel(copy.isAvailable() ? QStringLiteral("-") : copy.borrowedBy);
        borrowerLabel->setStyleSheet("QLabel { padding: 5px; border-bottom: 1px solid #eeeeee; }");
        borrowerLabel->setMinimumWidth(120);

        // 应还日期
        QLabel *dueDateLabel = new QLabel(copy.isAvailable() ? QStringLiteral("-") : copy.dueDate.toString("yyyy-MM-dd"));
        dueDateLabel->setStyleSheet("QLabel { padding: 5px; border-bottom: 1px solid #eeeeee; }");
        dueDateLabel->setMinimumWidth(120);

        copyLayout->addWidget(copyIdLabel);
        copyLayout->addWidget(statusLabel);
        copyLayout->addWidget(borrowerLabel);
        copyLayout->addWidget(dueDateLabel);

        copiesLayout_->addLayout(copyLayout);
    }

    // 添加统计信息
    int availableCopies = 0;
    int borrowedCopies = 0;
    for (const BookCopy &copy : copies) {
        if (copy.isAvailable()) {
            availableCopies++;
        } else {
            borrowedCopies++;
        }
    }

    QHBoxLayout *statsLayout = new QHBoxLayout();
    QLabel *totalLabel = new QLabel(QStringLiteral("总计：%1个副本").arg(copies.size()));
    totalLabel->setStyleSheet("QLabel { color: #666666; font-weight: bold; margin-top: 10px; }");

    QLabel *availableLabel = new QLabel(QStringLiteral("可借：%1本").arg(availableCopies));
    availableLabel->setStyleSheet("QLabel { color: #388e3c; font-weight: bold; margin-top: 10px; }");

    QLabel *borrowedLabel = new QLabel(QStringLiteral("已借：%1本").arg(borrowedCopies));
    borrowedLabel->setStyleSheet("QLabel { color: #d32f2f; font-weight: bold; margin-top: 10px; }");

    statsLayout->addWidget(totalLabel);
    statsLayout->addStretch();
    statsLayout->addWidget(availableLabel);
    statsLayout->addWidget(borrowedLabel);

    copiesLayout_->addLayout(statsLayout);
}

void BookDetailDialog::setupDescription()
{
    // 清除现有内容
    QLayoutItem *item;
    while ((item = descriptionLayout_->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    // 创建内容简介显示
    QString descriptionText = book_.description.isEmpty() ?
        QStringLiteral("暂无内容简介") : book_.description;

    descriptionLabel_ = new QLabel(descriptionText);
    descriptionLabel_->setWordWrap(true);
    descriptionLabel_->setStyleSheet(
        "QLabel {"
        "   color: #333333;"
        "   padding: 15px;"
        "   background-color: #f9f9f9;"
        "   border-radius: 5px;"
        "   line-height: 1.5;"
        "}"
    );

    // 设置最小高度以确保显示区域合适
    descriptionLabel_->setMinimumHeight(60);

    descriptionLayout_->addWidget(descriptionLabel_);
}