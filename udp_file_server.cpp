
#include <QObject>
#include <QUdpSocket>
#include <QtCore>
#include <QHostAddress>

#include "udp_file_server.h"

udp_file_server::udp_file_server(quint16 port, QObject *parent) : QUdpSocket(parent)
{
    bind_port = port;
}

void udp_file_server::start_service(){
    if(!bind(QHostAddress::LocalHost, bind_port)){
        qDebug() << "can't bind port " << bind_port ;
        QThread::sleep(5);
        qApp->exit(3);
        return;
    }
    qDebug() << "FTP Service Started";
    connect(this,SIGNAL(readyRead()),this,SLOT(parse_packet()));
}

void udp_file_server::test_message(){
    //buffer = QString("Testing Self").toUtf8() + '\0';
    //this->writeDatagram(buffer.data(), QHostAddress::LocalHost, bind_port);
    //if(handshake( QHostAddress::LocalHost, 1234)) qDebug() << "Handshake Successful!";
    qDebug() << "Sending File!";
    send_file("dream.jpg", QHostAddress::LocalHost, 1000);
}

bool udp_file_server::handshake(QHostAddress request_host, quint16 request_host_port){
    buffer = QString(hs_header).toUtf8() + '\0';
    writeDatagram(buffer.data(), request_host, request_host_port);
    if(!waitForReadyRead(1000)) {
        qDebug() << "No Response!";
        return false;
    }
    QHostAddress response_host;
    quint16 response_host_port;
    buffer.resize(pendingDatagramSize());
    readDatagram(buffer.data(), buffer.size(), &response_host, &response_host_port);
    if(buffer.contains("ACK")) return true;
    else return false;

}

void udp_file_server::parse_packet(){
    disconnect(this,SIGNAL(readyRead()),this,SLOT(parse_packet()));
    QHostAddress sender;
    quint16 sender_port;
    buffer.resize(pendingDatagramSize());
    readDatagram(buffer.data(), buffer.size(), &sender, &sender_port);
    if( buffer.contains(hs_header) ){
        buffer = QString("ACK").toUtf8() + '\0';
        writeDatagram(buffer.data(), sender, sender_port);
    }
    if( buffer.contains(ftp_header) ){
        buffer = QString("ACK").toUtf8() + '\0';
        writeDatagram(buffer.data(), sender, sender_port);
        recieve_file(sender,sender_port);
    }
    connect(this,SIGNAL(readyRead()),this,SLOT(parse_packet()));
}

void udp_file_server::recieve_file(QHostAddress current_sender, quint16 current_sender_port){
    QHostAddress sender;
    quint16 sender_port;
    QString file_name;
    QFile local_file;
    if(waitForReadyRead(1000)){
        buffer.resize(pendingDatagramSize());
        readDatagram(buffer.data(), buffer.size(), &sender, &sender_port);
        file_name=QString::fromUtf8(buffer.data());
        local_file.setFileName(file_name);
        local_file.open(QIODevice::WriteOnly);
        while(waitForReadyRead(100)){
            buffer.resize(pendingDatagramSize());
            readDatagram(buffer.data(), buffer.size(), &sender, &sender_port);
            local_file.write(buffer);
        }
        local_file.close();
    }
}

void udp_file_server::send_file(QString file_name, QHostAddress server, quint16 server_port){
    QFile local_file(file_name);
    if(!local_file.open(QIODevice::ReadOnly)){ qDebug() << "File not available";     return;}
    disconnect(this,SIGNAL(readyRead()),this,SLOT(parse_packet()));
    buffer = QString(ftp_header).toUtf8() + '\0';
    writeDatagram(buffer.data(), server, server_port);
    if(!waitForReadyRead(1000)) {
        qDebug() << "No Response!";
        return;
    }

    QHostAddress response_host;
    quint16 response_host_port;
    buffer.resize(pendingDatagramSize());
    readDatagram(buffer.data(), buffer.size(), &response_host, &response_host_port);
    if(!buffer.contains("ACK") ) { qDebug() << "Invalid Repsonse" << buffer.data();   return;}
    buffer = file_name.toUtf8() + '\0';
    writeDatagram(buffer.data(), server, server_port);
    while(!local_file.atEnd()){
        Sleeper::msleep(1);
        writeDatagram(local_file.read(udp_max_size), server, server_port);
    }
    local_file.close();
    buffer = QString(eof_header).toUtf8() + '\0';
    writeDatagram(buffer.data(), server, server_port);
    connect(this,SIGNAL(readyRead()),this,SLOT(parse_packet()));
    qDebug() << "File Sent!";
}
