#ifndef USERMANAGEWIN_H
#define USERMANAGEWIN_H

#include <QMainWindow>

namespace Ui {
class usermanagewin;
}

class usermanagewin : public QMainWindow
{
    Q_OBJECT

public:
    explicit usermanagewin(QWidget *parent = nullptr);
    ~usermanagewin();

private:
    Ui::usermanagewin *ui;
};

#endif // USERMANAGEWIN_H
