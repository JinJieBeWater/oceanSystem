#include "login.h"
#include "ui_login.h"
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

#include "mainwin.h"
#include "Log.h"

login::login(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::login)
{
    ui->setupUi(this);
    speech = new QTextToSpeech(this);
    mainWin = new mainwin(this);

    // 初始化数据库
    initializeDatabase();
}

login::~login()
{
    // 关闭数据库连接
    if (db.isOpen())
    {
        db.close();
    }
    QSqlDatabase::removeDatabase(QSqlDatabase::defaultConnection);

    delete ui;
}

void login::initializeDatabase()
{
    // 检查是否已有默认连接
    if (QSqlDatabase::contains(QSqlDatabase::defaultConnection))
    {
        db = QSqlDatabase::database(QSqlDatabase::defaultConnection);
    }
    else
    {
        db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName("D:/University/Yue inlay/oceanSystem.db");
    }

    if (!db.open())
    {
        QMessageBox::critical(this, "Database Error",
                              "Failed to open the database!\n" + db.lastError().text());
        Log() << "Database error: " << db.lastError().text();
    }
    else
    {
        Log() << "Database opened successfully.";
        createTables(); // 确保表存在
    }
}

void login::createTables()
{
    QSqlQuery query(db);
    if (!query.exec("CREATE TABLE IF NOT EXISTS users ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "username TEXT UNIQUE NOT NULL, "
                    "password TEXT NOT NULL)"))
    {
        Log() << "Table creation error: " << query.lastError().text();
        QMessageBox::critical(this, "Database Error",
                              "Failed to create tables!\n" + query.lastError().text());
    }
}

void login::on_signInBtn_clicked()
{
    QString username = ui->usernameLineEdit->text().trimmed();
    QString password = ui->passwordLineEdit->text().trimmed();

    if (username.isEmpty() || password.isEmpty())
    {
        QMessageBox::critical(this, "Error", "Username or password cannot be empty!");
        return;
    }

    if (!db.isOpen())
    {
        QMessageBox::critical(this, "Database Error", "Database is not open!");
        return;
    }

    QSqlQuery query(db);
    query.prepare("SELECT id FROM users WHERE username = :username AND password = :password");
    query.bindValue(":username", username);
    query.bindValue(":password", password);

    if (!query.exec())
    {
        Log() << "Login query error: " << query.lastError().text()
              << "\nQuery: " << query.lastQuery()
              << "\nBound values: " << query.boundValues();
        QMessageBox::critical(this, "Database Error",
                              "Query failed!\n" + query.lastError().text());
        return;
    }

    if (query.next())
    {
        Log() << "User" << username << "logged in successfully";
        speech->say("Sign In successful!");
        mainWin->show();
        this->hide();
    }
    else
    {
        Log() << "Failed login attempt for user" << username;
        QMessageBox::critical(this, "Error", "Invalid username or password!");
    }
}

void login::on_signUpBtn_clicked()
{
    QString username = ui->usernameLineEdit->text().trimmed();
    QString password = ui->passwordLineEdit->text().trimmed();

    // 输入验证
    if (username.isEmpty() || password.isEmpty())
    {
        QMessageBox::critical(this, "Error", "Username or password cannot be empty!");
        return;
    }

    if (username.length() < 3 || password.length() < 6)
    {
        QMessageBox::critical(this, "Error",
                              "Username must be at least 3 characters and password at least 6 characters long!");
        return;
    }

    if (username.length() > 20 || password.length() > 20)
    {
        QMessageBox::critical(this, "Error",
                              "Username and password cannot exceed 20 characters!");
        return;
    }

    if (username.contains(" ") || password.contains(" "))
    {
        QMessageBox::critical(this, "Error", "Username and password cannot contain spaces!");
        return;
    }

    if (username.contains(";") || password.contains(";"))
    {
        QMessageBox::critical(this, "Error", "Username and password cannot contain semicolons!");
        return;
    }

    if (!db.isOpen())
    {
        QMessageBox::critical(this, "Database Error", "Database is not open!");
        return;
    }

    // 检查用户名是否已存在
    QSqlQuery checkQuery(db);
    checkQuery.prepare("SELECT id FROM users WHERE username = :username");
    checkQuery.bindValue(":username", username);

    if (!checkQuery.exec())
    {
        Log() << "Username check error: " << checkQuery.lastError().text();
        QMessageBox::critical(this, "Database Error",
                              "Check failed!\n" + checkQuery.lastError().text());
        return;
    }

    if (checkQuery.next())
    {
        QMessageBox::critical(this, "Error", "Username already exists!");
        return;
    }

    // 插入新用户
    QSqlQuery insertQuery(db);
    insertQuery.prepare("INSERT INTO users (username, password) VALUES (:username, :password)");
    insertQuery.bindValue(":username", username);
    insertQuery.bindValue(":password", password);

    if (insertQuery.exec())
    {
        Log() << "User" << username << "registered successfully";
        speech->say("Sign Up successful!");
        mainWin->show();
        this->hide();
    }
    else
    {
        Log() << "Registration error: " << insertQuery.lastError().text()
              << "\nQuery: " << insertQuery.lastQuery()
              << "\nBound values: " << insertQuery.boundValues();
        QMessageBox::critical(this, "Database Error",
                              "Failed to sign up!\n" + insertQuery.lastError().text());
    }
}
