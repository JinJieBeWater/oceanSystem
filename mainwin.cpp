#include "mainwin.h"
#include "ui_mainwin.h"

mainwin::mainwin(QWidget *parent) : QMainWindow(parent),
                                    ui(new Ui::mainwin)
{
    ui->setupUi(this);
    userManageWin = new usermanagewin(this);
}

mainwin::~mainwin()
{
    delete ui;
}

void mainwin::on_userManageNav_clicked()
{
    userManageWin->show();
    this->hide(); // 隐藏当前窗口
}
