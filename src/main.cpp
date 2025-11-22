#include <QApplication>
#include <QMessageBox>
#include <QDialog> // 包含 QDialog 头文件
#include <QTimer>   // 用于延迟关闭登录窗口
#include <QIcon>    // 用于设置应用图标

// 包含你的头文件
#include "./widget/mainwindow.h"
#include "./utils/log.h" // <--- 1. 包含登录对话框头文件

/**
 * @brief 主函数
 * 
 * 功能说明：
 * 1. 创建并显示登录/注册对话框
 * 2. 等待用户登录成功
 * 3. 登录成功后显示主窗口
 * 
 * 注意：
 * - 对话框是模态的，会阻塞直到用户登录成功或取消
 * - 只有登录成功（返回 Accepted）才会显示主窗口
 * - 用户注册时对话框不会关闭，只是切换界面
 */
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 设置应用图标（使用ICO格式用于Windows任务栏和标题栏）
    a.setWindowIcon(QIcon(":/library.ico"));

    // --- 创建并显示登录对话框 ---
    Log log;

    // 使用 exec() 显示模态对话框，这会阻塞代码直到对话框关闭
    // 返回值说明：
    // - QDialog::Accepted: 用户登录成功，继续执行显示主窗口
    // - QDialog::Rejected: 用户点击取消或关闭窗口，退出程序
    //
    // 重要：当用户点击"点击注册"按钮时，对话框不会关闭，
    // 只是切换界面，exec() 会继续阻塞等待用户操作
    int result = log.exec();

    // 检查对话框的返回值
    if (result != QDialog::Accepted) {
        // 如果用户取消登录或关闭窗口，直接退出程序
        // 注意：这不会在用户注册时触发，因为注册时对话框不会关闭
        return 0;
    }

    // --- 登录成功，创建并显示主窗口 ---
    MainWindow w;
    // 将当前用户信息传递给主窗口（用户名 + 是否管理员 + 用户数据文件路径）
    w.setCurrentUser(log.getUsername(), log.isAdmin(), log.getUsersFilePath());

    w.show();

    // 进入主事件循环，显示主窗口
    return QApplication::exec();
}
