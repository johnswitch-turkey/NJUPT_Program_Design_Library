#ifndef LOG_H
#define LOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>

class Log : public QDialog
{
    Q_OBJECT

public:
    explicit Log(QWidget *parent = nullptr);
    ~Log();

    QString getUsername() const;
    QString getPassword() const;

private slots:
    void onLogin();
    void onCancel();

private:
    void setupUI();
    void setupAnimations();
    void setupStyles();
    void paintEvent(QPaintEvent *event) override;
    void showEvent(QShowEvent *event) override;

    bool validateCredentials(const QString &username, const QString &password);

    // UI 成员
    QLabel *titleLabel;
    QLabel *subtitleLabel;
    QLineEdit *usernameEdit;
    QLineEdit *passwordEdit;
    QPushButton *loginButton;
    QPushButton *cancelButton;
    QGraphicsOpacityEffect *opacityEffect;
    QPropertyAnimation *fadeInAnimation;

    // 登录结果
    QString currentUsername;
    QString currentPassword;
};

#endif // LOG_H
