#ifndef MYTCPSERWER_H
#define MYTCPSERWER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QDataStream>

class myTCPserwer : public QObject
{
    Q_OBJECT
public:
    explicit myTCPserwer(QObject *parent = nullptr);
    ~myTCPserwer();

    bool uruchomSerwer(quint16 port);
    void zatrzymajSerwer();
    void wyslijDane(quint8 typ, const QByteArray &dane);
    bool isListening() const;

signals:
    void klientPodlaczony();
    void klientOdlaczony();
    void odebranoDane(quint8 typ, QByteArray dane);

private slots:
    void obsluzNowePolaczenie();
    void czytajDaneZGniazda();
    void obsluzRozlaczenieKlienta();

private:
    QTcpServer *serwer;
    QTcpSocket *gniazdoKlienta;
    QByteArray bufor;
    const quint32 NAGLOWEK_SYNCHRONIZACJI = 0xABCD1234;
};

#endif
