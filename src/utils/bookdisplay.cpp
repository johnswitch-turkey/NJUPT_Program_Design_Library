// bookdisplay.cpp
#include "bookdisplay.h"

#include <QIntValidator>
#include <QDoubleValidator>
#include <QRegularExpression>
#include <QHBoxLayout>

BookDialog::BookDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(QStringLiteral("📚 图书信息"));
    setFixedSize(550, 700);
    setModal(true);

    // 创建索引号前缀下拉框和数字输入框
    indexPrefixCombo_ = new QComboBox(this);
    indexNumberEdit_ = new QLineEdit(this);
    nameEdit_ = new QLineEdit(this);
    authorEdit_ = new QLineEdit(this);
    publisherEdit_ = new QLineEdit(this);
    locationEdit_ = new QComboBox(this);
    categoryEdit_ = new QComboBox(this);
    priceEdit_ = new QLineEdit(this);
    inDateEdit_ = new QDateEdit(this);
    descriptionEdit_ = new QTextEdit(this);

    // 设置索引号前缀选项
    indexPrefixCombo_->addItem("CS - 计算机科学", "CS");
    indexPrefixCombo_->addItem("LIT - 文学", "LIT");
    indexPrefixCombo_->addItem("HIS - 历史", "HIS");
    indexPrefixCombo_->addItem("SCI - 科学", "SCI");
    indexPrefixCombo_->addItem("ENG - 外语", "ENG");
    indexPrefixCombo_->addItem("ART - 艺术", "ART");
    indexPrefixCombo_->addItem("PHI - 哲学", "PHI");
    indexPrefixCombo_->addItem("ECO - 经济", "ECO");
    indexPrefixCombo_->addItem("MGT - 管理", "MGT");
    indexPrefixCombo_->addItem("LAW - 法律", "LAW");
    indexPrefixCombo_->addItem("MED - 医学", "MED");
    indexPrefixCombo_->addItem("ENG - 工程", "ENG");
    indexPrefixCombo_->addItem("OTH - 其他", "OTH");
    indexPrefixCombo_->setCurrentIndex(0);

    // 设置数字输入框限制（只能输入数字，最多3位）
    auto *numberValidator = new QIntValidator(1, 999, this);
    indexNumberEdit_->setValidator(numberValidator);
    indexNumberEdit_->setPlaceholderText("请输入数字（如：001）");

    auto *priceValidator = new QDoubleValidator(0.0, 1e9, 2, this);
    priceValidator->setNotation(QDoubleValidator::StandardNotation);
    priceEdit_->setValidator(priceValidator);

    inDateEdit_->setCalendarPopup(true);
    inDateEdit_->setDisplayFormat("yyyy-MM-dd");
    // 设置入库日期默认为当前日期
    inDateEdit_->setDate(QDate::currentDate());

    // 设置内容简介编辑框
    descriptionEdit_->setPlaceholderText(QStringLiteral("请输入图书的内容简介..."));
    descriptionEdit_->setMaximumHeight(120);
    descriptionEdit_->setMinimumHeight(80);

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
        "QLineEdit, QDateEdit, QComboBox, QTextEdit {"
        "    padding: 10px;"
        "    border: 2px solid #e9ecef;"
        "    border-radius: 8px;"
        "    background-color: #ffffff;"
        "    font-size: 13px;"
        "    color: #495057;"
        "}"
        "QLineEdit:focus, QDateEdit:focus, QComboBox:focus, QTextEdit:focus {"
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

    indexPrefixCombo_->setStyleSheet(inputStyle);
    indexNumberEdit_->setStyleSheet(inputStyle);
    nameEdit_->setStyleSheet(inputStyle);
    authorEdit_->setStyleSheet(inputStyle);
    publisherEdit_->setStyleSheet(inputStyle);
    locationEdit_->setStyleSheet(inputStyle);
    categoryEdit_->setStyleSheet(inputStyle);
    priceEdit_->setStyleSheet(inputStyle);
    inDateEdit_->setStyleSheet(inputStyle);
    descriptionEdit_->setStyleSheet(inputStyle);

    // 设置组合框最小高度
    indexPrefixCombo_->setMinimumHeight(40);
    indexNumberEdit_->setMinimumHeight(40);
    locationEdit_->setMinimumHeight(40);
    categoryEdit_->setMinimumHeight(40);
    inDateEdit_->setMinimumHeight(40);

    auto *form = new QFormLayout(this);
    form->setSpacing(15);
    form->setVerticalSpacing(20);
    form->setContentsMargins(30, 30, 30, 30);

    // 创建索引号的水平布局
    auto *indexLayout = new QHBoxLayout();
    indexLayout->addWidget(indexPrefixCombo_, 2); // 前缀占2/3宽度
    indexLayout->addWidget(indexNumberEdit_, 1);   // 数字占1/3宽度
    indexLayout->setSpacing(5);

    // 添加图标美化标签
    form->addRow(QStringLiteral("🔢 索引号"), indexLayout);
    form->addRow(QStringLiteral("📖 名称"), nameEdit_);
    form->addRow(QStringLiteral("✍️ 作者"), authorEdit_);
    form->addRow(QStringLiteral("🏢 出版社"), publisherEdit_);
    form->addRow(QStringLiteral("📍 馆藏地址"), locationEdit_);
    form->addRow(QStringLiteral("📂 类别"), categoryEdit_);
    form->addRow(QStringLiteral("💰 价格"), priceEdit_);
    form->addRow(QStringLiteral("📅 入库日期"), inDateEdit_);
    form->addRow(QStringLiteral("📝 内容简介"), descriptionEdit_);

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

    // 连接索引号前缀变化的信号
    connect(indexPrefixCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &BookDialog::onIndexPrefixChanged);

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
    // 解析索引号
    QString indexId = b.indexId;
    QString prefix = "CS"; // 默认前缀
    QString number = "";

    // 使用正则表达式或字符串解析来分离前缀和数字
    QRegularExpression re("([A-Z]+)(\\d+)");
    QRegularExpressionMatch match = re.match(indexId);
    if (match.hasMatch()) {
        prefix = match.captured(1);
        number = match.captured(2);
    }

    // 设置索引号前缀
    int prefixIndex = indexPrefixCombo_->findData(prefix);
    if (prefixIndex >= 0) {
        indexPrefixCombo_->setCurrentIndex(prefixIndex);
    }

    // 设置索引号数字
    indexNumberEdit_->setText(number);

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
    descriptionEdit_->setText(b.description);
}

Book BookDialog::getBook() const
{
    Book b;

    // 组合索引号：前缀 + 数字（补零到3位）
    QString prefix = indexPrefixCombo_->currentData().toString();
    QString number = indexNumberEdit_->text().trimmed();

    // 将数字格式化为3位数，前面补零
    if (!number.isEmpty()) {
        int num = number.toInt();
        number = QString("%1").arg(num, 3, 10, QChar('0'));
    }

    b.indexId = prefix + number;
    b.name = nameEdit_->text().trimmed();
    b.author = authorEdit_->text().trimmed();
    b.publisher = publisherEdit_->text().trimmed();
    b.location = locationEdit_->currentText();
    b.category = categoryEdit_->currentText().trimmed();
    b.price = priceEdit_->text().toDouble();
    b.inDate = inDateEdit_->date();
    b.description = descriptionEdit_->toPlainText().trimmed();
    return b;
}

void BookDialog::onIndexPrefixChanged()
{
    // 当索引号前缀改变时，可以在这里添加相关逻辑
    // 例如：更新数字输入框的提示文本，或者根据前缀自动建议类别等
    QString prefix = indexPrefixCombo_->currentData().toString();

    // 可以根据前缀自动设置类别
    if (prefix == "CS") {
        categoryEdit_->setCurrentText("计算机科学");
    } else if (prefix == "LIT") {
        categoryEdit_->setCurrentText("文学");
    } else if (prefix == "HIS") {
        categoryEdit_->setCurrentText("历史");
    } else if (prefix == "SCI") {
        categoryEdit_->setCurrentText("科学");
    } else if (prefix == "ENG") {
        categoryEdit_->setCurrentText("外语");
    } else if (prefix == "ART") {
        categoryEdit_->setCurrentText("艺术");
    } else if (prefix == "PHI") {
        categoryEdit_->setCurrentText("哲学");
    }
}

