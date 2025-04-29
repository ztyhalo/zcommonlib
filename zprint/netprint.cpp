#include "netprint.h"
#include <assert.h>
#include <stdarg.h>
#include <time.h>
#include "zprint.h"

using namespace std;

Print_Server * gNetP=NULL;

UPDATE_SOCKET::UPDATE_SOCKET(quint16 sendp , quint16 recvp)
    :upsendport(sendp), uprecport(recvp)
{
    upsocket =  new QUdpSocket(this);
    if(upsocket == NULL)
    {
        qDebug("update socket create failed!");
        return;
    }

    qDebug("bind 0x%x %d", uprecport, upsocket->bind(uprecport, QUdpSocket::ShareAddress));
    connect(upsocket, SIGNAL(readyRead()), this, SLOT(updateDataProcess()));
}


void UPDATE_SOCKET::updateDataProcess()
{

    QByteArray datagram;
    while (upsocket->hasPendingDatagrams()) {

        QHostAddress sender;
        quint16 port;
        datagram.resize(upsocket->pendingDatagramSize());
        upsocket->readDatagram(datagram.data(), datagram.size(), &sender, &port);
//        qDebug() << "udp rev callback " << port;
         up_data_process(datagram, sender);
//        emit data_receive( datagram, sender);
    }
}
int UPDATE_SOCKET::up_data_process(const QByteArray & data, const QHostAddress & host)
{
    qDebug() << data;
    (void)host;
    return 0;
}


int UPDATE_SOCKET::send_data(const QString & msg)
{
    return upsocket->writeDatagram(msg.toUtf8(), QHostAddress::Broadcast, upsendport);
}

int UPDATE_SOCKET::send_data(const QString & msg, const QString & ip)
{
    return upsocket->writeDatagram(msg.toUtf8(), QHostAddress(ip), upsendport);
}

int UPDATE_SOCKET::send_data(const QString & msg, const QHostAddress & ip)
{
    return upsocket->writeDatagram(msg.toUtf8(), ip, upsendport);
}

Print_Server::Print_Server(quint16 sendp, quint16 recvp):
    UPDATE_SOCKET(sendp, recvp),constate(0)
{
    sem_init(&psem, 0, 0);
}

Print_Server::~Print_Server()
{
    zprintf3("Print_Server destruct!\n");
    if(this->running)
    {
        constate = 0;
        this->running = 0;
        sem_post(&psem);
        this->waitEnd();
        if(sem_destroy(&psem) != 0)
        {
            zprintf1("Print_Server destruct sem_t error!\n");
            perror("Z_Buf_T sem_destory");
        }
    }

}
int Print_Server::up_data_process(const QByteArray & data, const QHostAddress & host)
{
    qDebug() << "data receive!";
    QString msg = data;
    if(msg == "start printf!")
    {
        zprintf1("net printf start!\n");
        constate = 1;
        printhost = host;
        this->start();
    }
    else if (msg == "stop printf!")
    {
        zprintf1("net printf stop!\n");
        constate = 0;
    }
    return 0;
}

void  Print_Server::netprintf(const char * format, ...)
{

    if(constate == 0) return;
    va_list args;
//     lock();

    struct timeval tv;
    struct Print_Time tm;
    gettimeofday(&tv, NULL);
    tm.tv = tv;
    va_start(args, format);

    char buf[1024];
    memset(buf, 0, 1024);
    (void) vsnprintf(buf, sizeof(buf),format, args);
    va_end(args);
    QString mesg(buf);
    lock();
    info.insert(tm, mesg);
    unlock();
    sem_post(&psem);


//    unlock();

}

void Print_Server::run()
{
    char buf[1024];
    struct timeval tv;
    QString        mg;

    while (this->running)
    {

        sem_wait(&(this->psem));
        if(this->running == 0)
           break;
        lock();
        QMap<struct Print_Time, QString >::iterator iter = info.begin();

        if(iter != info.end())
        {
            tv = iter.key().tv;
            mg = iter.value();
            info.erase(iter);
        }
        unlock();
         memset(buf, 0, 1024);
        struct tm info;
        if(localtime_r(&tv.tv_sec, &info) == NULL)
        {
             perror("localtime_r failed");
        }
        else
        {
            sprintf(buf,"%d-%02d-%02d %02d:%02d:%02d.%06ld ",
                 1900+info.tm_year, 1+info.tm_mon, info.tm_mday,
                 info.tm_hour, info.tm_min, info.tm_sec, tv.tv_usec);

        }
        QString mesg(buf);
        send_data(mesg + " " + mg);
    }
}

Print_Client::Print_Client(quint16 sendp , quint16 recvp)
    :UPDATE_SOCKET(sendp, recvp)
{
    send_data("start printf!");
}


int Print_Client::up_data_process(const QByteArray & data, const QHostAddress & host)
{
    QString msg = data;
    (void)host;
    qDebug() << msg;
    return 0;
}


//void  nprintf(const char * format, ...)
//{
//    if(gNetP != NULL)
//    {

//        va_list args;
//        va_start(args, format);
//        gNetP->netprintf(format, args);
//        va_end(args);
//    }
//}
