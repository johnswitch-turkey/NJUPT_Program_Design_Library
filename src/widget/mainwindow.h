#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>
#include <QTableView>
#include <QLineEdit>
#include <QPushButton>
#include <QDockWidget>
#include <QMenuBar>
#include <QMenu>
#include <QAction>

// 前向声明，避免在头文件中包含Qt的头文件
class QToolBar;
class QScrollArea;
class QStatusBar;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // UI交互相关的槽函数
    void toggleTheme();
    void onSearch();

    // --- 以下是缺失的函数声明 ---
    void onAdd();
    void onEdit();
    void onRemove();
    void onBorrow();
    void onReturn();
    void onShowDue();
    void onSortByBorrow();
    void onOpen();
    void onSave();
    void onShowAll();
    void onSwitchMode();
    void onFilterByCategory();
    void onFilterByLocation();
    void onShowAvailable();
    void onShowBorrowed();
    void onShowTopBorrowed();
    void onShowRecentlyAdded();
    void onShowExpensiveBooks();
    void onShowCheapBooks();
    void onShowStatistics();
    void onSortByName();
    void onSortByCategory();
    void onSortByLocation();
    void onSortByPrice();
    void onSortByDate();
    void onSortByBorrowCount();
    void onAdvancedSearch();
    void onExportData();
    void onImportData();
    void onBackupData();
    void onRestoreData();


private:
    // UI设置函数
    void setupTable();
    void setupMenuBar();
    void setupSearchBar();
    void setupActions();
    void setupThemeToggle();
    void setupStyles();
    void applyTheme(bool isDark);
    QString getThemeStyles(bool isDark);
    QDockWidget* createDockWidgetFromScrollArea(QScrollArea *scrollArea);

    // UI更新函数
    void updateUIForUserMode();
    void updateStatusBar();

    // UI成员变量
    Ui::MainWindow *ui;
    QStandardItemModel *model_;
    QTableView *tableView_;
    QLineEdit *searchEdit_;
    QPushButton *searchButton_;
    QPushButton *themeToggleButton_;

    // 菜单和工具栏
    QMenuBar* menuBar_;
    QMenu* bookMenu_;
    QMenu* queryMenu_;
    QMenu* sortMenu_;
    QMenu* dataMenu_;
    QMenu* systemMenu_;

    // 状态和主题
    bool isDarkMode_;
};
#endif // MAINWINDOW_H
