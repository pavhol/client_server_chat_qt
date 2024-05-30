#include "dialog.h"
#include "ui_dialog.h"
#include <QMessageBox>
Dialog::Dialog(QWidget *parent) :QDialog(parent), ui(new Ui::Dialog)
{
    ui->setupUi(this);
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("./ChatLogs.sqlite");
    if(db.open())
    {
        QMessageBox::information(0,"Success!", "Connection with database is successed!");
    }
    else
    {
        QMessageBox::warning(0,"database error", db.lastError().text());
    }
    _serv = new MyServer(this, this);
    connect(this, SIGNAL(messageFromGui(QString,QStringList)), _serv, SLOT(onMessageFromGui(QString,QStringList)));
    connect(_serv, SIGNAL(addLogToGui(QString,QColor)), this, SLOT(onAddLogToGui(QString,QColor)));
    connect(_serv,SIGNAL(addUser(QString)),this,SLOT(onAddUserToGui(QString)));
    connect(_serv,SIGNAL(removeUser(QString)),this,SLOT(onRemoveUserFromGui(QString)));
    if (_serv->doStartServer(QHostAddress::LocalHost, 1234))
    {
        addToLog(" server started at "+ui->leHost->text()+":"+ui->lePort->text(), Qt::green);
        ui->pbStartStop->setText("Stop server");
    }
    else
    {
        addToLog(" server not started at "+_serv->serverAddress().toString()+":" + QString::number(_serv->serverPort()), Qt::red);
        ui->pbStartStop->setChecked(true);
    }
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::onAddUserToGui(QString name)
{
    ui->lwUsers->addItem(name);
}

void Dialog::onRemoveUserFromGui(QString name)
{
    for (int i = 0; i < ui->lwUsers->count(); ++i)
        if (ui->lwUsers->item(i)->text() == name)
        {
            ui->lwUsers->takeItem(i);
            break;
        }
}


void Dialog::onAddLogToGui(QString string, QColor color)
{
    addToLog(string, color);
}

void Dialog::on_pbSend_clicked()
{
    if (ui->lwUsers->count() == 0)
    {
        QMessageBox::information(this, "", "No clients connected");
        return;
    }
    QStringList l;
    if (!ui->cbToAll->isChecked())
        foreach (QListWidgetItem *i, ui->lwUsers->selectedItems())
            l << i->text();
    emit messageFromGui(ui->pteMessage->document()->toPlainText(), l);
    ui->pteMessage->clear();
    if (l.isEmpty())
        addToLog("Sended public server message", Qt::black);
    else
        addToLog("Sended private server message to "+l.join(","), Qt::black);
}

void Dialog::on_cbToAll_clicked()
{
    if (ui->cbToAll->isChecked())
        ui->pbSend->setText("Send To All");
    else
        ui->pbSend->setText("Send To Selected");
}


void Dialog::on_pbStartStop_toggled(bool checked)
{
    if (checked)
    {
        addToLog(" server stopped at "+_serv->serverAddress().toString()+":"+QString::number(_serv->serverPort()), Qt::green);
        _serv->doStopServer();
        ui->pbStartStop->setText("Start server");
    }
    else
    {
        QHostAddress addr;
        if (!addr.setAddress(ui->leHost->text()))
        {
            addToLog(" invalid address "+ui->leHost->text(), Qt::red);
            return;
        }
        if (_serv->doStartServer(addr, ui->lePort->text().toInt()))
        {
            addToLog(" server started at "+ui->leHost->text()+":"+ui->lePort->text(), Qt::green);
            ui->pbStartStop->setText("Stop server");
        }
        else
        {
            addToLog(" server not started at "+ui->leHost->text()+":"+ui->lePort->text(), Qt::red);
            ui->pbStartStop->setChecked(true);
        }
    }
}

void Dialog::addToLog(QString text, QColor color)
{
    ui->lwLog->insertItem(0, QTime::currentTime().toString()+text);
    ui->lwLog->item(0)->setTextColor(color);
    QSqlQuery query(db);
    QString qry_create = "CREATE TABLE IF NOT EXISTS Messages("
                         "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                         "datetime TEXT,"
                         "message TEXT"
                         ");";

    QString qry_insert = "INSERT INTO Messages(datetime, message)"
                         "VALUES ("
                         "(SELECT datetime(\'now\')), \'" + text + "\'" +
                         ");";
    query.exec(qry_create);
    query.exec(qry_insert);
    // if(query.exec(qry_create))
    //     qDebug() << "query exec() " << qry_create;
    // else
    //     qDebug() << "query error() " << query.lastError();
    // if(query.exec(qry_insert))
    //     qDebug() << "query exec() " << qry_insert;
    // else
    //     qDebug() << "query error() " << query.lastError();

    db.commit();
}
