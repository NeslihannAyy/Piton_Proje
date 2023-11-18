#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);



    ui->tabWidget->setUsesScrollButtons(false);
    ui->lineEdit_UserLogin_pass->setValidator(new QIntValidator());
    ui->lineEdit_transferMoney_customerID->setValidator(new QIntValidator());
    ui->lineEdit_withdrawMoney_amount->setValidator(new QIntValidator());
    ui->lineEdit_depositMoney_amount->setValidator(new QIntValidator());
    ui->lineEdit_transferMoney_amount->setValidator(new QIntValidator());

    ui->label_loginInfo->setText("Hoşgeldiniz. Lütfen giriş yapınız.");
    serverStatus=false;
    connectServer();
    timer= new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(reconnect2Server()));
    timer->start(3000);


}

MainWindow::~MainWindow()
{
    if(socket->isOpen())
        socket->close();
    delete ui;
}

void MainWindow::readSocket()
{
    QByteArray buffer;

    QDataStream socketStream(socket);
    socketStream.setVersion(QDataStream::Qt_5_14);

    socketStream.startTransaction();
    socketStream >> buffer;

    if(!socketStream.commitTransaction())
    {
        QString message = QString("%1 :: Veri gelmesi bekleniyor..").arg(socket->socketDescriptor());
        emit newMessage(message);
        return;
    }

    buffer = buffer.mid(128);

    QString message = QString("%1;%2").arg(socket->socketDescriptor()).arg(QString::fromStdString(buffer.toStdString()));
    emit newMessage(message);

}

void MainWindow::discardSocket()
{
    socket->deleteLater();
    socket=nullptr;
    ui->statusBar->showMessage("Bağlantı Kesildi!");
    serverStatus=false;

    if(ID!=-1)
    {

        ui->label_ServerInfoMessage->setText("Oturumunuz sonlandırıldı!");

        ID=-1;
        bank="";
        name="";
        balance=-1;
    }

    else
    {

        ui->label_ServerInfoMessage->setText("QTCPServer ile bağlantı sonlandırıldı.");


    }

}

void MainWindow::connectServer()
{
    socket = new QTcpSocket(this);

    connect(this, &MainWindow::newMessage, this, &MainWindow::readMessage);
    connect(socket, &QTcpSocket::readyRead, this, &MainWindow::readSocket);
    connect(socket, &QTcpSocket::disconnected, this, &MainWindow::discardSocket);

    //    connect(socket, &QAbstractSocket::connected, this, &MainWindow::connectServer);

    socket->connectToHost(QHostAddress::LocalHost,8080);

    if(socket->waitForConnected())
    {
        ui->statusBar->showMessage("Sunucuya bağlandı.");
        serverStatus=true;


        ui->tabWidget->setTabEnabled(0,true);
        ui->lineEdit_UserLogin_userName->clear();


    }

    else
    {
        ui->statusBar->showMessage(QString("Bağlantı bekleniyor... - HATA: %1.").arg(socket->errorString()));

    }




}

void MainWindow::reconnect2Server()
{
    if(!serverStatus)
    {

        //serverStatus=false;
        socket = new QTcpSocket(this);

        connect(this, &MainWindow::newMessage, this, &MainWindow::readMessage);
        connect(socket, &QTcpSocket::readyRead, this, &MainWindow::readSocket);
        connect(socket, &QTcpSocket::disconnected, this, &MainWindow::discardSocket);


        //    connect(socket, &QAbstractSocket::connected, this, &MainWindow::connectServer);

        socket->connectToHost(QHostAddress::LocalHost,8080);

        if(socket->waitForConnected())
        {
            ui->statusBar->showMessage("Sunucuya bağlandı.");
            serverStatus=true;






        }

        else
        {
            ui->statusBar->showMessage(QString("Bağlantı bekleniyor... - HATA: %1.").arg(socket->errorString()));
    //        exit(EXIT_FAILURE);
        }
    }
    else
        ui->statusBar->showMessage("Sunucuya bağlandı.");
}



void MainWindow::displayError(QAbstractSocket::SocketError socketError)
{

       switch (socketError) {
           case QAbstractSocket::RemoteHostClosedError:
           break;
           case QAbstractSocket::HostNotFoundError:
               ui->statusBar->showMessage(QString("Bağlantı bekleniyor... - Host adını ve bağlantı noktası ayarlarını kontrol edin. :").arg(socket->errorString()));
           break;
           case QAbstractSocket::ConnectionRefusedError:
               ui->statusBar->showMessage(QString("Bağlantı bekleniyor... - QTCPServer'ın çalıştığından emin olun. :").arg(socket->errorString()));

           break;
           default:
               //ui->statusBar->showMessage(QString("Bağlantı bekleniyor... - HATA: %1.").arg(socket->errorString()));
               qDebug()<<"default error: "<<socket->errorString();
               //QMessageBox::information(this, "QTCPClient", QString("Bir hata oluştu: %1.").arg(socket->errorString()));
           break;
       }

}

void MainWindow::readMessage(const QString& str)
{


    qDebug()<<"str:"<<str;
    if(strGet!=str)
    {
       strGet=str;
       qDebug()<<"strGet:"<<strGet;

       if(str.contains(';'))
       {
           strServerPort=str.split(';');
           strList= strServerPort[1].split(',');
           if(strList[0]=="UserLogin")
           {
               if(strList[1]!="incorrect username or password")
               {
                   if(strList[1]!="sameUserError"){


                   ID=strList[1].toInt();
                   name=strList[2];
                   bank=strList[3];
                   balance=strList[4].toDouble();

                   ui->label_CustomerData_ID->setText(QString::number(ID));
                   ui->label_CustomerData_name->setText(name);
                   ui->label_CustomerData_nameOfTheBank->setText(bank);
                   ui->label_CustomerData_balance->setText(QString::number(balance)+"₺");

                   }
                   else
                   {
                       QMessageBox::warning(this,"Kullanıcı Girişi Reddedildi ","Kullanıcı başka bir client üzerinde giriş yapmış.");
                   }


               }
               else
               {
                   QMessageBox::warning(this,"Müşteri Girişi","Kullanıcı adı veya şifre yanlış.");
               }
           }
           else if(strList[0]=="depositMoney")
           {
               if(strList[1]=="success")
               {
                   balance=strList[2].toDouble();
                   ui->label_CustomerData_balance->setText(QString::number(balance)+"₺");

                   QMessageBox::information(this,"Para Yatırma","Para Yatırma başarılı.\nMevcut Bakiye: " +strList[2]+"₺");

               }
           }
           else if(strList[0]=="withdrawMoney")
           {
               if(strList[1]=="success")
               {
                   balance=strList[2].toDouble();
                   ui->label_CustomerData_balance->setText(QString::number(balance)+"₺");

                   QMessageBox::information(this,"Para Çekme","Para Çekildi.\nMevcut Bakiye: " +strList[2]+"₺");


               }
               else
               {
                   QMessageBox::information(this,"Para Çekme","Yetersiz Bakiye!\nMevcut Bakiye: " +strList[2]+"₺");

               }
           }
           else if(strList[0]=="transferMoney")
           {
               if(strList[1]=="success")
               {
                   balance=strList[3].toDouble();
                   ui->label_CustomerData_balance->setText(QString::number(balance)+"₺");

                   QMessageBox::information(this,"Para Transferi","Para Transferi Gerçekleşti. \n"+strList[2]+"\nMevcut Bakiye: " +strList[3]+"₺");


                   ui->lineEdit_transferMoney_amount->clear();
                   ui->lineEdit_transferMoney_customerID->clear();
                   ui->lineEdit_transferMoney_customerName->clear();
                   ui->label_nameHint->clear();
               }
               else if(strList[1]=="receiverIsNotFound"){

                   QMessageBox::warning(this,"Para Transferi","Alıcı Bulunamadı!\nLütfen alıcı müşteri numarasını kontrol ediniz. ");

               }
               else if(strList[1]=="receiverNameIsIncorrect"){

                   QMessageBox::warning(this,"Para Transferi","Alıcı Adı Hatalı!\nLütfen alıcı adını kontrol ediniz. ");

               }
               else if(strList[1]=="invalidReceiver")
               {

                   QMessageBox::warning(this,"Para Transferi","Geçersiz alıcı!\n Lütfen alıcı müşteri numarasını kontrol ediniz. ");

               }
               else
               {
                   QMessageBox::warning(this,"Para Transferi","Yetersiz Bakiye! \nMevcut Bakiye: " +strList[2]+"₺");

               }
           }
           else if(strList[0]=="customerIDCheck")
           {
               if(strList[1]!='-')
               {
                   recevier_name= strList[1].split(' ')[0];
                   recevier_lastName=strList[1].split(' ')[1];
                   ui->label_nameHint->setText("İpucu: "+recevier_name[0]+recevier_name[1]+"*** "+recevier_lastName[0]+recevier_lastName[1]+"***");

               }
               else
               {
                   ui->label_nameHint->setText("Alıcı Bulunamadı");

               }

           }
           else if(strList[0]=="moneyTransferHasArrived")
           {
               if(strList[1]!=ID)
               {
                   ID=strList[1].toInt();
                   name=strList[2];
                   bank=strList[3];
                   balance=strList[4].toDouble();

                   ui->label_CustomerData_ID->setText(QString::number(ID));
                   ui->label_CustomerData_name->setText(name);
                   ui->label_CustomerData_nameOfTheBank->setText(bank);
                   ui->label_CustomerData_balance->setText(QString::number(balance)+"₺");

                   QMessageBox::information(this,"Para Transferi","Hesabınıza Para Geldi! ☺ \nMevcut Bakiye: " +strList[4]+"₺");


               }


           }


       }

    }


}

void MainWindow::sendStr()
{
    QDataStream socketStream(socket);
    socketStream.setVersion(QDataStream::Qt_5_14);



    QByteArray byteArray =  str.toUtf8();
    socketStream << byteArray;
}

void MainWindow::userLogin()
{
//   if(serverStatus)
//   {
       if (invalidEntry(ui->lineEdit_UserLogin_userName->text()+ui->lineEdit_UserLogin_pass->text()))
       {
           if(socket)
           {
               if(socket->isOpen())
               {

                   str = "UserLogin,"+ui->lineEdit_UserLogin_userName->text() + ","+ ui->lineEdit_UserLogin_pass->text();
                   sendStr();

                   ui->lineEdit_UserLogin_pass->clear();

               }
               else
                   QMessageBox::critical(this,"QTCPClient","Socket kapalı.");
           }
           else
               QMessageBox::critical(this,"QTCPClient","Client, sunucuya bağlı değil.");

       }
       else
           QMessageBox::critical(this,"Hata!","Kullanıcı adı veya şifre izin verilmeyen karekter içeriyor.");



}

void MainWindow::depositMoney()
{
    if(ui->lineEdit_depositMoney_amount->text().toDouble()>0)
    {
        if(socket)
        {
            if(socket->isOpen())
            {
                str = "depositMoney,"+QString::number(ID)+","+ QString::number(balance)+","+ui->lineEdit_depositMoney_amount->text();

                sendStr();

                ui->lineEdit_depositMoney_amount->clear();

            }
            else
                QMessageBox::critical(this,"QTCPClient","Socket kapalı.");
        }
        else
            QMessageBox::critical(this,"QTCPClient","Client, sunucuya bağlı değil.");

    }
    else
        QMessageBox::critical(this,"Tutar Hatalı!","Lütfen sıfırdan büyük bir değer giriniz.");
}

void MainWindow::withdrawMoney()
{
    if(ui->lineEdit_withdrawMoney_amount->text().toDouble()>0)
    {
        if(socket)
        {
            if(socket->isOpen())
            {
                str =  "withdrawMoney,"+
                        QString::number(ID)+","+
                        QString::number(balance)+","+
                        ui->lineEdit_withdrawMoney_amount->text();

                sendStr();

                ui->lineEdit_withdrawMoney_amount->clear();

            }
            else
                QMessageBox::critical(this,"QTCPClient","");
        }
        else
            QMessageBox::critical(this,"QTCPClient","Client, sunucuya bağlı değil.");
    }
    else
        QMessageBox::critical(this,"Tutar Hatalı!","Lütfen sıfırdan büyük bir değer giriniz.");

}

void MainWindow::transferMoney()
{
    if(invalidEntry(ui->lineEdit_transferMoney_customerName->text()))
    {
        if(ui->lineEdit_transferMoney_amount->text().toDouble()>0)
        {
            if(socket)
            {
                if(socket->isOpen())
                {                   //0
                    str = "transferMoney,"+QString::number(ID)+","+             //1
                            ui->lineEdit_transferMoney_customerID->text()+","+  //2
                            QString::number(balance)+","+                       //3
                            ui->lineEdit_transferMoney_amount->text()+","+      //4
                            ui->lineEdit_transferMoney_customerName->text();    //5

                    sendStr();

                }
                else
                    QMessageBox::critical(this,"QTCPClient","Socket kapalı.");
            }
            else
                QMessageBox::critical(this,"QTCPClient","Client, sunucuya bağlı değil.");

        }
        else
            QMessageBox::critical(this,"Tutar Hatalı!","Lütfen sıfırdan büyük bir değer giriniz.");
    }

    else
        QMessageBox::critical(this,"Hata!","Alıcı adı geçersiz karekter içeriyor.");

}

bool MainWindow::invalidEntry(QString text)
{
    if(text.contains(';') || text.contains(',') || text.length()==0)
    {
        return false;
    }
    else
        return true;
}



void MainWindow::on_pushButton_depositMoney_clicked()
{


    ui->label_depositMoney_CustomerData->setText("Kullanılabilir Bakiye: "+QString::number(balance)+"₺");
}


void MainWindow::on_pushButton_withdrawMoney_clicked()
{

    ui->label_withdrawMoney_CustomerData->setText("Kullanılabilir Bakiye: "+QString::number(balance)+"₺");
}


void MainWindow::on_pushButton_transferMoney_clicked()
{

    ui->label_transferMoney_CustomerData->setText("Kullanılabilir Bakiye: "+QString::number(balance)+"₺");

}


void MainWindow::on_pushButton_exit_clicked()
{
    if(socket)
    {
        if(socket->isOpen())
        {
            str = "userLogout,"+QString::number(ID);
            sendStr();
        }
        else
            QMessageBox::critical(this,"QTCPClient","Socket kapalı."
                                                    "\nSunucuya oturum kapatma bilgisi gönderilemedi.");
    }
    else
        QMessageBox::critical(this,"QTCPClient","Client, sunucuya bağlı değil."
                                                "\nSunucuya oturum kapatma bilgisi gönderilemedi.");


    QMessageBox::information(this,"Oturum Kapatıldı","Sn. "+name+ " Oturumunuz sonlandırıldı.");

    ID=-1;
    bank="";
    name="";
    balance=-1;

}


void MainWindow::on_pushButton_UserLogin_login_clicked()
{
    userLogin();
}
void MainWindow::on_lineEdit_UserLogin_pass_returnPressed()
{
    userLogin();
}


void MainWindow::on_pushButton_depositMoney_cancel_clicked()
{

    ui->lineEdit_depositMoney_amount->clear();

}


void MainWindow::on_pushButton_depositMoney_okay_clicked()
{
    depositMoney();

}

void MainWindow::on_lineEdit_depositMoney_amount_returnPressed()
{
   depositMoney();
}


void MainWindow::on_pushButton_withdrawMoney_cancel_clicked()
{

    ui->lineEdit_withdrawMoney_amount->clear();

}


void MainWindow::on_pushButton_withdrawMoney_okay_clicked()
{
    withdrawMoney();

}

void MainWindow::on_lineEdit_withdrawMoney_amount_returnPressed()
{
    withdrawMoney();
}


void MainWindow::on_pushButton_transferMoney_cancel_clicked()
{


    ui->lineEdit_transferMoney_amount->clear();
    ui->lineEdit_transferMoney_customerID->clear();
    ui->lineEdit_transferMoney_customerName->clear();
    ui->label_nameHint->clear();

}

void MainWindow::on_pushButton_transferMoney_okay_clicked()
{
    transferMoney();

}

void MainWindow::on_lineEdit_transferMoney_amount_returnPressed()
{
    transferMoney();
}

void MainWindow::on_lineEdit_transferMoney_customerID_editingFinished()
{
    if(ui->lineEdit_transferMoney_customerID->text().length()>0)
    {
        if(socket)
        {
            if(socket->isOpen())
            {
                str = "customerIDCheck,"+                            //0
                      ui->lineEdit_transferMoney_customerID->text(); //1

                sendStr();
            }
            else
                QMessageBox::critical(this,"QTCPClient","Socket kapalı.");
        }
        else
            QMessageBox::critical(this,"QTCPClient","Client, sunucuya bağlı değil.");
    }


}



void MainWindow::on_lineEdit_transferMoney_customerID_textEdited(const QString &arg1)
{
    ui->label_nameHint->clear();
}


void MainWindow::on_pushButton_UserLogin_cancel_clicked()
{
    ui->lineEdit_UserLogin_userName->clear();
    ui->lineEdit_UserLogin_pass->clear();

}


