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

    // 设置当前登录用户及模式（学生/管理员），以及用户数据文件路径
    void setCurrentUser(const QString &username, bool isAdminMode, const QString &usersFilePath);

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
    void onShowMyBorrows();              // 学生查看自己的借阅信息
    void onShowBookBorrowHistory();      // 管理员查看某本书的借阅记录

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

    // 用户与借阅相关的辅助函数
    QJsonArray loadUsersJson() const;
    bool saveUsersJson(const QJsonArray &array) const;
    QStringList getCurrentUserAllowedCategories() const;
    bool currentUserHasBorrowed(const QString &indexId) const;
    void addBorrowRecordForCurrentUser(const Book &book, const QDate &borrowDate, const QDate &dueDate);
    void markBorrowRecordReturnedForCurrentUser(const QString &indexId, const QDate &returnDate);
    QString borrowRecordsForCurrentUserText() const;
    QString borrowHistoryForBookText(const QString &indexId) const;

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

    // 当前登录用户信息
    QString currentUsername_;
    bool isAdminMode_ = false;          // 是否以管理员模式登录
    QString usersFilePath_;             // 用户数据文件路径
    QStringList allowedCategories_;     // 当前学生可借类别，空表示不限制


};

#endif // MAINWINDOW_H
