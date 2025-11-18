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
    , adminCheckBox_(nullptr)
    , isRegisterMode_(false)
    , isAdminMode_(false)
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

    // 加载用户数据（如果不存在则自动创建并写入默认账号）
    loadUsers();

    // 确保预置管理员和学生账号存在
    // 管理员：B24010616 / B24010608，密码 123
    auto ensureUser = [this](const QString &username,
                             const QString &password,
                             const QString &role,
                             const QStringList &allowedCategories = {}) {
        // 检查是否已经存在
        for (const QJsonValue &value : std::as_const(usersArray_)) {
            if (!value.isObject()) continue;
            QJsonObject obj = value.toObject();
            if (obj.value("username").toString() == username) {
                // 已存在则只补充缺失字段
                if (!obj.contains("role")) obj["role"] = role;
                if (!obj.contains("allowedCategories")) {
                    QJsonArray cats;
                    for (const auto &c : allowedCategories) {
                        cats.append(c);
                    }
                    obj["allowedCategories"] = cats;
                }
                // 确保密码为指定值（方便调试）
                obj["password"] = password;

                // 写回数组
                for (int i = 0; i < usersArray_.size(); ++i) {
                    QJsonObject existing = usersArray_.at(i).toObject();
                    if (existing.value("username").toString() == username) {
                        usersArray_[i] = obj;
                        break;
                    }
                }
                return;
            }
        }

        // 不存在则追加新用户
        QJsonObject newUser;
        newUser["username"] = username;
        newUser["password"] = password;
        newUser["role"] = role;
        QJsonArray cats;
        for (const auto &c : allowedCategories) {
            cats.append(c);
        }
        newUser["allowedCategories"] = cats;
        newUser["borrows"] = QJsonArray(); // 预留借阅信息
        usersArray_.append(newUser);
    };

    // 两个管理员
    ensureUser(QStringLiteral("B24010616"), QStringLiteral("123"), QStringLiteral("admin"));
    ensureUser(QStringLiteral("B24010608"), QStringLiteral("123"), QStringLiteral("admin"));
    // 两个虚拟学生账号，密码 123，给不同的可借类别
    ensureUser(QStringLiteral("S24010001"),
               QStringLiteral("123"),
               QStringLiteral("student"),
               QStringList{QStringLiteral("计算机科学"), QStringLiteral("外语")});
    ensureUser(QStringLiteral("S24010002"),
               QStringLiteral("123"),
               QStringLiteral("student"),
               QStringList{QStringLiteral("文学"), QStringLiteral("历史")});

    // 保存可能更新后的用户数据
    saveUsers();

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

    // 为整个对话框设置一个现代化的背景色
    this->setStyleSheet("Log { background-color: #f0f2f5; }");

    // 标题
    titleLabel_ = new QLabel("登录", this);
    titleLabel_->setAlignment(Qt::AlignCenter);
    QFont titleFont = titleLabel_->font();
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    titleLabel_->setFont(titleFont);
    // 设置标题样式
    titleLabel_->setStyleSheet("color: #2c3e50; margin-top: 10px; margin-bottom: 20px;");
    mainLayout_->addWidget(titleLabel_);

    // 用户名输入框
    usernameEdit_ = new QLineEdit(this);
    usernameEdit_->setPlaceholderText("请输入用户名");
    // 设置输入框样式
    usernameEdit_->setStyleSheet(
        "QLineEdit {"
        "   border: 1px solid #cccccc;"
        "   border-radius: 8px;"
        "   padding: 10px;"
        "   font-size: 14px;"
        "   background-color: white;"
        "}"
        "QLineEdit:focus {"
        "   border: 2px solid #3498db;"
        "}"
    );
    mainLayout_->addWidget(usernameEdit_);

    // 密码输入框
    passwordEdit_ = new QLineEdit(this);
    passwordEdit_->setPlaceholderText("请输入密码");
    passwordEdit_->setEchoMode(QLineEdit::Password);
    // 设置密码框样式（与用户名框保持一致）
    passwordEdit_->setStyleSheet(
        "QLineEdit {"
        "   border: 1px solid #cccccc;"
        "   border-radius: 8px;"
        "   padding: 10px;"
        "   font-size: 14px;"
        "   background-color: white;"
        "}"
        "QLineEdit:focus {"
        "   border: 2px solid #3498db;"
        "}"
    );
    mainLayout_->addWidget(passwordEdit_);

    // 管理员模式勾选框
    adminCheckBox_ = new QCheckBox(QStringLiteral("以管理员模式登录"), this);
    adminCheckBox_->setToolTip(QStringLiteral("勾选后，将尝试以管理员身份登录（仅限管理员账号）"));
    mainLayout_->addWidget(adminCheckBox_);

    // 按钮布局
    QHBoxLayout *buttonLayout = new QHBoxLayout();

    // 登录/注册按钮
    actionButton_ = new QPushButton("登录", this);
    actionButton_->setDefault(true);
    // 设置主按钮样式
    actionButton_->setStyleSheet(
        "QPushButton {"
        "   background-color: #3498db;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 8px;"
        "   padding: 10px;"
        "   font-size: 14px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: #2980b9;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #21618c;"
        "}"
    );
    connect(actionButton_, &QPushButton::clicked, this, &Log::performLogin);
    buttonLayout->addWidget(actionButton_);

    // 取消按钮
    cancelButton_ = new QPushButton("取消", this);
    // 设置取消按钮样式
    cancelButton_->setStyleSheet(
        "QPushButton {"
        "   background-color: transparent;"
        "   color: #7f8c8d;"
        "   border: 1px solid #bdc3c7;"
        "   border-radius: 8px;"
        "   padding: 10px;"
        "   font-size: 14px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #ecf0f1;"
        "   border-color: #95a5a6;"
        "}"
    );
    connect(cancelButton_, &QPushButton::clicked, this, &QDialog::reject);
    buttonLayout->addWidget(cancelButton_);

    mainLayout_->addLayout(buttonLayout);

    // 切换按钮（切换到注册/登录）
    switchButton_ = new QPushButton("点击注册", this);
    switchButton_->setFlat(true);
    // 设置切换按钮样式（像超链接）
    switchButton_->setStyleSheet(
        "QPushButton {"
        "   color: #3498db;"
        "   text-decoration: underline;"
        "   padding: 5px;"
        "}"
        "QPushButton:hover {"
        "   color: #2980b9;"
        "}"
    );
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
    if (!validateUser(username, password)) {
        QMessageBox::warning(this, "登录失败", "用户名或密码错误！");
        passwordEdit_->clear();
        passwordEdit_->setFocus();
        return;
    }

    // 读取用户角色
    QString role = QStringLiteral("student");
    for (const QJsonValue &value : std::as_const(usersArray_)) {
        if (!value.isObject()) continue;
        QJsonObject obj = value.toObject();
        if (obj.value("username").toString() == username) {
            role = obj.value("role").toString(QStringLiteral("student"));
            break;
        }
    }

    // 根据勾选情况决定是否以管理员模式登录
    if (adminCheckBox_ && adminCheckBox_->isChecked()) {
        if (role != QStringLiteral("admin")) {
            QMessageBox::warning(this, "登录失败", "该账号不是管理员，不能以管理员模式登录！");
            return;
        }
        isAdminMode_ = true;
    } else {
        isAdminMode_ = false;
    }

    currentUsername_ = username;
    QString welcome = isAdminMode_
                      ? QStringLiteral("欢迎管理员 %1！").arg(username)
                      : QStringLiteral("欢迎，%1！").arg(username);
    QMessageBox::information(this, "登录成功", welcome);
    accept(); // 返回Accepted，允许主程序继续执行
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

    // 添加新用户，默认学生角色，初始可借所有类别
    QJsonObject newUser;
    newUser["username"] = username;
    newUser["password"] = password; // 注意：实际应用中应该对密码进行加密
    newUser["role"] = QStringLiteral("student");
    newUser["allowedCategories"] = QJsonArray(); // 空表示不限制
    newUser["borrows"] = QJsonArray();
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


