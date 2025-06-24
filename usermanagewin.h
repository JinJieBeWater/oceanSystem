#ifndef USERMANAGEWIN_H
#define USERMANAGEWIN_H

#include <QMainWindow>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QTableWidgetItem>

namespace Ui
{
    class usermanagewin;
}

class usermanagewin : public QMainWindow
{
    Q_OBJECT

public:
    explicit usermanagewin(QWidget *parent = nullptr);
    ~usermanagewin();

private slots:
    void on_btnSearchUserName_clicked();

private:
    Ui::usermanagewin *ui;
    QSqlDatabase db;
    void refreshTable(const QString &usernameFilter = "");
};

#endif // USERMANAGEWIN_H
