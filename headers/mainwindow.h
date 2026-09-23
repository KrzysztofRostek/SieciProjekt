#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QButtonGroup>
#include <QTimer>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <vector>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>
#include <QScrollArea>
#include <QHash>
#include <QDateTime>

#include "Kontroler.h"
#include "mytcpclient.h"
#include "mytcpserwer.h"
#include "Network.h"
#include "PakietLicz.h"
#include "sieciowka.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    enum TrybPracy {
        Stacjonarny,
        SieciowyKlientJednostronny,
        SieciowySerwerJednostronny,
        SieciowyKlientObustronny,
        SieciowySerwerObustronny
    };

    enum SerializationType {
        SERIAL_JSON,
        SERIAL_BINARY
    };

private slots:
    void on_ARX_model_button_clicked();
    void on_model_update(std::vector<double> A, std::vector<double> B, int k, double szum);
    void updateControllerParams();
    void on_start_stop_clicked();
    void on_reset_clicked();
    void updateSimulation();
    void on_save_button_2_clicked();
    void on_read_button_2_clicked();
    void on_add_syg_button_clicked();
    void on_del_syg_button_clicked();
    void zresetujSiec();
    void on_btnOtworzSieciowke_clicked();
    void ustawStanRozlaczony();
    void onTcpConnectedSerwer();
    void onTcpConnectedKlient();
    void onTcpDisconnected();
    void onClientDataReceived(quint8 typ, QByteArray data);
    void onServerDataReceived(quint8 typ, QByteArray data);
    void onNetworkTimer();
    void updateNetworkStats(int pps, double avgLat, int lost);
    void onServerTimer();
    void sendHeartbeat();
protected:
    void closeEvent(QCloseEvent *event) override;

private:
    Ui::MainWindow *ui;
    Sieciowka *oknoSieciowe;

    bool isManualDisconnect;

    Kontroler* kontroler;
    QTimer *networkTimer;
    QTimer *serverTimer;
    QTimer *heartbeatTimer;

    QButtonGroup *groupRegulator;
    QButtonGroup *groupIntegral;
    QButtonGroup *groupSignal;

    QTimer *timer;
    bool is_running;
    double time_step;

    QLineSeries *series_zadana;
    QLineSeries *series_regulowana;
    QLineSeries *series_uchyb;
    QLineSeries *series_sterowanie;
    QLineSeries *series_P;
    QLineSeries *series_I;
    QLineSeries *series_D;

    myTCPclient *klient;
    myTCPserwer *serwer;
    PacketStats *stats;

    int currentPacketId;
    QHash<int, QDateTime> pendingPackets;
    QHash<int, double> sentPacketTimes;
    double lastReceivedY;
    double lastReceivedU = 0.0;
    double lastReceivedZadana = 0.0;
    double lastReceivedUchyb = 0.0;
    double lastReceivedP = 0.0;
    double lastReceivedI = 0.0;
    double lastReceivedD = 0.0;
    double simulationTime;

    void updateTimeStep();
    void sendConfig();
    void updateCharts(double setpoint, double cv, double error, double control,
                      double p, double i, double d);

    TrybPracy aktualnyTryb;
    void ustawTrybGUI(TrybPracy nowyTryb);

    SerializationType currentSerialization;
    bool bilateralMode;
    int clientSampleCounter;
    int serverSampleCounter;
    int lastReceivedServerSample;
    int missedPackets;

    QByteArray serializeControl(int id, double t, double u, double zadana,
                                double uchyb, double p, double i, double d);
    bool deserializeControl(const QByteArray &data, int &id, double &t, double &u,
                            double &zadana, double &uchyb, double &p, double &i, double &d);
    QByteArray serializeOutput(int id, double t, double y, double sterowanie);
    bool deserializeOutput(const QByteArray &data, int &id, double &t, double &y, double &sterowanie);
    void sendSyncPacket();
    void processSyncPacket(const QByteArray &data);
    void checkSynchronization();

    QLabel* netLampka;
    int consecutiveMissedPackets = 0;
};

#endif // MAINWINDOW_H
