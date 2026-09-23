#include "myTCPclient.h"

myTCPclient::myTCPclient(QObject *parent) : QObject(parent)
{
    gniazdo = new QTcpSocket(this);
    connect(gniazdo, &QTcpSocket::connected, this, &myTCPclient::obsluzPolaczenie);
    connect(gniazdo, &QTcpSocket::disconnected, this, &myTCPclient::obsluzRozlaczenie);
    connect(gniazdo, &QTcpSocket::readyRead, this, &myTCPclient::czytajDaneZGniazda);

    connect(gniazdo, &QTcpSocket::errorOccurred, this, [this](QAbstractSocket::SocketError) {
        gniazdo->abort();
    });
}

myTCPclient::~myTCPclient()
{
    gniazdo->abort();
}

void myTCPclient::polaczZSerwerem(const QString &ip, quint16 port)
{
    gniazdo->abort();
    bufor.clear();
    gniazdo->connectToHost(ip, port);
}

void myTCPclient::rozlaczZSerwerem()
{
    if(gniazdo){
        gniazdo->disconnectFromHost();
        gniazdo->abort();
    }
    bufor.clear();
}

bool myTCPclient::isConnected() const
{
    return gniazdo->state() == QAbstractSocket::ConnectedState;
}

void myTCPclient::wyslijDane(quint8 typ, const QByteArray &dane)
{
    if (!isConnected()) return;
    QByteArray ramka;
    QDataStream strumien(&ramka, QIODevice::WriteOnly);
    quint32 dlugoscDanych = static_cast<quint32>(dane.size());
    strumien << NAGLOWEK_SYNCHRONIZACJI;
    strumien << typ;
    strumien << dlugoscDanych;
    ramka.append(dane);
    gniazdo->write(ramka);
    gniazdo->flush();
}

void myTCPclient::obsluzPolaczenie()
{
    qDebug() << "myTCPclient::polaczono";
    emit polaczono();
}

void myTCPclient::obsluzRozlaczenie()
{
    emit rozlaczono();
}

void myTCPclient::czytajDaneZGniazda()
{
    bufor.append(gniazdo->readAll());
    while (bufor.size() >= 9) {
        QDataStream strumien(bufor);
        quint32 odebranyNaglowek;
        quint8 typPaczki;
        quint32 dlugoscDanych;
        strumien >> odebranyNaglowek >> typPaczki >> dlugoscDanych;
        if (odebranyNaglowek != NAGLOWEK_SYNCHRONIZACJI) {
            bufor.remove(0, 1);
            continue;
        }
        if (bufor.size() >= (9 + dlugoscDanych)) {
            QByteArray wlasciweDane = bufor.mid(9, dlugoscDanych);
            bufor.remove(0, 9 + dlugoscDanych);
            emit odebranoDane(typPaczki, wlasciweDane);
        } else break;
    }
}
