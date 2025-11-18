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
    void onAddBook();
    void onEditBook();
    void onDeleteBook();

    // 核心业务功能
    void onBorrow();
    void onReturn();
    void onWarn();
    void onShowAll();
    void onSwitchMode();
    void onHeaderSectionClicked(int section);

    // 数据管理
    void onOpen();
    void onSave();
    void onImport();
    void onExport();
    void onRefresh();

    // 筛选菜单
    void onCategoryFilterChanged(QAction* action);
    void onStatusFilterChanged(QAction* action);
    void onLocationFilterChanged(QAction* action);

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
    void loadData(); // 加载数据
    void refreshTable();   // 刷新表格显示

    // --- 新增：筛选并显示特定图书列表的辅助函数 ---
    void displayBooks(const QVector<Book> &booksToShow);

    // UI更新
    void updateStatusBar();
    void showBookDialog(const Book& book = Book(), bool isEdit = false);

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
    bool isEditMode_;

    bool isWarn = false;

};

#endif // MAINWINDOW_H
