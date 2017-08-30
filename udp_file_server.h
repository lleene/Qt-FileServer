#ifndef UDP_FILE_SERVER_H
#define UDP_FILE_SERVER_H

#include <QObject>
#include <QUdpSocket>
#include <QtCore>
#include <QHostAddress>
#include <QFile>

#define hs_header "Handshake Header"
#define eof_header "CBIT EOF Header"
#define ftp_header "CBIT FTP Header"
#define udp_max_size 0x8000

class udp_file_server : public QUdpSocket
{
    Q_OBJECT

public:
    explicit udp_file_server(quint16 port, QObject *parent = Q_NULLPTR);
    void send_file(QString file_name, QHostAddress server, quint16 server_port);

private:
    quint16 bind_port;
    QByteArray buffer;
    void recieve_file(QHostAddress sender, quint16 sender_port);
    bool handshake(QHostAddress sender, quint16 sender_port);

signals:

public slots:
    void start_service();
    void test_message();

private slots:
    void parse_packet();

};

class Sleeper : public QThread
{
public:
    static void msleep(unsigned long msecs){QThread::msleep(msecs);}
};


#endif // UDP_FILE_SERVER_H
