#ifndef TCP_FILE_SERVER_H
#define TCP_FILE_SERVER_H

#include <QtCore>
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QHostAddress>
#include <QThread>
#include <QFile>

#define eof_header "CBIT EOF Header"
#define ftp_header "CBIT FTP Header"
#define tcp_max_size 0x8000

class tcp_file_server : public QTcpServer
{
    Q_OBJECT

public:
    explicit tcp_file_server(quint16 port, QObject *parent = Q_NULLPTR);

private:
    QByteArray buffer;
    quint16 bind_port;

public slots:
    void start_service();
    void newClient();
};

class tcp_handle : public QTcpSocket
{
    Q_OBJECT

private:
    QByteArray buffer;
    void recieve_file();

public slots:
    void parse_packet();

};

class tcp_test_client : public QTcpSocket
{
    Q_OBJECT

public:
    explicit tcp_test_client(quint16 port, QObject *parent = Q_NULLPTR);
    void send_file(QString file_name);

private:
    quint16 bind_port;
    QByteArray buffer;

public slots:
    void test_message();

};



#endif // TCP_FILE_SERVER_H
