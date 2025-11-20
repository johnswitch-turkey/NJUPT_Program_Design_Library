// bookdisplay.cpp
#include "bookdisplay.h"

#include <QIntValidator>
#include <QDoubleValidator>

BookDialog::BookDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(QStringLiteral("📚 图书信息"));
    setFixedSize(500, 500);
    setModal(true);

    indexIdEdit_ = new QLineEdit(this);
    nameEdit_ = new QLineEdit(this);
    authorEdit_ = new QLineEdit(this);
    publisherEdit_ = new QLineEdit(this);
    locationEdit_ = new QComboBox(this);
    categoryEdit_ = new QComboBox(this);
    priceEdit_ = new QLineEdit(this);
    inDateEdit_ = new QDateEdit(this);

    auto *priceValidator = new QDoubleValidator(0.0, 1e9, 2, this);
    priceValidator->setNotation(QDoubleValidator::StandardNotation);
    priceEdit_->setValidator(priceValidator);

    inDateEdit_->setCalendarPopup(true);
    inDateEdit_->setDisplayFormat("yyyy-MM-dd");

    // 设置馆藏地址下拉框选项
    locationEdit_->addItem(QStringLiteral("三牌楼"));
    locationEdit_->addItem(QStringLiteral("仙林"));
    locationEdit_->setCurrentIndex(0);

    // 设置类别下拉框选项
    categoryEdit_->addItem(QStringLiteral("人文"));
    categoryEdit_->addItem(QStringLiteral("科技"));
    categoryEdit_->addItem(QStringLiteral("外语"));
    categoryEdit_->addItem(QStringLiteral("艺术"));
    categoryEdit_->addItem(QStringLiteral("历史"));
    categoryEdit_->addItem(QStringLiteral("哲学"));
    categoryEdit_->addItem(QStringLiteral("经济"));
    categoryEdit_->addItem(QStringLiteral("管理"));
    categoryEdit_->addItem(QStringLiteral("法律"));
    categoryEdit_->addItem(QStringLiteral("医学"));
    categoryEdit_->addItem(QStringLiteral("工程"));
    categoryEdit_->addItem(QStringLiteral("其他"));
    categoryEdit_->setCurrentIndex(0);
    categoryEdit_->setEditable(true);

    // 美化输入框样式
    QString inputStyle =
        "QLineEdit, QDateEdit, QComboBox {"
        "    padding: 10px;"
        "    border: 2px solid #e9ecef;"
        "    border-radius: 8px;"
        "    background-color: #ffffff;"
        "    font-size: 13px;"
        "    color: #495057;"
        "}"
        "QLineEdit:focus, QDateEdit:focus, QComboBox:focus {"
        "    border-color: #007bff;"
        "    background-color: #f8f9ff;"
        "}"
        "QComboBox::drop-down {"
        "    border: none;"
        "    width: 20px;"
        "}"
        "QComboBox::down-arrow {"
        "    image: url(data:image/svg+xml;base64,PHN2ZyB3aWR0aD0iMTIiIGhlaWdodD0iOCIgdmlld0JveD0iMCAwIDEyIDgiIGZpbGw9Im5vbmUiIHhtbG5zPSJodHRwOi8vd3d3LnczLm9yZy8yMDAwL3N2ZyI+CjxwYXRoIGQ9Ik0xIDFMNiA2TDExIDEiIHN0cm9rZT0iIzQ5NTA1NyIgc3Ryb2tlLXdpZHRoPSIyIiBzdHJva2UtbGluZWNhcD0icm91bmQiIHN0cm9rZS1saW5lam9pbj0icm91bmQiLz4KPC9zdmc+Cg==);"
        "    width: 12px;"
        "    height: 8px;"
        "}"
        "QComboBox QAbstractItemView {"
        "    border: 2px solid #e9ecef;"
        "    border-radius: 8px;"
        "    background-color: #ffffff;"
        "    selection-background-color: #007bff;"
        "    selection-color: white;"
        "    padding: 5px;"
        "}";

    indexIdEdit_->setStyleSheet(inputStyle);
    nameEdit_->setStyleSheet(inputStyle);
    authorEdit_->setStyleSheet(inputStyle);
    publisherEdit_->setStyleSheet(inputStyle);
    locationEdit_->setStyleSheet(inputStyle);
    categoryEdit_->setStyleSheet(inputStyle);
    priceEdit_->setStyleSheet(inputStyle);
    inDateEdit_->setStyleSheet(inputStyle);

    auto *form = new QFormLayout(this);
    form->setSpacing(15);
    form->setVerticalSpacing(20);
    locationEdit_->setMinimumHeight(40);
    categoryEdit_->setMinimumHeight(40);
    inDateEdit_->setMinimumHeight(40);
    form->setContentsMargins(30, 30, 30, 30);

    // 添加图标美化标签
    form->addRow(QStringLiteral("🔢 索引号"), indexIdEdit_);
    form->addRow(QStringLiteral("📖 名称"), nameEdit_);
    form->addRow(QStringLiteral("✍️ 作者"), authorEdit_);
    form->addRow(QStringLiteral("🏢 出版社"), publisherEdit_);
    form->addRow(QStringLiteral("📍 馆藏地址"), locationEdit_);
    form->addRow(QStringLiteral("📂 类别"), categoryEdit_);
    form->addRow(QStringLiteral("💰 价格"), priceEdit_);
    form->addRow(QStringLiteral("📅 入库日期"), inDateEdit_);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    buttons->setStyleSheet(
        "QPushButton {"
        "    background-color: #007bff;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 6px;"
        "    padding: 10px 20px;"
        "    font-size: 13px;"
        "    font-weight: bold;"
        "    min-width: 80px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #0056b3;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #004085;"
        "}"
        "QPushButton[text=\"Cancel\"] {"
        "    background-color: #6c757d;"
        "}"
        "QPushButton[text=\"Cancel\"]:hover {"
        "    background-color: #545b62;"
        "}"
    );

    form->addRow(buttons);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    // 设置对话框样式
    setStyleSheet(
        "QDialog {"
        "    background-color: #f8f9fa;"
        "    border: 1px solid #dee2e6;"
        "    border-radius: 12px;"
        "}"
        "QLabel {"
        "    font-size: 13px;"
        "    font-weight: bold;"
        "    color: #495057;"
        "    margin-bottom: 5px;"
        "}"
    );
}

void BookDialog::setBook(const Book &b)
{
    indexIdEdit_->setText(b.indexId);
    nameEdit_->setText(b.name);
    authorEdit_->setText(b.author);
    publisherEdit_->setText(b.publisher);

    // 设置馆藏地址下拉框
    int locationIndex = locationEdit_->findText(b.location);
    if (locationIndex >= 0) {
        locationEdit_->setCurrentIndex(locationIndex);
    } else {
        locationEdit_->setCurrentIndex(0);
    }

    // 设置类别下拉框
    int categoryIndex = categoryEdit_->findText(b.category);
    if (categoryIndex >= 0) {
        categoryEdit_->setCurrentIndex(categoryIndex);
    } else {
        categoryEdit_->setCurrentText(b.category);
    }

    priceEdit_->setText(QString::number(b.price, 'f', 2));
    inDateEdit_->setDate(b.inDate.isValid() ? b.inDate : QDate::currentDate());
}

Book BookDialog::getBook() const
{
    Book b;
    b.indexId = indexIdEdit_->text().trimmed();
    b.name = nameEdit_->text().trimmed();
    b.author = authorEdit_->text().trimmed();
    b.publisher = publisherEdit_->text().trimmed();
    b.location = locationEdit_->currentText();
    b.category = categoryEdit_->currentText().trimmed();
    b.price = priceEdit_->text().toDouble();
    b.inDate = inDateEdit_->date();
    return b;
}


