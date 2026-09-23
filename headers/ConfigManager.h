#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QString>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QDateTime>
#include <QDebug>

#include <vector>

struct ConfigData {
    struct Main {
        double interwal_ms;
        int szerokosc_okna;
    } main;

    struct Regulator {
        QString typ;
        struct ONOFF {
            double u_on;
            double histereza;
        } onoff;
        struct PID {
            double k;
            double ti;
            double td;
            bool antiwindup;
            struct Limits {
                bool active;
                double min;
                double max;
            } limits;
            QString tryb_calki;
        } pid;
    } regulator;

    struct Model {
        std::vector<double> A;
        std::vector<double> B;
        int opoznienie;
        double szumy;
    } model;

    struct Signal {
        QString typ;
        struct Rect {
            double amp;
            double period;
            double offset;
            double fill;
            double activation_time;
        } rect;
        struct Sin {
            double amp;
            double period;
            double offset;
            double activation_time;
        } sin;
    } signal;
};

class ConfigManager {
public:
    static bool zapiszKonfiguracje(const ConfigData& config, const QString& directory);
    static ConfigData wczytajKonfiguracje(const QString& filePath);

    static QByteArray serializacja(const ConfigData& config);
    static ConfigData deserializacja(const QByteArray& dane);
};

#endif // CONFIGMANAGER_H
