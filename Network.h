#ifndef NETWORK_H
#define NETWORK_H

#include <QByteArray>
#include <QDataStream>
#include <QIODevice>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDateTime>


const quint8 PKT_CONFIG     = 0x01;
const quint8 PKT_CONTROL    = 0x02;
const quint8 PKT_OUTPUT     = 0x03;
const quint8 PKT_COMMAND    = 0x04;
const quint8 PKT_SYNC        = 0x05;
const quint8 PKT_HEARTBEAT   = 0x06;

struct SyncPacket {
    int clientSampleId;      // ID próbki po stronie regulatora
    int serverSampleId;      // ID próbki po stronie obiektu
    double clientTime;       // Czas regulatora
    double serverTime;       // Czas obiektu
    double intervalMs;       // Interwał w ms
    bool running;            // Czy symulacja działa

    static QByteArray toBinary(int clientId, int serverId, double clientT, double serverT,
                               double interval, bool isRunning) {
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);
        stream << clientId << serverId << clientT << serverT << interval << isRunning;
        return data;
    }

    static bool fromBinary(const QByteArray &data, int &clientId, int &serverId,
                           double &clientT, double &serverT, double &interval, bool &isRunning) {
        QDataStream stream(data);
        stream >> clientId >> serverId >> clientT >> serverT >> interval >> isRunning;
        return stream.status() == QDataStream::Ok;
    }
};

struct ControlPacketJSON {
    static QByteArray toJson(int id, double t, double u, double zadana, double uchyb, double p, double i, double d) {
        QJsonObject obj;
        obj["id"] = id;
        obj["t"] = t;
        obj["u"] = u;
        obj["zadana"] = zadana;
        obj["uchyb"] = uchyb;
        obj["p"] = p;
        obj["i"] = i;
        obj["d"] = d;
        return QJsonDocument(obj).toJson();
    }

    static bool fromJson(const QByteArray &data, int &id, double &t, double &u, double &zadana, double &uchyb, double &p, double &i, double &d) {
        QJsonObject obj = QJsonDocument::fromJson(data).object();
        if (obj.contains("id") && obj.contains("t") && obj.contains("u")) {
            id = obj["id"].toInt();
            t = obj["t"].toDouble();
            u = obj["u"].toDouble();
            zadana = obj["zadana"].toDouble();
            uchyb = obj["uchyb"].toDouble();
            p = obj["p"].toDouble();
            i = obj["i"].toDouble();
            d = obj["d"].toDouble();
            return true;
        }
        return false;
    }
};

struct OutputPacketJSON {
    static QByteArray toJson(int id, double t, double y, double sterowanie) {
        QJsonObject obj;
        obj["id"] = id;
        obj["t"] = t;
        obj["y"] = y;
        obj["sterowanie"] = sterowanie;
        return QJsonDocument(obj).toJson();
    }

    static bool fromJson(const QByteArray &data, int &id, double &t, double &y, double &sterowanie) {
        QJsonObject obj = QJsonDocument::fromJson(data).object();
        if (obj.contains("id") && obj.contains("t") && obj.contains("y")) {
            id = obj["id"].toInt();
            t = obj["t"].toDouble();
            y = obj["y"].toDouble();
            sterowanie = obj["sterowanie"].toDouble();
            return true;
        }
        return false;
    }
};

struct ControlPacketBinary {
    static QByteArray toBinary(int id, double t, double u, double zadana, double uchyb, double p, double i, double d) {
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);
        stream << id << t << u << zadana << uchyb << p << i << d;
        return data;
    }

    static bool fromBinary(const QByteArray &data, int &id, double &t, double &u, double &zadana, double &uchyb, double &p, double &i, double &d) {
        QDataStream stream(data);
        stream >> id >> t >> u >> zadana >> uchyb >> p >> i >> d;
        return stream.status() == QDataStream::Ok;
    }
};

struct OutputPacketBinary {
    static QByteArray toBinary(int id, double t, double y, double sterowanie) {
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);
        stream << id << t << y << sterowanie;
        return data;
    }

    static bool fromBinary(const QByteArray &data, int &id, double &t, double &y, double &sterowanie) {
        QDataStream stream(data);
        stream >> id >> t >> y >> sterowanie;
        return stream.status() == QDataStream::Ok;
    }
};

#endif // NETWORK_H
