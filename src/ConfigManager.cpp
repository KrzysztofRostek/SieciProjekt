#include "ConfigManager.h"
#include <QJsonArray>

bool ConfigManager::zapiszKonfiguracje(const ConfigData &config, const QString &directory)
{
    QString czas = QDateTime::currentDateTime().toString("HH_mm_ss");
    QString nazwaPliku = QString("%1/config_data_%2.json").arg(directory).arg(czas);

    QFile plik(nazwaPliku);
    if (!plik.open(QIODevice::WriteOnly)) {
        qWarning() << "Nie udalo sie otworzyc pliku do zapisu:" << nazwaPliku;
        return false;
    }

    QJsonObject root;

    QJsonObject mainSettings;
    mainSettings["interwal_ms"] = config.main.interwal_ms;
    mainSettings["szerokosc_okna"] = config.main.szerokosc_okna;
    root["ustawienia_glowne"] = mainSettings;

    QJsonObject regulator;
    regulator["aktywny_regulator"] = config.regulator.typ;

    QJsonObject reg_onoff;
    reg_onoff["sterowanie_uon"] = config.regulator.onoff.u_on;
    reg_onoff["histereza"] = config.regulator.onoff.histereza;
    regulator["regulator_on_off"] = reg_onoff;

    QJsonObject reg_pid;
    reg_pid["wzmocnienie"] = config.regulator.pid.k;
    reg_pid["stala_calkowania"] = config.regulator.pid.ti;
    reg_pid["stala_rozniczkowania"] = config.regulator.pid.td;
    reg_pid["anti_windup"] = config.regulator.pid.antiwindup;
    reg_pid["tryb_pracy_calki"] = config.regulator.pid.tryb_calki;

    QJsonObject limits;
    limits["aktywne"] = config.regulator.pid.limits.active;
    limits["dolne"] = config.regulator.pid.limits.min;
    limits["gorne"] = config.regulator.pid.limits.max;
    reg_pid["ograniczenia_sterowania"] = limits;

    regulator["pid"] = reg_pid;
    root["regulator"] = regulator;

    QJsonObject model_arx;
    QJsonArray a_arr, b_arr;
    for (double v : config.model.A)
        a_arr.append(v);
    for (double v : config.model.B)
        b_arr.append(v);

    model_arx["wspolczynniki_a"] = a_arr;
    model_arx["wspolczynniki_b"] = b_arr;
    model_arx["opoznienie_transportowe"] = config.model.opoznienie;
    model_arx["szumy"] = config.model.szumy;
    root["model_arx"] = model_arx;

    QJsonObject sygnaly;
    sygnaly["wybrany_typ"] = config.signal.typ;

    QJsonArray syg_prost_arr;
    QJsonObject syg_prost;
    syg_prost["amplituda"] = config.signal.rect.amp;
    syg_prost["okres"] = config.signal.rect.period;
    syg_prost["skladowa_stala"] = config.signal.rect.offset;
    syg_prost["wypelnienie"] = config.signal.rect.fill;
    syg_prost["czas_aktywacji"] = config.signal.rect.activation_time;
    syg_prost_arr.append(syg_prost);
    sygnaly["sygnaly_prostokatne"] = syg_prost_arr;

    QJsonArray syg_sin_arr;
    QJsonObject syg_sin;
    syg_sin["amplituda"] = config.signal.sin.amp;
    syg_sin["okres"] = config.signal.sin.period;
    syg_sin["skladowa_stala"] = config.signal.sin.offset;
    syg_sin["czas_aktywacji"] = config.signal.sin.activation_time;
    syg_sin_arr.append(syg_sin);
    sygnaly["sygnaly_sinusoidalne"] = syg_sin_arr;

    root["sygnaly"] = sygnaly;

    QJsonDocument doc(root);
    plik.write(doc.toJson());
    plik.close();

    qDebug() << "Zapisano konfiguracje do pliku:" << nazwaPliku;
    return true;
}

ConfigData ConfigManager::wczytajKonfiguracje(const QString &filePath)
{
    ConfigData config;
    // Default values if load fails
    config.main = {100.0, 100};

    QFile plik(filePath);
    if (!plik.open(QIODevice::ReadOnly)) {
        qWarning() << "Nie udalo sie otworzyc pliku do odczytu:" << filePath;
        return config;
    }

    QByteArray dane = plik.readAll();
    plik.close();

    QJsonDocument doc = QJsonDocument::fromJson(dane);
    if (!doc.isObject()) {
        return config;
    }

    QJsonObject root = doc.object();

    if (root.contains("ustawienia_glowne")) {
        QJsonObject m = root["ustawienia_glowne"].toObject();
        config.main.interwal_ms = m["interwal_ms"].toDouble();
        config.main.szerokosc_okna = m["szerokosc_okna"].toInt();
    }

    if (root.contains("regulator")) {
        QJsonObject r = root["regulator"].toObject();
        config.regulator.typ = r["aktywny_regulator"].toString();

        if (r.contains("regulator_on_off")) {
            QJsonObject onoff = r["regulator_on_off"].toObject();
            config.regulator.onoff.u_on = onoff["sterowanie_uon"].toDouble();
            config.regulator.onoff.histereza = onoff["histereza"].toDouble();
        }

        if (r.contains("pid")) {
            QJsonObject pid = r["pid"].toObject();
            config.regulator.pid.k = pid["wzmocnienie"].toDouble();
            config.regulator.pid.ti = pid["stala_calkowania"].toDouble();
            config.regulator.pid.td = pid["stala_rozniczkowania"].toDouble();
            config.regulator.pid.antiwindup = pid["anti_windup"].toBool();
            config.regulator.pid.tryb_calki = pid["tryb_pracy_calki"].toString();

            if (pid.contains("ograniczenia_sterowania")) {
                QJsonObject l = pid["ograniczenia_sterowania"].toObject();
                config.regulator.pid.limits.active = l["aktywne"].toBool();
                config.regulator.pid.limits.min = l["dolne"].toDouble();
                config.regulator.pid.limits.max = l["gorne"].toDouble();
            }
        }
    }

    if (root.contains("model_arx")) {
        QJsonObject arx = root["model_arx"].toObject();
        QJsonArray a = arx["wspolczynniki_a"].toArray();
        QJsonArray b = arx["wspolczynniki_b"].toArray();
        config.model.A.clear();
        config.model.B.clear();
        for (const auto &v : a)
            config.model.A.push_back(v.toDouble());
        for (const auto &v : b)
            config.model.B.push_back(v.toDouble());
        config.model.opoznienie = arx["opoznienie_transportowe"].toInt();
        config.model.szumy = arx["szumy"].toDouble();
    }

    if (root.contains("sygnaly")) {
        QJsonObject s = root["sygnaly"].toObject();
        config.signal.typ = s["wybrany_typ"].toString();

        if (s.contains("sygnaly_prostokatne")) {
            QJsonArray arr = s["sygnaly_prostokatne"].toArray();
            if (!arr.isEmpty()) {
                QJsonObject r = arr.first().toObject();
                config.signal.rect.amp = r["amplituda"].toDouble();
                config.signal.rect.period = r["okres"].toDouble();
                config.signal.rect.offset = r["skladowa_stala"].toDouble();
                config.signal.rect.fill = r["wypelnienie"].toDouble();
                config.signal.rect.activation_time = r["czas_aktywacji"].toDouble();
            }
        }
        if (s.contains("sygnaly_sinusoidalne")) {
            QJsonArray arr = s["sygnaly_sinusoidalne"].toArray();
            if (!arr.isEmpty()) {
                QJsonObject si = arr.first().toObject();
                config.signal.sin.amp = si["amplituda"].toDouble();
                config.signal.sin.period = si["okres"].toDouble();
                config.signal.sin.offset = si["skladowa_stala"].toDouble();
                config.signal.sin.activation_time = si["czas_aktywacji"].toDouble();
            }
        }
    }

    return config;
}
QByteArray ConfigManager::serializacja(const ConfigData &config)
{
    QJsonObject root;

    QJsonObject mainSettings;
    mainSettings["interwal_ms"] = config.main.interwal_ms;
    mainSettings["szerokosc_okna"] = config.main.szerokosc_okna;
    root["ustawienia_glowne"] = mainSettings;

    QJsonObject regulator;
    regulator["aktywny_regulator"] = config.regulator.typ;

    QJsonObject reg_pid;
    reg_pid["wzmocnienie"] = config.regulator.pid.k;
    reg_pid["stala_calkowania"] = config.regulator.pid.ti;
    reg_pid["stala_rozniczkowania"] = config.regulator.pid.td;
    regulator["pid"] = reg_pid;
    root["regulator"] = regulator;

    QJsonObject model_arx;
    QJsonArray a_arr, b_arr;
    for (double v : config.model.A) a_arr.append(v);
    for (double v : config.model.B) b_arr.append(v);

    model_arx["wspolczynniki_a"] = a_arr;
    model_arx["wspolczynniki_b"] = b_arr;
    model_arx["opoznienie_transportowe"] = config.model.opoznienie;
    root["model_arx"] = model_arx;

    QJsonDocument doc(root);
    return doc.toJson(QJsonDocument::Compact);
}

ConfigData ConfigManager::deserializacja(const QByteArray &dane)
{
    ConfigData config;

    config.main.interwal_ms = 100.0;
    config.main.szerokosc_okna = 10;

    QJsonDocument doc = QJsonDocument::fromJson(dane);
    if (!doc.isObject()) return config;

    QJsonObject root = doc.object();

    if (root.contains("ustawienia_glowne")) {
        QJsonObject m = root["ustawienia_glowne"].toObject();
        config.main.interwal_ms = m["interwal_ms"].toDouble();
        config.main.szerokosc_okna = m["szerokosc_okna"].toInt();
    }

    if (root.contains("regulator")) {

        QJsonObject r = root["regulator"].toObject();

        if (r.contains("pid")) {

            QJsonObject pid = r["pid"].toObject();
            config.regulator.pid.k = pid["wzmocnienie"].toDouble();
            config.regulator.pid.ti = pid["stala_calkowania"].toDouble();
            config.regulator.pid.td = pid["stala_rozniczkowania"].toDouble();
        }
    }

    if (root.contains("model_arx")) {

        QJsonObject arx = root["model_arx"].toObject();
        QJsonArray a = arx["wspolczynniki_a"].toArray();
        QJsonArray b = arx["wspolczynniki_b"].toArray();
        for (const auto &v : a) config.model.A.push_back(v.toDouble());
        for (const auto &v : b) config.model.B.push_back(v.toDouble());
        config.model.opoznienie = arx["opoznienie_transportowe"].toInt();
    }

    return config;
}
