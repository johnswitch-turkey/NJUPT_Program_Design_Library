#include "bookdetaildialog.h"
#include "../utils/bookcopymanager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QGroupBox>
#include <QFrame>
#include <QFont>
#include <QPixmap>

BookDetailDialog::BookDetailDialog(const Book &book, QWidget *parent)
    : QDialog(parent), book_(book)
{
    setupUI();
    setupBookInfo();
    setupCopiesInfo();
}

void BookDetailDialog::setupUI()
{
    setWindowTitle(QStringLiteral("图书详情"));
    setMinimumSize(600, 400);
    resize(650, 500);

    mainLayout_ = new QVBoxLayout(this);

    // 创建滚动区域
    scrollArea_ = new QScrollArea(this);
    scrollArea_->setWidgetResizable(true);
    scrollArea_->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    contentWidget_ = new QWidget();
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget_);

    // 图书信息组
    bookInfoGroup_ = new QGroupBox(QStringLiteral("基本信息"), contentWidget_);
    bookInfoLayout_ = new QVBoxLayout(bookInfoGroup_);
    contentLayout->addWidget(bookInfoGroup_);

    // 副本信息组
    copiesGroup_ = new QGroupBox(QStringLiteral("副本信息"), contentWidget_);
    copiesLayout_ = new QVBoxLayout(copiesGroup_);
    contentLayout->addWidget(copiesGroup_);

    contentLayout->addStretch();

    scrollArea_->setWidget(contentWidget_);
    mainLayout_->addWidget(scrollArea_);

    // 关闭按钮
    closeButton_ = new QPushButton(QStringLiteral("关闭"), this);
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton_);
    mainLayout_->addLayout(buttonLayout);

    connect(closeButton_, &QPushButton::clicked, this, &QDialog::accept);

    // 设置样式
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