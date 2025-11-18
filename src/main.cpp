#include <QApplication>
#include <QMessageBox>
#include <QDialog> // 包含 QDialog 头文件
#include <QTimer>   // 用于延迟关闭登录窗口

// 包含你的头文件
#include "./widget/mainwindow.h"
#include "./utils/log.h" // <--- 1. 包含登录对话框头文件

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // --- 2. 创建并显示登录对话框 ---
    Log log;

    // 使用 exec() 显示模态对话框，这会阻塞代码直到对话框关闭
    // 如果用户点击"登录"且验证成功，loginDialog.exec() 会返回 QDialog::Accepted
    // 如果用户点击"取消"或关闭窗口，则返回 QDialog::Rejected
    if (log.exec() != QDialog::Accepted) {
        // 如果登录失败或用户取消，则直接退出程序
        return 0;
    }

    // --- 3. 登录成功，创建并显示主窗口 ---
    MainWindow w;

    // (可选) 你可以从登录对话框获取用户信息，并在主窗口中使用
    // QString username = loginDialog.getUsername();
    // w.setCurrentUser(username); // 假设你在 MainWindow 中有这个方法

    w.show();

    return QApplication::exec();
}
