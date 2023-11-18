#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    TCP_Server = new QTcpServer();
    if(TCP_Server->listen(QHostAddress::Any,8080))
    {
        connect(TCP_Server, &QTcpServer::newConnection, this, &MainWindow::newConnection);
        ui->statusbar->showMessage("TCP Server Started");
    }
    else
    {
        QMessageBox::information(this, "TCP Server Error, ", TCP_Server->errorString());
    }

}
MainWindow::~MainWindow()
{
    delete ui;

}

void MainWindow::readSocket()
{

    QTcpSocket *socket = reinterpret_cast<QTcpSocket*>(sender());
    QByteArray message = socket->readAll();
    qDebug() << "Raw Message: " << message;


    QString messageStr = QString::fromUtf8(message);
    qDebug() << "Converted Message: " << messageStr;


    QStringList parts = messageStr.split("UserLogin,");
    qDebug() << "Split Parts: " << parts;


    if (parts.size() >= 2) {
        QStringList userDataParts = parts[1].split(',');


        if (userDataParts.size() >= 2) {
            UserLogin user;
            user.name = userDataParts[0].trimmed();
            user.password = userDataParts[1].trimmed();


            qDebug() << "name:" << user.name << ", password:" << user.password;
        } else {
            qDebug() << "Hatalı kullanıcı verisi formatı";
        }
    } else {
        qDebug() << "Hatalı mesaj formatı";
    }
  // user_login_check();
}




void MainWindow::discardSocket()
{
    QTcpSocket *socket = reinterpret_cast<QTcpSocket*>(sender());
    int idx = Client_List.indexOf(socket);
    if(idx > -1)
    {

        Client_List.removeAt(idx);

    }
    ui->comboBox_Client_List->clear();
    foreach(QTcpSocket *sockettemp, Client_List)
    {
        ui->comboBox_Client_List->addItem(QString::number(sockettemp->socketDescriptor()));
    }
    socket-> deleteLater();
}

void MainWindow::newConnection()
{
    while (TCP_Server->hasPendingConnections())
    {
        AddToSocketList(TCP_Server->nextPendingConnection());
    }
}

void MainWindow::AddToSocketList(QTcpSocket *socket)
{
    Client_List.append(socket);
    connect(socket, &QTcpSocket::readyRead, this , &MainWindow::readSocket);
    connect(socket, &QTcpSocket::disconnected, this , &MainWindow::discardSocket );
    ui->textEdit_Messages->append("Client Is connected with Server : Socket ID : "+ QString::number(socket->socketDescriptor()));
    ui->comboBox_Client_List->addItem(QString::number(socket->socketDescriptor()));
}
void MainWindow::on_open_customer_information()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Müşteri bilgilerini seç", "", "Txt Dosyaları(*.txt)");
    if (!filePath.isEmpty()) {
        QFile file(filePath);

        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream stream(&file);

            QList<Customer> customers; // Her bir müşteriyi tutmak için QList

            while (!stream.atEnd()) {
                Customer customer;


                QString line = stream.readLine().trimmed();
                QStringList keyValue = line.split(",");

                for (const QString& pair : keyValue) {
                    QStringList item = pair.split(":");
                    if (item.size() == 2) {
                        QString key = item[0].trimmed().remove("\"");
                        QString value = item[1].trimmed().remove("\"");

                        // Veriyi ilgili alanlara yerleştir
                        if (key == "name") {
                            customer.name = value;
                        } else if (key == "password") {
                            customer.password = value;
                        } else if (key == "client number") {
                            customer.clientNumber = value;
                        } else if (key == "balance") {
                            customer.balance = value;
                        } else if (key == "bank") {
                            customer.bank = value;
                        }
                    }
                }


                customers.append(customer);


                qDebug() << "Current Customer: Name: " << customer.name
                         << ", Password: " << customer.password
                         << ", Client Number: " << customer.clientNumber
                         << ", Balance: " << customer.balance
                         << ", Bank: " << customer.bank;


            }

            file.close();

            // Şimdi customers listesinde her bir müşteri bilgisine erişebilirsiniz.
            // Örneğin, tüm müşteri bilgilerini yazdıralım:
            for (const Customer& customer : customers) {
                qDebug() << "Stored Customer: Name: " << customer.name
                         << ", Password: " << customer.password
                         << ", Client Number: " << customer.clientNumber
                         << ", Balance: " << customer.balance
                         << ", Bank: " << customer.bank;

            }
        }

    }

}


void MainWindow::Send_file(QTcpSocket *socket, QString filename)
{
    if(socket)
    {
            if(socket->isOpen())
            {
                QFile filedata(filename);
                if (filedata.open(QIODevice::ReadOnly))
                {
                    QFileInfo fileinfo(filedata);
                    QString FileNameWithExt(fileinfo.fileName());
                    QDataStream socketstream(socket);
                    socketstream.setVersion(QDataStream::Qt_5_14);
                    QString header = "filename: " + FileNameWithExt + ",filesize: " + QString::number(filedata.size());
                    header.prepend(" ");
                    header.resize(128);

                    QByteArray headerByteArray = header.toUtf8();

                    QByteArray ByteFileData = filedata.readAll();


                    ByteFileData.prepend(headerByteArray);


                    socketstream << ByteFileData;



               }

                else
                {
                    qDebug()<< "File not open";
                 }


            }
            else
            {
                qDebug()<< "Client Socket not open";

            }
    }
    else
    {
        qDebug()<< "Client Socket is invalid";

    }

}
//Serverda yer alması gereken eksik fonksiyonlar
//void MainWindow::user_login_check(){

//}
//void MainWindow::displayError(QAbstractSocket::SocketError socketError)
//{

//}

//void MainWindow::replyMessage(const QString &str)
//{

//}

//void MainWindow::moneyTransferMessage(int receiverID)
//{

//}

//void MainWindow::refreshLabel()
//{

//}

//void MainWindow::AutoSentMessage()
//{

//}

//bool MainWindow::sameUserError(int clientID)
//{

//}

