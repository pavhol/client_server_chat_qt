#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QDebug>
#include <QtGui>
#include <QtCore>
#include "myserver.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
namespace Ui {
    class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();

private:
    QSqlDatabase db;
    Ui::Dialog *ui;
    MyServer *_serv;
    void addToLog(QString text, QColor color);

signals:
    void messageFromGui(QString message, const QStringList &users);

public slots:
    void onAddUserToGui(QString name);
    void onRemoveUserFromGui(QString name);
    void onAddLogToGui(QString string, QColor color);

private slots:
    void on_pbSend_clicked();
    void on_cbToAll_clicked();
    void on_pbStartStop_toggled(bool checked);
};

#endif // DIALOG_H
