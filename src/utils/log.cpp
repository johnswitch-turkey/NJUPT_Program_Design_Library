#include "log.h"
#include <QApplication>
#include <QStandardPaths>

Log::Log(QWidget *parent)
    : QDialog(parent)
    , mainLayout_(nullptr)
    , titleLabel_(nullptr)
    , usernameEdit_(nullptr)
    , passwordEdit_(nullptr)
    , actionButton_(nullptr)
    , switchButton_(nullptr)
    , cancelButton_(nullptr)
    , isRegisterMode_(false)
{
    // 设置用户数据文件路径（存储在应用程序数据目录）
    // 1. 获取可执行文件所在的目录
    QString appDirPath = QCoreApplication::applicationDirPath();

    // 2. 构建目标目录路径 (上一级目录的src/source)
    QDir targetDir(appDirPath);
    bool success = targetDir.cdUp(); // 先进入上一级目录
    if (success) {
        targetDir.cd("src");        // 进入src目录
        targetDir.cd("resource");     // 进入source目录
    } else {
        // 如果cdUp()失败（例如，在根目录），则回退到可执行文件目录
        targetDir = QDir(appDirPath);
    }

    // 3. 确保目标目录存在，如果不存在则创建
    QString absoluteTargetPath = targetDir.absolutePath();
    if (!targetDir.exists()) {
        targetDir.mkpath(absoluteTargetPath);
    }

    // 4. 组合成最终的文件完整路径
    usersFilePath_ = absoluteTargetPath + "/users.json";

    // 加载用户数据
    loadUsers();

    // 设置UI
    setupUI();

    // 设置窗口属性
    setWindowTitle("登录");
    setModal(true);
    setFixedSize(350, 250);
}

Log::~Log()
{
}

void Log::setupUI()
{
    mainLayout_ = new QVBoxLayout(this);

    // 标题
    titleLabel_ = new QLabel("登录", this);
    titleLabel_->setAlignment(Qt::AlignCenter);
    QFont titleFont = titleLabel_->font();
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    titleLabel_->setFont(titleFont);
    mainLayout_->addWidget(titleLabel_);

    // 用户名输入框
    usernameEdit_ = new QLineEdit(this);
    usernameEdit_->setPlaceholderText("请输入用户名");
    mainLayout_->addWidget(usernameEdit_);

    // 密码输入框
    passwordEdit_ = new QLineEdit(this);
    passwordEdit_->setPlaceholderText("请输入密码");
    passwordEdit_->setEchoMode(QLineEdit::Password);
    mainLayout_->addWidget(passwordEdit_);

    // 按钮布局
    QHBoxLayout *buttonLayout = new QHBoxLayout();

    // 登录/注册按钮
    actionButton_ = new QPushButton("登录", this);
    actionButton_->setDefault(true);
    connect(actionButton_, &QPushButton::clicked, this, &Log::performLogin);
    buttonLayout->addWidget(actionButton_);

    // 取消按钮
    cancelButton_ = new QPushButton("取消", this);
    connect(cancelButton_, &QPushButton::clicked, this, &QDialog::reject);
    buttonLayout->addWidget(cancelButton_);

    mainLayout_->addLayout(buttonLayout);

    // 切换按钮（切换到注册/登录）
    switchButton_ = new QPushButton("点击注册", this);
    switchButton_->setFlat(true);
    connect(switchButton_, &QPushButton::clicked, this, &Log::switchToRegister);
    mainLayout_->addWidget(switchButton_);

    setLayout(mainLayout_);
}

void Log::switchToRegister()
{
    isRegisterMode_ = true;
    titleLabel_->setText("注册");
    actionButton_->setText("注册");
    switchButton_->setText("返回登录");
    usernameEdit_->clear();
    passwordEdit_->clear();

    // 断开之前的连接，连接新的槽函数
    actionButton_->disconnect();
    connect(actionButton_, &QPushButton::clicked, this, &Log::performRegister);
    switchButton_->disconnect();
    connect(switchButton_, &QPushButton::clicked, this, &Log::switchToLogin);
}

void Log::switchToLogin()
{
    isRegisterMode_ = false;
    titleLabel_->setText("登录");
    actionButton_->setText("登录");
    switchButton_->setText("点击注册");
    usernameEdit_->clear();
    passwordEdit_->clear();

    // 断开之前的连接，连接新的槽函数
    actionButton_->disconnect();
    connect(actionButton_, &QPushButton::clicked, this, &Log::performLogin);
    switchButton_->disconnect();
    connect(switchButton_, &QPushButton::clicked, this, &Log::switchToRegister);
}

void Log::performLogin()
{
    QString username = usernameEdit_->text().trimmed();
    QString password = passwordEdit_->text();

    // 验证输入
    if (username.isEmpty()) {
        QMessageBox::warning(this, "登录失败", "请输入用户名！");
        usernameEdit_->setFocus();
        return;
    }

    if (password.isEmpty()) {
        QMessageBox::warning(this, "登录失败", "请输入密码！");
        passwordEdit_->setFocus();
        return;
    }

    // 验证用户名和密码
    if (validateUser(username, password)) {
        currentUsername_ = username;
        QMessageBox::information(this, "登录成功", QString("欢迎，%1！").arg(username));
        accept(); // 返回Accepted，允许主程序继续执行
    } else {
        QMessageBox::warning(this, "登录失败", "用户名或密码错误！");
        passwordEdit_->clear();
        passwordEdit_->setFocus();
    }
}

void Log::performRegister()
{
    QString username = usernameEdit_->text().trimmed();
    QString password = passwordEdit_->text();

    // 验证输入
    if (username.isEmpty()) {
        QMessageBox::warning(this, "注册失败", "请输入用户名！");
        usernameEdit_->setFocus();
        return;
    }

    if (password.isEmpty()) {
        QMessageBox::warning(this, "注册失败", "请输入密码！");
        passwordEdit_->setFocus();
        return;
    }

    if (password.length() < 3) {
        QMessageBox::warning(this, "注册失败", "密码长度至少为3个字符！");
        passwordEdit_->clear();
        passwordEdit_->setFocus();
        return;
    }

    // 检查用户名是否已存在
    if (userExists(username)) {
        QMessageBox::warning(this, "注册失败", "该用户名已存在，请选择其他用户名！");
        usernameEdit_->clear();
        usernameEdit_->setFocus();
        return;
    }

    // 添加新用户
    QJsonObject newUser;
    newUser["username"] = username;
    newUser["password"] = password; // 注意：实际应用中应该对密码进行加密
    usersArray_.append(newUser);

    // 保存用户数据
    if (saveUsers()) {
        QMessageBox::information(this, "注册成功", "账户注册成功！请登录。");
        // 切换到登录界面
        switchToLogin();
        usernameEdit_->setText(username); // 保留用户名，方便用户直接输入密码
        passwordEdit_->setFocus();
    } else {
        QMessageBox::critical(this, "注册失败", "保存用户数据失败！");
        // 移除刚才添加的用户
        usersArray_.removeAt(usersArray_.size() - 1);
    }
}

bool Log::loadUsers()
{
    QFile file(usersFilePath_);
    if (!file.exists()) {
        // 文件不存在，创建空的用户数组
        usersArray_ = QJsonArray();
        return true;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(nullptr, "警告", QString("无法读取用户数据文件：%1").arg(file.errorString()));
        usersArray_ = QJsonArray();
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || !doc.isArray()) {
        QMessageBox::warning(nullptr, "警告", "用户数据文件格式错误，将创建新的数据文件。");
        usersArray_ = QJsonArray();
        return false;
    }

    usersArray_ = doc.array();
    return true;
}

bool Log::saveUsers()
{
    QFile file(usersFilePath_);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::critical(nullptr, "错误", QString("无法保存用户数据文件：%1").arg(file.errorString()));
        return false;
    }

    QJsonDocument doc(usersArray_);
    file.write(doc.toJson());
    file.close();
    return true;
}

bool Log::userExists(const QString &username)
{
    for (const QJsonValue &value : usersArray_) {
        if (value.isObject()) {
            QJsonObject user = value.toObject();
            if (user.value("username").toString() == username) {
                return true;
            }
        }
    }
    return false;
}

bool Log::validateUser(const QString &username, const QString &password)
{
    for (const QJsonValue &value : usersArray_) {
        if (value.isObject()) {
            QJsonObject user = value.toObject();
            if (user.value("username").toString() == username &&
                user.value("password").toString() == password) {
                return true;
            }
        }
    }
    return false;
}

