#ifndef MYTCPCLIENT_H
#define MYTCPCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QDataStream>

class myTCPclient : public QObject
{
    Q_OBJECT
public:
    explicit myTCPclient(QObject *parent = nullptr);
    ~myTCPclient();
    void polaczZSerwerem(const QString &ip, quint16 port);
    void rozlaczZSerwerem();
    void wyslijDane(quint8 typ, const QByteArray &dane);
    bool isConnected() const;
signals:
    void polaczono();
    void rozlaczono();
    void odebranoDane(quint8 typ, QByteArray dane);
    void wystapilBlad(QString opisBledu);
private slots:
    void czytajDaneZGniazda();
    void obsluzPolaczenie();
    void obsluzRozlaczenie();
private:
    QTcpSocket *gniazdo;
    QByteArray bufor;
    const quint32 NAGLOWEK_SYNCHRONIZACJI = 0xABCD1234;
};

#endif // MYTCPCLIENT_H
