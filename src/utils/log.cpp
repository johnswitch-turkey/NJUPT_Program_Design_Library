#include "log.h"
#include <QApplication>
#include <QScreen>
#include <QPainter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <QGraphicsDropShadowEffect>

Log::Log(QWidget *parent)
    : QDialog(parent)
{
    setupUI();
    setupAnimations();
    setupStyles();
}

Log::~Log()
{
}

void Log::setupUI()
{
    // 设置窗口属性
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(400, 450); // 高度可以稍微调小一点
    setModal(true);

    // 居中显示
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);

    // 创建主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    mainLayout->setSpacing(20);

    // 标题标签
    titleLabel = new QLabel("图书管理系统", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(
        "QLabel {"
        "    color: #1C1C1E;"
        "    font-size: 24px;"
        "    font-weight: 600;"
        "    background: transparent;"
        "    font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;"
        "}"
    );

    // 副标题标签
    subtitleLabel = new QLabel("请登录以继续", this);
    subtitleLabel->setAlignment(Qt::AlignCenter);
    subtitleLabel->setStyleSheet(
        "QLabel {"
        "    color: #8E8E93;"
        "    font-size: 16px;"
        "    font-weight: 400;"
        "    background: transparent;"
        "    font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;"
        "}"
    );

    // 创建表单布局
    QFormLayout *formLayout = new QFormLayout();
    formLayout->setSpacing(15);

    // 用户名输入框
    usernameEdit = new QLineEdit(this);
    usernameEdit->setPlaceholderText("请输入用户名");
    usernameEdit->setStyleSheet(
        "QLineEdit {"
        "    background-color: #F2F2F7;"
        "    border: 2px solid #E5E5EA;"
        "    border-radius: 12px;"
        "    padding: 12px 16px;"
        "    font-size: 16px;"
        "    color: #1C1C1E;"
        "    font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;"
        "}"
        "QLineEdit:focus {"
        "    border-color: #007AFF;"
        "    background-color: #FFFFFF;"
        "}"
    );

    // 密码输入框
    passwordEdit = new QLineEdit(this);
    passwordEdit->setPlaceholderText("请输入密码");
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setStyleSheet(
        "QLineEdit {"
        "    background-color: #F2F2F7;"
        "    border: 2px solid #E5E5EA;"
        "    border-radius: 12px;"
        "    padding: 12px 16px;"
        "    font-size: 16px;"
        "    color: #1C1C1E;"
        "    font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;"
        "}"
        "QLineEdit:focus {"
        "    border-color: #007AFF;"
        "    background-color: #FFFFFF;"
        "}"
    );

    // 添加标签和输入框到表单
    formLayout->addRow("👤 用户名:", usernameEdit);
    formLayout->addRow("🔒 密码:", passwordEdit);

    // 按钮布局
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(12);

    // 登录按钮
    loginButton = new QPushButton("登录", this);
    loginButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #007AFF;"
        "    color: #FFFFFF;"
        "    border: none;"
        "    border-radius: 12px;"
        "    padding: 12px 24px;"
        "    font-size: 16px;"
        "    font-weight: 600;"
        "    font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;"
        "    min-width: 100px;"
        "    min-height: 44px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #0051D5;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #0041B8;"
        "}"
    );

    // 取消按钮
    cancelButton = new QPushButton("取消", this);
    cancelButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #F2F2F7;"
        "    color: #1C1C1E;"
        "    border: 2px solid #E5E5EA;"
        "    border-radius: 12px;"
        "    padding: 12px 24px;"
        "    font-size: 16px;"
        "    font-weight: 500;"
        "    font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;"
        "    min-width: 100px;"
        "    min-height: 44px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #E5E5EA;"
        "    border-color: #C7C7CC;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #D1D1D6;"
        "}"
    );

    buttonLayout->addWidget(loginButton);
    buttonLayout->addWidget(cancelButton);

    // 添加到主布局
    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(subtitleLabel);
    mainLayout->addLayout(formLayout);
    mainLayout->addLayout(buttonLayout);

    // 设置透明度效果
    opacityEffect = new QGraphicsOpacityEffect(this);
    setGraphicsEffect(opacityEffect);
    opacityEffect->setOpacity(1.0);

    // 添加阴影效果
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(20);
    shadow->setColor(QColor(0, 0, 0, 30));
    shadow->setOffset(0, 4);
    // 注意：需要将阴影效果应用到某个子控件上，这里为了简化，先注释掉
    // titleLabel->setGraphicsEffect(shadow);

    // 连接信号
    connect(loginButton, &QPushButton::clicked, this, &Log::onLogin);
    connect(cancelButton, &QPushButton::clicked, this, &Log::onCancel);

    // 设置回车键登录
    connect(usernameEdit, &QLineEdit::returnPressed, this, &Log::onLogin);
    connect(passwordEdit, &QLineEdit::returnPressed, this, &Log::onLogin);
}

void Log::setupAnimations()
{
    fadeInAnimation = new QPropertyAnimation(opacityEffect, "opacity", this);
    fadeInAnimation->setDuration(300);
    fadeInAnimation->setStartValue(0.0);
    fadeInAnimation->setEndValue(1.0);
    fadeInAnimation->setEasingCurve(QEasingCurve::OutQuad);
}

void Log::setupStyles()
{
    setStyleSheet("QDialog { background-color: transparent; }");
}

void Log::paintEvent(QPaintEvent * /* event */)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // iOS风格背景
    QRect rect = this->rect().adjusted(10, 10, -10, -10);
    painter.setBrush(QBrush(QColor(255, 255, 255, 250)));
    painter.setPen(QPen(QColor(200, 200, 200, 100), 1));
    painter.drawRoundedRect(rect, 20, 20);
}

void Log::onLogin()
{
    QString username = usernameEdit->text().trimmed();
    QString password = passwordEdit->text().trimmed();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请输入用户名和密码");
        return;
    }

    // 普通用户验证
    if (!validateCredentials(username, password)) {
        QMessageBox::warning(this, "登录失败", "用户名或密码错误");
        return;
    }

    currentUsername = username;
    currentPassword = password;
    accept();
}

void Log::onCancel()
{
    reject();
}

QString Log::getUsername() const
{
    return currentUsername;
}

QString Log::getPassword() const
{
    return currentPassword;
}

bool Log::validateCredentials(const QString &username, const QString &password)
{
    // 简化的用户验证，实际项目中应该连接数据库
    // 这里允许任何非空用户名和密码登录
    return !username.isEmpty() && !password.isEmpty();
}

void Log::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    raise();
    activateWindow();
}
