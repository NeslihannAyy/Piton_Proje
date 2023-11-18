#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtNetwork/QTcpServer>
#include <QtGui>
#include <QtWidgets>
#include <QtCore>
#include <QTcpSocket>
#include <QString>
#include <QList>



QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE
struct Customer {
    QString name;
    QString password;
    QString clientNumber;
    QString balance;
    QString bank;
};
struct UserLogin {
    QString name;
    QString password;
};
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private slots:
    void readSocket();
    void discardSocket();
    void newConnection();
    void AddToSocketList(QTcpSocket *socket);
    void on_open_customer_information();
    //void user_login_check();

//    void displayError(QAbstractSocket::SocketError socketError);
//     void replyMessage(const QString& str);
//    void sendMessage(QTcpSocket* socket);
//    void moneyTransferMessage(int receiverID);
//    void refreshLabel();
//    void AutoSentMessage();
//    bool sameUserError(int clientID);


private:
    void Send_file(QTcpSocket *socket, QString filename);
private:
    Ui::MainWindow *ui;
    QTcpServer *TCP_Server;
    QList<QTcpSocket*> Client_List;
};
#endif // MAINWINDOW_H
