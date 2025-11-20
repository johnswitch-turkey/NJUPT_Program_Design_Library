// borrowdialog.h
#ifndef BORROWDIALOG_H
#define BORROWDIALOG_H

#include <QDialog>
#include <QDate>
#include <QVector>
#include "../utils/book.h"
#include "../utils/bookcopy.h"

class QVBoxLayout;
class QHBoxLayout;
class QLabel;
class QComboBox;
class QDateEdit;
class QPushButton;
class QLineEdit;

class BorrowDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BorrowDialog(const Book &book, const QVector<BookCopy> &availableCopies, QWidget *parent = nullptr);
    ~BorrowDialog();

    BookCopy getSelectedCopy() const;
    QDate getDueDate() const;

private slots:
    void onOkClicked();
    void onCancelClicked();
    void validateInput();

private:
    void setupUI();
    void updateCopyInfo();

    Book book_;
    QVector<BookCopy> availableCopies_;
    BookCopy selectedCopy_;
    QDate dueDate_;

    // UI组件
    QLabel *bookInfoLabel_;
    QComboBox *copyComboBox_;
    QDateEdit *dueDateEdit_;
    QPushButton *okButton_;
    QPushButton *cancelButton_;
    QLabel *statusLabel_;
};

#endif // BORROWDIALOG_H