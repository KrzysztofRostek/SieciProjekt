#include "PakietLicz.h"
#include <QDebug>

PacketStats::PacketStats(QObject *parent)
    : QObject(parent), sentCount(0), recvCount(0), lostPackets(0),
    packetsPerSecond(0), avgLatency(0.0)
{
    statsTimer = new QTimer(this);
    connect(statsTimer, &QTimer::timeout, this, &PacketStats::updateStats);
    statsTimer->start(1000);
    timer.start();
}

void PacketStats::packetSent()
{
    sentCount++;
}

void PacketStats::packetReceived()
{
    recvCount++;
}

void PacketStats::recordLatency(qint64 ms)
{
    latencies.append(ms);
}

void PacketStats::updateStats()
{
    packetsPerSecond = recvCount;
    if (!latencies.isEmpty()) {
        qint64 sum = 0;
        for (qint64 l : latencies) sum += l;
        avgLatency = (double)sum / latencies.size();
        latencies.clear();
    }
    int diff = sentCount - recvCount;
    if (diff > 0) lostPackets += diff;
    else lostPackets = 0;
    sentCount = recvCount = 0;
    emit statsUpdated(packetsPerSecond, avgLatency, lostPackets);
}

void PacketStats::addLostPackages(int ilosc) {

    if (ilosc > 0) {
        lostPackets+=ilosc;
    }
    else lostPackets = 0;
}
