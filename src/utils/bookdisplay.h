// bookdisplay.h
#ifndef BOOKDISPLAY_H
#define BOOKDISPLAY_H

#include <QDialog>
#include <QLineEdit>
#include <QDateEdit>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QComboBox>
#include "book.h"

class BookDialog : public QDialog {
    Q_OBJECT
public:
    explicit BookDialog(QWidget *parent = nullptr);

    void setBook(const Book &book);
    Book getBook() const;

private:
    QLineEdit *indexIdEdit_ = nullptr;
    QLineEdit *nameEdit_ = nullptr;
    QLineEdit *authorEdit_ = nullptr;
    QLineEdit *publisherEdit_ = nullptr;
    QComboBox *locationEdit_ = nullptr;
    QComboBox *categoryEdit_ = nullptr;
    QLineEdit *priceEdit_ = nullptr;
    QDateEdit *inDateEdit_ = nullptr;
};

#endif //BOOKDISPLAY_H

