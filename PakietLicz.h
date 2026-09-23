#ifndef PAKIETLICZ_H
#define PAKIETLICZ_H

#include <QObject>
#include <QElapsedTimer>
#include <QTimer>

class PacketStats : public QObject {
    Q_OBJECT
public:
    explicit PacketStats(QObject *parent = nullptr);
    void packetSent();
    void packetReceived();
    void recordLatency(qint64 ms);
    int getPacketsPerSecond() const { return packetsPerSecond; }
    double getAvgLatency() const { return avgLatency; }
    int getLostPackets() const { return lostPackets; }
    void addLostPackages(int ilosc);

signals:
    void statsUpdated(int pps, double avgLat, int lost);
private slots:
    void updateStats();
private:
    int sentCount, recvCount, lostPackets;
    int packetsPerSecond;
    double avgLatency;
    QElapsedTimer timer;
    QTimer *statsTimer;
    QList<qint64> latencies;
};

#endif // PAKIETLICZ_H
