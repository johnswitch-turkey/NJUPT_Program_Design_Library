// borrowdialog.cpp
#include "borrowdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QDateEdit>
#include <QPushButton>
#include <QGroupBox>
#include <QFormLayout>
#include <QMessageBox>

BorrowDialog::BorrowDialog(const Book &book, const QVector<BookCopy> &availableCopies, QWidget *parent)
    : QDialog(parent), book_(book), availableCopies_(availableCopies)
{
    setupUI();
    validateInput();
}

BorrowDialog::~BorrowDialog()
{
}

void BorrowDialog::setupUI()
{
    setWindowTitle(QStringLiteral("借阅图书"));
    setMinimumWidth(400);
    setModal(true);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // 图书信息
    bookInfoLabel_ = new QLabel();
    bookInfoLabel_->setWordWrap(true);
    bookInfoLabel_->setStyleSheet("QLabel { font-weight: bold; font-size: 14px; color: #2c3e50; padding: 10px; background-color: #f8f9fa; border-radius: 8px; }");

    QString bookInfo = QStringLiteral("📚 《%1》\n📖 索引号：%2\n✍️ 作者：%3\n🏢 出版社：%4")
                       .arg(book_.name)
                       .arg(book_.indexId)
                       .arg(book_.author)
                       .arg(book_.publisher);
    bookInfoLabel_->setText(bookInfo);
    mainLayout->addWidget(bookInfoLabel_);

    // 选择副本
    QGroupBox *copyGroup = new QGroupBox(QStringLiteral("选择副本"));
    QVBoxLayout *copyLayout = new QVBoxLayout(copyGroup);

    copyComboBox_ = new QComboBox();
    for (const BookCopy &copy : availableCopies_) {
        QString copyText = QStringLiteral("副本 %1 (%2)")
                          .arg(copy.copyNumber)
                          .arg(copy.copyId);
        copyComboBox_->addItem(copyText, copy.copyId);
    }

    if (availableCopies_.isEmpty()) {
        copyComboBox_->addItem(QStringLiteral("暂无可借副本"), QString());
        copyComboBox_->setEnabled(false);
    }

    connect(copyComboBox_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &BorrowDialog::updateCopyInfo);

    copyLayout->addWidget(copyComboBox_);
    mainLayout->addWidget(copyGroup);

    // 归还日期
    QGroupBox *dateGroup = new QGroupBox(QStringLiteral("归还日期"));
    QVBoxLayout *dateLayout = new QVBoxLayout(dateGroup);

    dueDateEdit_ = new QDateEdit();
    dueDateEdit_->setCalendarPopup(true);
    dueDateEdit_->setMinimumDate(QDate::currentDate().addDays(1));
    dueDateEdit_->setMaximumDate(QDate::currentDate().addDays(90));
    dueDateEdit_->setDate(QDate::currentDate().addDays(30));
    dueDateEdit_->setDisplayFormat("yyyy-MM-dd");

    dateLayout->addWidget(dueDateEdit_);
    mainLayout->addWidget(dateGroup);

    // 状态标签
    statusLabel_ = new QLabel();
    statusLabel_->setWordWrap(true);
    statusLabel_->setStyleSheet("QLabel { color: #e74c3c; padding: 8px; }");
    mainLayout->addWidget(statusLabel_);

    // 按钮
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    okButton_ = new QPushButton(QStringLiteral("确认借阅"));
    okButton_->setMinimumWidth(100);
    cancelButton_ = new QPushButton(QStringLiteral("取消"));
    cancelButton_->setMinimumWidth(100);

    buttonLayout->addWidget(okButton_);
    buttonLayout->addWidget(cancelButton_);
    mainLayout->addLayout(buttonLayout);

    connect(okButton_, &QPushButton::clicked, this, &BorrowDialog::onOkClicked);
    connect(cancelButton_, &QPushButton::clicked, this, &BorrowDialog::onCancelClicked);
    connect(dueDateEdit_, &QDateEdit::dateChanged, this, &BorrowDialog::validateInput);

    updateCopyInfo();
}

void BorrowDialog::updateCopyInfo()
{
    if (availableCopies_.isEmpty()) {
        statusLabel_->setText("❌ 该图书暂无可借副本");
        return;
    }

    int currentIndex = copyComboBox_->currentIndex();
    if (currentIndex >= 0 && currentIndex < availableCopies_.size()) {
        selectedCopy_ = availableCopies_[currentIndex];
        statusLabel_->setText(QString("✅ 已选择副本 %1").arg(selectedCopy_.copyNumber));
    }

    validateInput();
}

void BorrowDialog::validateInput()
{
    bool isValid = true;
    QString statusText;

    if (availableCopies_.isEmpty()) {
        isValid = false;
        statusText = "❌ 该图书暂无可借副本";
    } else if (dueDateEdit_->date() <= QDate::currentDate()) {
        isValid = false;
        statusText = "❌ 归还日期必须晚于当前日期";
    } else if (dueDateEdit_->date() > QDate::currentDate().addDays(90)) {
        isValid = false;
        statusText = "❌ 归还日期不能超过30天";
    } else {
        statusText = QString("✅ 已选择副本 %1，归还日期：%2")
                     .arg(selectedCopy_.copyNumber)
                     .arg(dueDateEdit_->date().toString("yyyy-MM-dd"));
    }

    statusLabel_->setText(statusText);
    okButton_->setEnabled(isValid);
}

BookCopy BorrowDialog::getSelectedCopy() const
{
    return selectedCopy_;
}

QDate BorrowDialog::getDueDate() const
{
    return dueDateEdit_->date();
}

void BorrowDialog::onOkClicked()
{
    if (!okButton_->isEnabled()) {
        return;
    }

    accept();
}

void BorrowDialog::onCancelClicked()
{
    reject();
}