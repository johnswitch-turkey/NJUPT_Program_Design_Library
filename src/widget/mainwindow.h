#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>
#include <QTableView>
#include <QPushButton> // <--- 添加这个头文件
#include "../utils/librarymanager.h" // 核心数据管理器

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // UI交互
    void toggleTheme();
    void onSearch();

    // 核心业务功能
    void onBorrow();
    void onReturn();
    void onShowAll();
    void onSwitchMode(); // <--- 恢复这个声明

    // 数据管理
    void onOpen();
    void onSave();

private:
    // UI设置
    void setupTable();
    void setupMenuBar();
    void setupSearchBar();
    void setupActions();
    void setupThemeToggle();
    void setupStyles();
    void applyTheme(bool isDark);
    QString getThemeStyles(bool isDark);
    QDockWidget* createDockWidgetFromScrollArea(class QScrollArea *scrollArea);

    // 数据与显示
    void loadSampleData(); // 加载示例数据
    void refreshTable();   // 刷新表格显示

    // UI更新
    void updateStatusBar();

    // UI成员
    Ui::MainWindow *ui;
    LibraryManager library_;       // 核心数据管理器
    QStandardItemModel *model_;     // 表格模型
    QTableView *tableView_;         // 表格视图
    QLineEdit *searchEdit_;
    QPushButton *searchButton_;      // <--- 恢复这个声明
    QPushButton *themeToggleButton_; // <--- 恢复这个声明

    // 状态和主题
    bool isDarkMode_;
};

#endif // MAINWINDOW_H
