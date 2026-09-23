#include "mytcpserwer.h"

myTCPserwer::myTCPserwer(QObject *parent) : QObject(parent), gniazdoKlienta(nullptr)
{
    serwer = new QTcpServer(this);
    connect(serwer, &QTcpServer::newConnection, this, &myTCPserwer::obsluzNowePolaczenie);
}

myTCPserwer::~myTCPserwer() { zatrzymajSerwer(); }

bool myTCPserwer::uruchomSerwer(quint16 port)
{
    zatrzymajSerwer();
    return serwer->listen(QHostAddress::Any, port);
}

void myTCPserwer::zatrzymajSerwer()
{
    serwer->close();
    if (gniazdoKlienta) {
        QTcpSocket *tempGniazdo=gniazdoKlienta;
        gniazdoKlienta = nullptr;
        tempGniazdo->abort();
        tempGniazdo->deleteLater();
    }
    bufor.clear();
}

void myTCPserwer::obsluzNowePolaczenie()
{
    if (serwer->hasPendingConnections()) {

        gniazdoKlienta = serwer->nextPendingConnection();

        connect(gniazdoKlienta, &QTcpSocket::readyRead, this, &myTCPserwer::czytajDaneZGniazda);
        connect(gniazdoKlienta, &QTcpSocket::disconnected, this, &myTCPserwer::obsluzRozlaczenieKlienta);

        emit klientPodlaczony();
    }
}

void myTCPserwer::obsluzRozlaczenieKlienta()
{
    if(gniazdoKlienta){
        gniazdoKlienta->deleteLater();
        gniazdoKlienta = nullptr;
    }

    bufor.clear();
    emit klientOdlaczony();
}

void myTCPserwer::wyslijDane(quint8 typ, const QByteArray &dane)
{
    if (!gniazdoKlienta || gniazdoKlienta->state() != QAbstractSocket::ConnectedState) return;

    QByteArray ramka;
    QDataStream strumien(&ramka, QIODevice::WriteOnly);

    quint32 dlugoscDanych = static_cast<quint32>(dane.size());

    strumien << NAGLOWEK_SYNCHRONIZACJI;
    strumien << typ;
    strumien << dlugoscDanych;

    ramka.append(dane);

    gniazdoKlienta->write(ramka);
    gniazdoKlienta->flush();
}

bool myTCPserwer::isListening() const
{
    return serwer && serwer->isListening();
}

void myTCPserwer::czytajDaneZGniazda()
{
    if (!gniazdoKlienta) return;

    bufor.append(gniazdoKlienta->readAll());

    while (bufor.size() >= 9) {

        QDataStream strumien(bufor);
        quint32 odebranyNaglowek;
        quint8 typPaczki;
        quint32 dlugoscDanych;

        strumien >> odebranyNaglowek >> typPaczki >> dlugoscDanych;

        if (odebranyNaglowek != NAGLOWEK_SYNCHRONIZACJI) { bufor.remove(0, 1); continue; }

        if (bufor.size() >= (9 + dlugoscDanych)) {

            QByteArray wlasciweDane = bufor.mid(9, dlugoscDanych);
            bufor.remove(0, 9 + dlugoscDanych);
            emit odebranoDane(typPaczki, wlasciweDane);
        }
        else break;
    }
}
