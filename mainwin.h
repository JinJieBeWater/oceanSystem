#ifndef MAINWIN_H
#define MAINWIN_H

#include <QMainWindow>

#include "usermanagewin.h"

namespace Ui
{
    class mainwin;
}

class mainwin : public QMainWindow
{
    Q_OBJECT

public:
    explicit mainwin(QWidget *parent = nullptr);
    ~mainwin();

private slots:
    void on_userManageNav_clicked();

private:
    Ui::mainwin *ui;

    usermanagewin *userManageWin; // 用户管理窗口
};

#endif // MAINWIN_H
