#include "tcp_file_server.h"

tcp_file_server::tcp_file_server(quint16 port, QObject *parent) : QTcpServer(parent)
{
    bind_port = port;
}

tcp_test_client::tcp_test_client(quint16 port, QObject *parent) : QTcpSocket(parent)
{
    bind_port = port;
}


void tcp_file_server::start_service(){
    if(!listen(QHostAddress::LocalHost, bind_port)){
        qDebug() << "can't bind port " << bind_port ;
        QThread::sleep(5);
        qApp->exit(3);
        return;
    }
    qDebug() << "FTP Service Started";
    connect(this,SIGNAL(newConnection()),this,SLOT(newClient()));
}

void tcp_file_server::newClient()
{
    QTcpSocket *tcp_client = nextPendingConnection();
    if(!tcp_client->waitForReadyRead(1000)){
        qDebug() << "Client timed out!";
        return;
    }
    buffer=tcp_client->readAll();
    if( buffer.contains(ftp_header) ){
        buffer = QString("ACK").toUtf8();
        tcp_client->write(buffer);
        QString file_name;
        QFile local_file;
        if(tcp_client->waitForReadyRead(1000)){
            buffer=tcp_client->readAll();
            file_name=QString::fromUtf8(buffer.data());
            //local_file.setFileName(file_name);
            local_file.setFileName("temp.out");
            local_file.open(QIODevice::WriteOnly);
            while(tcp_client->waitForReadyRead(100)){
                buffer=tcp_client->readAll();
                local_file.write(buffer);
            }
            local_file.close();
            qDebug()<<"File Recieved";
        }
    }
}

void tcp_handle::parse_packet(){
}

void tcp_handle::recieve_file(){
}


void tcp_test_client::test_message(){
    connectToHost(QHostAddress::LocalHost, bind_port);
    if(!waitForConnected(1000)) {
        qDebug() << "Could Not Connect!";
        QThread::sleep(5);
        qApp->exit(3);
        return;
    }

    //buffer = QString("Testing Self").toUtf8();
    //write(buffer);
    send_file("dream.jpg");
}

void tcp_test_client::send_file(QString file_name){
    QFile local_file(file_name);
    if(!local_file.open(QIODevice::ReadOnly)){ qDebug() << "File not available";     return;}
    if(!isWritable()){
        qDebug() << "Not writable";
        return;
    }
    buffer = QString(ftp_header).toUtf8();
    qDebug() << "Sending " << buffer;
    write(buffer);
    if(!waitForReadyRead(1000)) {
        qDebug() << "No Response";
        return;
    }
    buffer = readAll();
    if(!buffer.contains("ACK") ) { qDebug() << "Invalid Repsonse" << buffer.data();   return;}
    buffer = file_name.toUtf8();
    write(buffer);
    waitForBytesWritten();
    while(!local_file.atEnd()){
        write(local_file.read(tcp_max_size));
    }
    local_file.close();
    buffer = QString(eof_header).toUtf8();
}
