#ifndef SIGNUPDIALOG_H
#define SIGNUPDIALOG_H

#include <QDialog>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QMessageBox>
#include "Log.h"

namespace Ui
{
    class signupdialog;
}

class signupdialog : public QDialog
{
    Q_OBJECT

public:
    explicit signupdialog(QWidget *parent = nullptr);
    ~signupdialog();

private slots:
    void onSignUpClicked();

private:
    Ui::signupdialog *ui;
    QSqlDatabase db;
};

#endif // SIGNUPDIALOG_H
