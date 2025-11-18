#ifndef LOG_H
#define LOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDir>

/**
 * @brief 登录/注册对话框类
 * 
 * 功能：
 * 1. 提供登录界面，如果用户没有账户，可以切换到注册界面
 * 2. 注册新账户并保存用户名和密码到JSON文件
 * 3. 验证登录时的用户名和密码
 * 4. 登录成功后返回Accepted，允许主程序继续执行
 */
class Log : public QDialog
{
    Q_OBJECT

public:
    explicit Log(QWidget *parent = nullptr);
    ~Log();

    // 获取当前登录的用户名
    QString getUsername() const { return currentUsername_; }

private slots:
    // 切换到注册界面
    void switchToRegister();
    // 切换到登录界面
    void switchToLogin();
    // 执行登录操作
    void performLogin();
    // 执行注册操作
    void performRegister();

private:
    // 初始化UI
    void setupUI();
    // 加载用户数据
    bool loadUsers();
    // 保存用户数据
    bool saveUsers();
    // 检查用户名是否存在
    bool userExists(const QString &username);
    // 验证用户名和密码
    bool validateUser(const QString &username, const QString &password);

    // UI组件
    QVBoxLayout *mainLayout_;
    QLabel *titleLabel_;
    QLineEdit *usernameEdit_;
    QLineEdit *passwordEdit_;
    QPushButton *actionButton_;      // 登录/注册按钮
    QPushButton *switchButton_;      // 切换到注册/登录按钮
    QPushButton *cancelButton_;

    // 数据
    QJsonArray usersArray_;          // 存储所有用户信息
    QString currentUsername_;        // 当前登录的用户名
    QString usersFilePath_;          // 用户数据文件路径

    // 状态
    bool isRegisterMode_;            // 是否为注册模式
};

#endif // LOG_H

