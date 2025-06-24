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
    void on_btnCreateConfirm_clicked();
    void on_btnEditConfirm_clicked();
    void on_btnDeleteConfirm_clicked();

private:
    Ui::usermanagewin *ui;
    QSqlDatabase db;
    void refreshTable(const QString &usernameFilter = "");
};

#endif // USERMANAGEWIN_H
