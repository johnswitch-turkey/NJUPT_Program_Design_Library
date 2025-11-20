#ifndef BOOKDETAILDIALOG_H
#define BOOKDETAILDIALOG_H

#include <QDialog>
#include <QJsonObject>
#include "../utils/book.h"

class QLabel;
class QPushButton;
class QVBoxLayout;
class QHBoxLayout;
class QScrollArea;
class QGroupBox;

class BookDetailDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BookDetailDialog(const Book &book, QWidget *parent = nullptr);

private:
    void setupUI();
    void setupBookInfo();
    void setupCopiesInfo();

    Book book_;

    // UI components
    QScrollArea *scrollArea_;
    QWidget *contentWidget_;
    QVBoxLayout *mainLayout_;

    // Book info group
    QGroupBox *bookInfoGroup_;
    QVBoxLayout *bookInfoLayout_;
    QLabel *indexIdLabel_;
    QLabel *nameLabel_;
    QLabel *authorLabel_;
    QLabel *publisherLabel_;
    QLabel *locationLabel_;
    QLabel *categoryLabel_;
    QLabel *priceLabel_;
    QLabel *inDateLabel_;
    QLabel *borrowCountLabel_;

    // Copies info group
    QGroupBox *copiesGroup_;
    QVBoxLayout *copiesLayout_;

    // Buttons
    QPushButton *closeButton_;
};

#endif // BOOKDETAILDIALOG_H