// mainwindow.h
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>
#include <QTableView>
#include <QPushButton>
#include <QMenu>
#include <QActionGroup>
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
    void onSwitchMode();
    void onHeaderSectionClicked(int section);

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
    QString getMenuStyles(bool isDark);
    void rebuildFilterMenus();
    void updateHeaderLabels();
    void showFilterMenu(QMenu *menu, int section);
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
    QPushButton *searchButton_;
    QPushButton *themeToggleButton_;
    QMenu *categoryFilterMenu_;
    QMenu *statusFilterMenu_;
    QMenu *locationFilterMenu_;     // 新增：馆藏地址筛选菜单
    QActionGroup *categoryActionGroup_;
    QActionGroup *statusActionGroup_;
    QActionGroup *locationActionGroup_; // 新增：馆藏地址动作组
    QString categoryFilter_;
    QString statusFilter_;
    QString locationFilter_;        // 新增：馆藏地址筛选条件

    // 状态和主题
    bool isDarkMode_;
};

#endif // MAINWINDOW_H
