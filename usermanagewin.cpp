#include "usermanagewin.h"
#include "ui_usermanagewin.h"

usermanagewin::usermanagewin(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::usermanagewin)
{
    ui->setupUi(this);
}

usermanagewin::~usermanagewin()
{
    delete ui;
}
