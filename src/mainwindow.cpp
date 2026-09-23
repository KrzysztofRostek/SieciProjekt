#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "arx.h"
#include <QtCharts/QLineSeries>
#include <cmath>
#include <QFileDialog>
#include <QMessageBox>
#include "ConfigManager.h"
#include "Kontroler.h"
#include <QJsonArray>
#include <QJsonObject>
#include <QButtonGroup>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , aktualnyTryb(Stacjonarny)
    , klient(new myTCPclient(this))
    , serwer(new myTCPserwer(this))
    , lastReceivedY(0.0)
    , simulationTime(0.0)
{
    ui->setupUi(this);
    ui->interwal_doubleSpinBox_2->setKeyboardTracking(false);

    oknoSieciowe = new Sieciowka(this);

    updateTimeStep();

    serverTimer = new QTimer(this);
    connect(serverTimer, &QTimer::timeout, this, &MainWindow::onServerTimer);

    kontroler = new Kontroler();

    if (kontroler && kontroler->get_uar() && kontroler->get_uar()->get_model()) {
        auto* model = kontroler->get_uar()->get_model();
        model->set_wspolczynniki_a({ -0.4 });
        model->set_wspolczynniki_b({ 0.6 });
        model->set_opoznienie(1);
        model->set_odchylenie_standardowe(0.0);
    }
    ui->sqrt_syg_radioButton->setChecked(true);

    updateControllerParams();

    groupRegulator = new QButtonGroup(this);

    groupIntegral = new QButtonGroup(this);
    groupIntegral->addButton(ui->przed_radioButton);
    groupIntegral->addButton(ui->w_sumie_radioButton);

    groupSignal = new QButtonGroup(this);
    groupSignal->addButton(ui->sqrt_syg_radioButton);
    groupSignal->addButton(ui->sin_syg_radioButton);

    ui->fill_square_doubleSpinBox_2->setRange(0, 100);
    is_running = false;
    time_step = 0.1;

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateSimulation);

    networkTimer = new QTimer(this);
    connect(networkTimer, &QTimer::timeout, this, &MainWindow::onNetworkTimer);

    serverTimer = new QTimer(this);
    connect(serverTimer, &QTimer::timeout, this, &MainWindow::onServerTimer);

    series_zadana = new QLineSeries(); series_zadana->setName("Zadana");
    series_regulowana = new QLineSeries(); series_regulowana->setName("Regulowana");
    QChart* chart1 = new QChart();
    chart1->addSeries(series_zadana); chart1->addSeries(series_regulowana);
    chart1->createDefaultAxes();
    chart1->axes(Qt::Horizontal).first()->setRange(0.0, ui->szer_okna_spinBox_2->value());
    chart1->axes(Qt::Vertical).first()->setRange(-10, 10);
    chart1->setTitle("Wartość zadana i regulowana");
    QChartView* view1 = new QChartView(chart1);
    view1->setRenderHint(QPainter::Antialiasing);
    ui->verticalLayout->addWidget(view1);

    series_uchyb = new QLineSeries();
    QChart* chart2 = new QChart();
    chart2->addSeries(series_uchyb);
    chart2->createDefaultAxes();
    chart2->axes(Qt::Horizontal).first()->setRange(0.0, ui->szer_okna_spinBox_2->value());
    chart2->axes(Qt::Vertical).first()->setRange(-10, 10);
    chart2->setTitle("Uchyb");
    QChartView* view2 = new QChartView(chart2);
    ui->horizontalLayout->addWidget(view2);

    series_sterowanie = new QLineSeries();
    QChart* chart3 = new QChart();
    chart3->addSeries(series_sterowanie);
    chart3->createDefaultAxes();
    chart3->axes(Qt::Horizontal).first()->setRange(0.0, ui->szer_okna_spinBox_2->value());
    chart3->axes(Qt::Vertical).first()->setRange(-10, 10);
    chart3->setTitle("Sterowanie");
    QChartView* view3 = new QChartView(chart3);
    ui->horizontalLayout->addWidget(view3);

    series_P = new QLineSeries(); series_P->setName("Proporcjonalny"); series_P->setColor(Qt::red);
    series_I = new QLineSeries(); series_I->setName("Całkujący"); series_I->setColor(Qt::green);
    series_D = new QLineSeries(); series_D->setName("Różniczkujący"); series_D->setColor(Qt::blue);
    QChart* chart4 = new QChart();
    chart4->addSeries(series_P); chart4->addSeries(series_I); chart4->addSeries(series_D);
    chart4->createDefaultAxes();
    chart4->axes(Qt::Horizontal).first()->setRange(0, ui->szer_okna_spinBox_2->value());
    chart4->axes(Qt::Vertical).first()->setRange(-10, 10);
    chart4->setTitle("Składowe sterowania PID");
    QChartView* view4 = new QChartView(chart4);
    ui->horizontalLayout->addWidget(view4);

    connect(ui->StartStop_button_2, &QPushButton::clicked, this, &MainWindow::on_start_stop_clicked);
    connect(ui->reset_button_2, &QPushButton::clicked, this, &MainWindow::on_reset_clicked);
    ui->przed_radioButton->setChecked(true);

    connect(ui->btnOtworzSieciowke, &QPushButton::clicked, this, &MainWindow::on_btnOtworzSieciowke_clicked);

    connect(ui->amp_square_doubleSpinBox_2, &QDoubleSpinBox::editingFinished, this, &MainWindow::updateControllerParams);
    connect(ui->period_square_doubleSpinBox_2, &QDoubleSpinBox::editingFinished, this, &MainWindow::updateControllerParams);
    connect(ui->sklad_stal_sqrt_doubleSpinBox, &QDoubleSpinBox::editingFinished, this, &MainWindow::updateControllerParams);
    connect(ui->fill_square_doubleSpinBox_2, &QDoubleSpinBox::editingFinished, this, &MainWindow::updateControllerParams);
    connect(ui->wzmocnienie_doubleSpinBox_3, &QDoubleSpinBox::editingFinished, this, &MainWindow::updateControllerParams);
    connect(ui->calka_doubleSpinBox_3, &QDoubleSpinBox::editingFinished, this, &MainWindow::updateControllerParams);
    connect(ui->rozniczka_doubleSpinBox_3, &QDoubleSpinBox::editingFinished, this, &MainWindow::updateControllerParams);
    connect(groupRegulator, QOverload<QAbstractButton*>::of(&QButtonGroup::buttonClicked), this, &MainWindow::updateControllerParams);
    connect(groupIntegral, QOverload<QAbstractButton*>::of(&QButtonGroup::buttonClicked), this, &MainWindow::updateControllerParams);
    connect(groupSignal, QOverload<QAbstractButton*>::of(&QButtonGroup::buttonClicked), this, &MainWindow::updateControllerParams);
    connect(ui->memory_reset_d_button_2, &QPushButton::clicked, this, &MainWindow::updateControllerParams);
    connect(ui->memory_reset_i_button_2, &QPushButton::clicked, this, &MainWindow::updateControllerParams);
    connect(ui->del_syg_button, &QPushButton::clicked, this, &MainWindow::updateControllerParams);
    connect(ui->interwal_doubleSpinBox_2, &QDoubleSpinBox::editingFinished, this, [this]() {
        updateTimeStep();
        if (is_running) {
            timer->setInterval(ui->interwal_doubleSpinBox_2->value());
        }
        if (networkTimer && networkTimer->isActive()) {
            networkTimer->setInterval(ui->interwal_doubleSpinBox_2->value());
        }

        sendConfig();
    });
    connect(ui->szer_okna_spinBox_2, &QSpinBox::editingFinished, this, [this]() {
        sendConfig();
    });

    stats = new PacketStats(this);

    connect(oknoSieciowe, &Sieciowka::zadaniePolaczenia, this, [this](QString ip, quint16 port) {
        klient->polaczZSerwerem(ip, port);
    });

    connect(oknoSieciowe, &Sieciowka::zadanieSerwera, this, [this](quint16 port) {
        if (serwer->uruchomSerwer(port)) {
            oknoSieciowe->ustawStanOczekiwania();
        }
    });

    connect(oknoSieciowe, &Sieciowka::zadanieRozlaczenia, this, [this]() {
        isManualDisconnect = true;
        klient->rozlaczZSerwerem();
        serwer->zatrzymajSerwer();
        ustawStanRozlaczony();
    });

    connect(oknoSieciowe, &Sieciowka::zmianaSerializacji, this, [this](bool isJson) {
        currentSerialization = isJson ? SERIAL_JSON : SERIAL_BINARY;
    });

    connect(oknoSieciowe, &Sieciowka::zmianaTrybuTaktowania, this, [this](bool obustronne) {
        bilateralMode = obustronne;
    });

    connect(klient, &myTCPclient::polaczono, this, &MainWindow::onTcpConnectedKlient);
    connect(serwer, &myTCPserwer::klientPodlaczony, this, &MainWindow::onTcpConnectedSerwer);
    connect(klient, &myTCPclient::rozlaczono, this, &MainWindow::onTcpDisconnected);
    connect(serwer, &myTCPserwer::klientOdlaczony, this, &MainWindow::onTcpDisconnected);

    connect(klient, &myTCPclient::odebranoDane, this, &MainWindow::onClientDataReceived);
    connect(serwer, &myTCPserwer::odebranoDane, this, &MainWindow::onServerDataReceived);

    connect(stats, &PacketStats::statsUpdated, this, &MainWindow::updateNetworkStats);

    currentPacketId = 0;
    lastReceivedY = 0.0;
    currentSerialization = SERIAL_JSON;
    bilateralMode = oknoSieciowe->czyTaktowanieObustronne();
    clientSampleCounter = 0;
    serverSampleCounter = 0;
    lastReceivedServerSample = -1;
    missedPackets = 0;

    isManualDisconnect = false;

    netLampka = new QLabel(this);
    netLampka->setFixedSize(16, 16);
    ui->horizontalLayout_4->insertWidget(0, netLampka);
    netLampka->setStyleSheet("background-color: #888888; border-radius: 8px; border: 1px solid #555555;");

    ustawStanRozlaczony();
    ustawTrybGUI(Stacjonarny);
    updateControllerParams();

    heartbeatTimer = new QTimer(this);
    connect(heartbeatTimer, &QTimer::timeout, this, &MainWindow::sendHeartbeat);
}

MainWindow::~MainWindow()
{
    delete kontroler;
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    QSettings settings("PK", "ARX");
    settings.clear();
    event->accept();
}

void MainWindow::on_btnOtworzSieciowke_clicked()
{
    oknoSieciowe->show();
}

void MainWindow::on_ARX_model_button_clicked()
{
    ARX* secWindow = new ARX(this);
    if (kontroler && kontroler->get_uar() && kontroler->get_uar()->get_model()) {
        secWindow->set_parametry(
            kontroler->get_uar()->get_model()->get_wspolczynniki_a(),
            kontroler->get_uar()->get_model()->get_wspolczynniki_b(),
            kontroler->get_uar()->get_model()->get_opoznienie(),
            kontroler->get_uar()->get_model()->get_odchylenie()
            );
    }
    connect(secWindow, &ARX::s_update_model_params, this, &MainWindow::on_model_update);
    secWindow->show();
}

void MainWindow::on_model_update(std::vector<double> A, std::vector<double> B, int k, double szum)
{
    if (kontroler && kontroler->get_uar() && kontroler->get_uar()->get_model()) {
        kontroler->get_uar()->get_model()->set_wspolczynniki_a(A);
        kontroler->get_uar()->get_model()->set_wspolczynniki_b(B);
        kontroler->get_uar()->get_model()->set_opoznienie(k);
        kontroler->get_uar()->get_model()->set_odchylenie_standardowe(szum);
    }
}


void MainWindow::on_start_stop_clicked()
{
    bool isNetworkActive =
        (aktualnyTryb == SieciowyKlientJednostronny && klient && klient->isConnected()) ||
        (aktualnyTryb == SieciowySerwerJednostronny && serwer && serwer->isListening()) ||
        (aktualnyTryb == SieciowyKlientObustronny && klient && klient->isConnected()) ||
        (aktualnyTryb == SieciowySerwerObustronny && serwer && serwer->isListening());

    if (isNetworkActive)
    {
        if (aktualnyTryb == SieciowyKlientJednostronny || aktualnyTryb == SieciowyKlientObustronny) {
            if (networkTimer->isActive()) {
                networkTimer->stop();
                if (bilateralMode) sendSyncPacket();
            } else {
                updateTimeStep();
                double intervalMs = ui->interwal_doubleSpinBox_2->value();
                networkTimer->start(static_cast<int>(intervalMs));
                if (bilateralMode) sendSyncPacket();
            }
        }

        if (aktualnyTryb == SieciowySerwerObustronny) {
            if (serverTimer->isActive()) {
                serverTimer->stop();
            } else {
                updateTimeStep();
                double intervalMs = ui->interwal_doubleSpinBox_2->value();
                serverTimer->start(static_cast<int>(intervalMs));
            }
        }
        return;
    }

    if (is_running)
    {
        timer->stop();
        is_running = false;
    }
    else
    {
        if (!series_zadana || !series_regulowana) return;

        if (series_zadana->count() == 0)
        {
            double t0 = 0.0;

            series_zadana->append(t0, 0.0);
            series_regulowana->append(t0, 0.0);
            series_uchyb->append(t0, 0.0);
            series_sterowanie->append(t0, 0.0);

            series_P->append(t0, 0.0);
            series_I->append(t0, 0.0);
            series_D->append(t0, 0.0);
        }

        double intervalMs =
            ui->interwal_doubleSpinBox_2->value();

        time_step = intervalMs / 1000.0;

        timer->start(static_cast<int>(intervalMs));

        is_running = true;
    }
}

void MainWindow::on_reset_clicked()
{
    if (timer) timer->stop();
    if (networkTimer) networkTimer->stop();
    if (serverTimer) serverTimer->stop();
    if (heartbeatTimer) heartbeatTimer->stop();

    is_running = false;

    if (kontroler) kontroler->reset();

    QByteArray emptyData;
    if (aktualnyTryb == SieciowyKlientJednostronny ||
        aktualnyTryb == SieciowyKlientObustronny) {
        if (klient && klient->isConnected())
            klient->wyslijDane(PKT_COMMAND, emptyData);
    } else if (aktualnyTryb == SieciowySerwerJednostronny ||
               aktualnyTryb == SieciowySerwerObustronny) {
        if (serwer && serwer->isListening())
            serwer->wyslijDane(PKT_COMMAND, emptyData);
    }

    simulationTime = 0.0;
    clientSampleCounter = 0;
    serverSampleCounter = 0;
    lastReceivedServerSample = -1;
    currentPacketId = 0;
    lastReceivedY = 0.0;

    if (series_zadana) series_zadana->clear();
    if (series_regulowana) series_regulowana->clear();
    if (series_uchyb) series_uchyb->clear();
    if (series_sterowanie) series_sterowanie->clear();
    if (series_P) series_P->clear();
    if (series_I) series_I->clear();
    if (series_D) series_D->clear();

    sentPacketTimes.clear();
}

void MainWindow::updateSimulation()
{
    if (!kontroler) return;
    if (!series_zadana || !series_regulowana || !series_uchyb ||
        !series_sterowanie || !series_P || !series_I || !series_D) {
        return;
    }

    kontroler->symuluj_krok(time_step);
    double setpoint = kontroler->get_wartosc_zadana();
    double y = kontroler->get_wyjscie();
    double e = kontroler->get_uchyb();
    double u = kontroler->get_sterowanie();
    double sk_p = kontroler->get_skladowa_p();
    double sk_i = kontroler->get_skladowa_i();
    double sk_d = kontroler->get_skladowa_d();
    updateCharts(setpoint, y, e, u, sk_p, sk_i, sk_d);
}

void MainWindow::updateTimeStep()
{
    double intervalMs = ui->interwal_doubleSpinBox_2->value();
    if (intervalMs <= 0) {
        intervalMs = 10.0;
    }
    time_step = intervalMs / 1000.0;
}

void MainWindow::updateControllerParams()
{
    if (!kontroler) return;
    ProstyUAR* uar = kontroler->get_uar();
    Generator* gen = kontroler->get_generator();

    if (ui->sin_syg_radioButton->isChecked()) {
        gen->ustaw_typ(Generator::SINUS);
        gen->ustaw_parametry_sinus(
            ui->amp_sinus_doubleSpinBox_2->value(),
            ui->period_sinus_doubleSpinBox_2->value(),
            ui->sklad_stal_sin_doubleSpinBox->value()
            );
    }
    else if (ui->sqrt_syg_radioButton->isChecked()) {
        gen->ustaw_typ(Generator::PROSTOKAT);
        gen->ustaw_parametry_prostokat(
            ui->amp_square_doubleSpinBox_2->value(),
            ui->period_square_doubleSpinBox_2->value(),
            ui->sklad_stal_sqrt_doubleSpinBox->value(),
            ui->fill_square_doubleSpinBox_2->value()
            );
    }
    else {
        gen->ustaw_typ(Generator::BRAK);
    }

        uar->ustaw_aktywny_regulator(ProstyUAR::TypRegulatora::PID);
        double k = ui->wzmocnienie_doubleSpinBox_3->value();
        double ti = ui->calka_doubleSpinBox_3->value();
        double td = ui->rozniczka_doubleSpinBox_3->value();
        RegulatorPID* pid = uar->get_pid();
        if (pid) {
            pid->set_nastawy(k, ti, td);
            if (ui->przed_radioButton->isChecked())
                pid->set_tryb_calki(RegulatorPID::tryb_calki::stala_przed_suma);
            else if (ui->w_sumie_radioButton->isChecked())
                pid->set_tryb_calki(RegulatorPID::tryb_calki::StalaPodSuma);
        }

}

void MainWindow::on_add_syg_button_clicked()
{
    if (ui->sin_syg_radioButton->isChecked()) {
        double amp = ui->amp_sinus_doubleSpinBox_2->value();
        double okres = ui->period_sinus_doubleSpinBox_2->value();
        double skladowa = ui->sklad_stal_sin_doubleSpinBox->value();
        QString sin_opis = QString("Amp=%1, T=%2, Stała=%3").arg(amp).arg(okres).arg(skladowa);
        ui->sygn_sinus_l_2->append(sin_opis);
    }
    else if (ui->sqrt_syg_radioButton->isChecked()) {
        double amp = ui->amp_square_doubleSpinBox_2->value();
        double okres = ui->period_square_doubleSpinBox_2->value();
        double skladowa = ui->sklad_stal_sqrt_doubleSpinBox->value();
        double fill = ui->fill_square_doubleSpinBox_2->value();
        QString sqrt_opis = QString("Amp=%1, T=%2, Stała=%3, Wyp=%4").arg(amp).arg(okres).arg(skladowa).arg(fill);
        ui->sygn_square_l_2->append(sqrt_opis);
    }
}

void MainWindow::on_del_syg_button_clicked()
{
    if (ui->sin_syg_radioButton->isChecked()) {
        ui->amp_sinus_doubleSpinBox_2->setValue(0.0);
        ui->period_sinus_doubleSpinBox_2->setValue(0.0);
        ui->sklad_stal_sin_doubleSpinBox->setValue(0.0);
    }
    else if (ui->sqrt_syg_radioButton->isChecked()) {
        ui->amp_square_doubleSpinBox_2->setValue(0.0);
        ui->sklad_stal_sqrt_doubleSpinBox->setValue(0.0);
        ui->fill_square_doubleSpinBox_2->setValue(0.5);
        ui->period_square_doubleSpinBox_2->setValue(0.0);
    }
}

void MainWindow::on_save_button_2_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Wybierz katalog zapisu", ".",
                                                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (dir.isEmpty()) return;
    ConfigData config;
    config.main.interwal_ms = ui->interwal_doubleSpinBox_2->value();
    config.main.szerokosc_okna = ui->szer_okna_spinBox_2->value();
    config.regulator.typ = "PID";

    config.regulator.pid.k = ui->wzmocnienie_doubleSpinBox_3->value();
    config.regulator.pid.ti = ui->calka_doubleSpinBox_3->value();
    config.regulator.pid.td = ui->rozniczka_doubleSpinBox_3->value();

    if (ui->przed_radioButton->isChecked()) config.regulator.pid.tryb_calki = "stala_przed_suma";
    else if (ui->w_sumie_radioButton->isChecked()) config.regulator.pid.tryb_calki = "stala_w_sumie";
    else config.regulator.pid.tryb_calki = "nieznany";

    ProstyUAR* uar = kontroler ? kontroler->get_uar() : nullptr;
    if (uar && uar->get_model()) {
        config.model.A = uar->get_model()->get_wspolczynniki_a();
        config.model.B = uar->get_model()->get_wspolczynniki_b();
        config.model.opoznienie = uar->get_model()->get_opoznienie();
        config.model.szumy = uar->get_model()->get_odchylenie();
    }

    if (ui->sin_syg_radioButton->isChecked()) config.signal.typ = "sinusoidalny";
    else config.signal.typ = "prostokatny";

    config.signal.rect.amp = ui->amp_square_doubleSpinBox_2->value();
    config.signal.rect.period = ui->period_square_doubleSpinBox_2->value();
    config.signal.rect.offset = ui->sklad_stal_sqrt_doubleSpinBox->value();
    config.signal.rect.fill = ui->fill_square_doubleSpinBox_2->value();

    config.signal.sin.amp = ui->amp_sinus_doubleSpinBox_2->value();
    config.signal.sin.period = ui->period_sinus_doubleSpinBox_2->value();
    config.signal.sin.offset = ui->sklad_stal_sin_doubleSpinBox->value();

    if (ConfigManager::zapiszKonfiguracje(config, dir))
        QMessageBox::information(this, "Sukces", "Konfiguracja zapisana pomyslnie!");
    else
        QMessageBox::critical(this, "Blad", "Nie udalo sie zapisac konfiguracji.");
}

void MainWindow::on_read_button_2_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Wybierz plik konfiguracji", ".", "Pliki JSON (*.json)");
    if (fileName.isEmpty()) return;
    ConfigData config = ConfigManager::wczytajKonfiguracje(fileName);

    ui->interwal_doubleSpinBox_2->setValue(config.main.interwal_ms);
    ui->szer_okna_spinBox_2->setValue(config.main.szerokosc_okna);

    ui->wzmocnienie_doubleSpinBox_3->setValue(config.regulator.pid.k);
    ui->calka_doubleSpinBox_3->setValue(config.regulator.pid.ti);
    ui->rozniczka_doubleSpinBox_3->setValue(config.regulator.pid.td);

    if (config.regulator.pid.tryb_calki == "stala_przed_suma") ui->przed_radioButton->setChecked(true);
    else if (config.regulator.pid.tryb_calki == "stala_w_sumie") ui->w_sumie_radioButton->setChecked(true);

    if (kontroler && kontroler->get_uar() && kontroler->get_uar()->get_model()) {
        kontroler->get_uar()->get_model()->set_wspolczynniki_a(config.model.A);
        kontroler->get_uar()->get_model()->set_wspolczynniki_b(config.model.B);
        kontroler->get_uar()->get_model()->set_odchylenie_standardowe(config.model.szumy);
    }

    if (config.signal.typ == "sinusoidalny") ui->sin_syg_radioButton->setChecked(true);
    else ui->sqrt_syg_radioButton->setChecked(true);

    ui->amp_square_doubleSpinBox_2->setValue(config.signal.rect.amp);
    ui->period_square_doubleSpinBox_2->setValue(config.signal.rect.period);
    ui->sklad_stal_sqrt_doubleSpinBox->setValue(config.signal.rect.offset);
    ui->fill_square_doubleSpinBox_2->setValue(config.signal.rect.fill);

    ui->amp_sinus_doubleSpinBox_2->setValue(config.signal.sin.amp);
    ui->period_sinus_doubleSpinBox_2->setValue(config.signal.sin.period);
    ui->sklad_stal_sin_doubleSpinBox->setValue(config.signal.sin.offset);

    QMessageBox::information(this, "Sukces", "Konfiguracja wczytana pomyslnie!");
    updateControllerParams();
}

void MainWindow::updateCharts(double setpoint, double cv, double error, double control,
                              double p, double i, double d)
{
    if (!kontroler) return;
    if (!series_zadana || !series_regulowana || !series_uchyb ||
        !series_sterowanie || !series_P || !series_I || !series_D) {
        return;
    }

    double current_time;

    bool networkMode =
        (aktualnyTryb == SieciowyKlientJednostronny && klient && klient->isConnected()) ||
        (aktualnyTryb == SieciowySerwerJednostronny && serwer && serwer->isListening()) ||
        (aktualnyTryb == SieciowyKlientObustronny && klient && klient->isConnected()) ||
        (aktualnyTryb == SieciowySerwerObustronny && serwer && serwer->isListening()); //

    if (networkMode)
        current_time = simulationTime;
    else
        current_time = kontroler->get_aktualny_czas();

    series_zadana->append(current_time, setpoint);
    series_regulowana->append(current_time, cv);
    series_uchyb->append(current_time, error);
    series_sterowanie->append(current_time, control);
    series_P->append(current_time, p);
    series_I->append(current_time, i);
    series_D->append(current_time, d);

    double x_max_limit = ui->szer_okna_spinBox_2->value();
    double x_min = 0.0;
    double x_max = x_max_limit;

    if (current_time > x_max_limit)
    {
        x_max = current_time;
        x_min = current_time - x_max_limit;
    }

    double zapas_pamieci = ui->szer_okna_spinBox_2->maximum();
    if (zapas_pamieci < 50.0) zapas_pamieci = 50.0;
    double próg_usuwania = current_time - zapas_pamieci;

    auto usunNiewidocznePunkty = [próg_usuwania](QLineSeries* s)
    {
        if (!s) return;

        QList<QPointF> points = s->points();
        int pointsToRemove = 0;

        for (const QPointF& pt : points) {
            if (pt.x() < próg_usuwania)
                pointsToRemove++;
            else
                break;
        }

        if (pointsToRemove > 0) {
            points.erase(points.begin(), points.begin() + pointsToRemove);
            s->replace(points);
        }
    };

    usunNiewidocznePunkty(series_zadana);
    usunNiewidocznePunkty(series_regulowana);
    usunNiewidocznePunkty(series_uchyb);
    usunNiewidocznePunkty(series_sterowanie);
    usunNiewidocznePunkty(series_P);
    usunNiewidocznePunkty(series_I);
    usunNiewidocznePunkty(series_D);

    auto setAxisX = [x_min, x_max](QLineSeries* s)
    {
        if (s && s->chart() && !s->chart()->axes(Qt::Horizontal).isEmpty())
        {
            s->chart()->axes(Qt::Horizontal).first()->setRange(x_min, x_max);
        }
    };

    setAxisX(series_zadana);
    setAxisX(series_uchyb);
    setAxisX(series_sterowanie);
    setAxisX(series_P);



    auto dopasujSkalePionowa = [x_min](QList<QLineSeries*> listaSerii)
    {
        if (listaSerii.isEmpty()) return;

        double minWartosc = std::numeric_limits<double>::max();
        double maxWartosc = std::numeric_limits<double>::lowest();
        bool czyZnalezionoJakikolwiekPunkt = false;

        for (QLineSeries* seria : listaSerii)
        {
            if (!seria) continue;

            QList<QPointF> punkty = seria->points();
            for (const QPointF& pt : punkty)
            {
                if (pt.x() >= x_min)
                {
                    czyZnalezionoJakikolwiekPunkt = true;
                    if (pt.y() < minWartosc) minWartosc = pt.y();
                    if (pt.y() > maxWartosc) maxWartosc = pt.y();
                }
            }
        }

        if (czyZnalezionoJakikolwiekPunkt)
        {
            double rozpietosc = maxWartosc - minWartosc;
            QChart* chart = listaSerii.first()->chart();

            if (chart && !chart->axes(Qt::Vertical).isEmpty())
            {
                QAbstractAxis* osY = chart->axes(Qt::Vertical).first();

                if (rozpietosc < 0.001)
                {
                    osY->setRange(minWartosc - 0.5, maxWartosc + 0.5);
                }
                else
                {
                    double margines = rozpietosc * 0.15;
                    if (margines < 0.1) margines = 0.1;
                    osY->setRange(minWartosc - margines, maxWartosc + margines);
                }
            }
        }
    };

    static int tickCounter = 0;
    tickCounter++;

    if (tickCounter >= 10) {
        dopasujSkalePionowa({ series_zadana, series_regulowana });
        dopasujSkalePionowa({ series_uchyb });
        dopasujSkalePionowa({ series_sterowanie });
        dopasujSkalePionowa({ series_P, series_I, series_D });
        tickCounter = 0;
    }
}


void MainWindow::ustawTrybGUI(TrybPracy nowyTryb)
{
    aktualnyTryb = nowyTryb;
    bilateralMode = (nowyTryb == SieciowyKlientObustronny || nowyTryb == SieciowySerwerObustronny);

    if (bilateralMode) {
        if (nowyTryb == SieciowyKlientObustronny) {
            if (serverTimer) serverTimer->stop();
            if (networkTimer && !networkTimer->isActive()) {
                double intervalMs = ui->interwal_doubleSpinBox_2->value();
            }
        } else if (nowyTryb == SieciowySerwerObustronny) {
            if (networkTimer) networkTimer->stop();
            if (serverTimer && !serverTimer->isActive()) {
                double intervalMs = ui->interwal_doubleSpinBox_2->value();
            }
        }
    }

    bool stacjonarny = (nowyTryb == Stacjonarny);
    bool klientMode = (nowyTryb == SieciowyKlientJednostronny || nowyTryb == SieciowyKlientObustronny);
    bool serwerMode = (nowyTryb == SieciowySerwerJednostronny || nowyTryb == SieciowySerwerObustronny);

    if (klientMode) {
        ui->przed_radioButton->setEnabled(true);
        ui->w_sumie_radioButton->setEnabled(true);
        ui->sqrt_syg_radioButton->setEnabled(true);
        ui->sin_syg_radioButton->setEnabled(true);
        ui->amp_square_doubleSpinBox_2->setEnabled(true);
        ui->period_square_doubleSpinBox_2->setEnabled(true);
        ui->sklad_stal_sqrt_doubleSpinBox->setEnabled(true);
        ui->fill_square_doubleSpinBox_2->setEnabled(true);
        ui->amp_sinus_doubleSpinBox_2->setEnabled(true);
        ui->period_sinus_doubleSpinBox_2->setEnabled(true);
        ui->sklad_stal_sin_doubleSpinBox->setEnabled(true);
        ui->add_syg_button->setEnabled(true);
        ui->del_syg_button->setEnabled(true);
        ui->sygn_sinus_l_2->setEnabled(true);
        ui->sygn_square_l_2->setEnabled(true);
        ui->ARX_model_button->setEnabled(false);
        ui->memory_reset_d_button_2->setEnabled(true);
        ui->memory_reset_i_button_2->setEnabled(true);
        ui->wzmocnienie_doubleSpinBox_3->setEnabled(true);
        ui->calka_doubleSpinBox_3->setEnabled(true);
        ui->rozniczka_doubleSpinBox_3->setEnabled(true);
        ui->interwal_doubleSpinBox_2->setEnabled(true);
        ui->szer_okna_spinBox_2->setEnabled(true);
        ui->StartStop_button_2->setEnabled(true);
        ui->reset_button_2->setEnabled(true);

        if (timer->isActive()) timer->stop();
        is_running = false;
    }
    else if (serwerMode) {
        ui->przed_radioButton->setEnabled(false);
        ui->w_sumie_radioButton->setEnabled(false);
        ui->sqrt_syg_radioButton->setEnabled(false);
        ui->sin_syg_radioButton->setEnabled(false);
        ui->amp_square_doubleSpinBox_2->setEnabled(false);
        ui->period_square_doubleSpinBox_2->setEnabled(false);
        ui->sklad_stal_sqrt_doubleSpinBox->setEnabled(false);
        ui->fill_square_doubleSpinBox_2->setEnabled(false);
        ui->amp_sinus_doubleSpinBox_2->setEnabled(false);
        ui->period_sinus_doubleSpinBox_2->setEnabled(false);
        ui->sklad_stal_sin_doubleSpinBox->setEnabled(false);
        ui->add_syg_button->setEnabled(false);
        ui->del_syg_button->setEnabled(false);
        ui->sygn_sinus_l_2->setEnabled(false);
        ui->sygn_square_l_2->setEnabled(false);
        ui->ARX_model_button->setEnabled(true);
        ui->memory_reset_d_button_2->setEnabled(false);
        ui->memory_reset_i_button_2->setEnabled(false);
        ui->wzmocnienie_doubleSpinBox_3->setEnabled(false);
        ui->calka_doubleSpinBox_3->setEnabled(false);
        ui->rozniczka_doubleSpinBox_3->setEnabled(false);
        ui->interwal_doubleSpinBox_2->setEnabled(false);
        ui->szer_okna_spinBox_2->setEnabled(false);
        ui->StartStop_button_2->setEnabled(false);
        ui->reset_button_2->setEnabled(false);
        ui->sqrt_syg_radioButton->setChecked(true);

        if (timer->isActive()) timer->stop();
        is_running = false;
    }
    else if (stacjonarny) {
        ui->przed_radioButton->setEnabled(true);
        ui->w_sumie_radioButton->setEnabled(true);
        ui->sqrt_syg_radioButton->setEnabled(true);
        ui->sin_syg_radioButton->setEnabled(true);
        ui->amp_square_doubleSpinBox_2->setEnabled(true);
        ui->period_square_doubleSpinBox_2->setEnabled(true);
        ui->sklad_stal_sqrt_doubleSpinBox->setEnabled(true);
        ui->fill_square_doubleSpinBox_2->setEnabled(true);
        ui->amp_sinus_doubleSpinBox_2->setEnabled(true);
        ui->period_sinus_doubleSpinBox_2->setEnabled(true);
        ui->sklad_stal_sin_doubleSpinBox->setEnabled(true);
        ui->add_syg_button->setEnabled(true);
        ui->del_syg_button->setEnabled(true);
        ui->sygn_sinus_l_2->setEnabled(true);
        ui->sygn_square_l_2->setEnabled(true);
        ui->ARX_model_button->setEnabled(true);
        ui->memory_reset_d_button_2->setEnabled(true);
        ui->memory_reset_i_button_2->setEnabled(true);
        ui->wzmocnienie_doubleSpinBox_3->setEnabled(true);
        ui->calka_doubleSpinBox_3->setEnabled(true);
        ui->rozniczka_doubleSpinBox_3->setEnabled(true);
        ui->interwal_doubleSpinBox_2->setEnabled(true);
        ui->szer_okna_spinBox_2->setEnabled(true);
        ui->StartStop_button_2->setEnabled(true);
        ui->reset_button_2->setEnabled(true);
    }
}

void MainWindow::ustawStanRozlaczony()
{
    oknoSieciowe->ustawStanRozlaczony();
    ustawTrybGUI(Stacjonarny);
}

void MainWindow::onTcpConnectedKlient()
{
    oknoSieciowe->ustawStanPolaczony(true);
    consecutiveMissedPackets = 0;

    if (oknoSieciowe->czyTaktowanieObustronne()) {
        ustawTrybGUI(SieciowyKlientObustronny);
    } else {
        ustawTrybGUI(SieciowyKlientJednostronny);
    }

    if (timer->isActive()) timer->stop();
    is_running = false;

    updateTimeStep();
    simulationTime = 0.0;
    sendConfig();

    if (bilateralMode) {
        heartbeatTimer->start(1000);
    }
}

void MainWindow::onTcpConnectedSerwer()
{
    oknoSieciowe->ustawStanPolaczony(false);
    consecutiveMissedPackets = 0;

    if (oknoSieciowe->czyTaktowanieObustronne()) {
        ustawTrybGUI(SieciowySerwerObustronny);
    } else {
        ustawTrybGUI(SieciowySerwerJednostronny);
    }

    if (timer->isActive()) timer->stop();
    is_running = false;

    if (bilateralMode) {
        heartbeatTimer->start(1000);
    }
    updateTimeStep();
}

void MainWindow::onTcpDisconnected()
{
    bool wasRunning = false;
    if (aktualnyTryb == SieciowyKlientJednostronny || aktualnyTryb == SieciowyKlientObustronny) {
        wasRunning = (networkTimer && networkTimer->isActive());
    } else if (aktualnyTryb == SieciowySerwerObustronny) {
        wasRunning = (serverTimer && serverTimer->isActive());
    } else if (aktualnyTryb == SieciowySerwerJednostronny) {
        wasRunning = true;
    }

    ustawStanRozlaczony();

    if (wasRunning) {
        if (kontroler && kontroler->get_uar()) {
            kontroler->get_uar()->set_aktualny_czas(simulationTime);
        }

        updateControllerParams();

        is_running = true;
        updateTimeStep();
        double intervalMs = ui->interwal_doubleSpinBox_2->value();
        timer->start(static_cast<int>(intervalMs));
    } else {
        is_running = false;
    }

    if (networkTimer) networkTimer->stop();
    if (serverTimer) serverTimer->stop();
    if (heartbeatTimer) heartbeatTimer->stop();
    pendingPackets.clear();


    clientSampleCounter = 0;
    serverSampleCounter = 0;
    lastReceivedServerSample = -1;
    missedPackets = 0;
    currentPacketId = 0;
    simulationTime = 0.0;
    lastReceivedY = 0.0;

    if (series_zadana) series_zadana->clear();
    if (series_regulowana) series_regulowana->clear();
    if (series_uchyb) series_uchyb->clear();
    if (series_sterowanie) series_sterowanie->clear();
    if (series_P) series_P->clear();
    if (series_I) series_I->clear();
    if (series_D) series_D->clear();

    if (!isManualDisconnect) {
        QMessageBox::warning(this,
                             "Błąd połączenia",
                             "Połączenie sieciowe zostało zerwane z przyczyn zewnętrznych!\n\n"
                             "Aplikacja powraca do trybu stacjonarnego",
                             QMessageBox::Ok);
    }
    isManualDisconnect = false;

    sentPacketTimes.clear();
}

void MainWindow::onClientDataReceived(quint8 typ, QByteArray data)
{
    if (typ == PKT_OUTPUT) {
        int id; double t; double y; double sterowanie;

        if (deserializeOutput(data, id, t, y, sterowanie)) {

            consecutiveMissedPackets = 0;

            if (bilateralMode) {
                int lost = 0;

                if (lastReceivedServerSample != -1 && id > lastReceivedServerSample + 1) {
                    lost = id - lastReceivedServerSample - 1;
                }

                lastReceivedServerSample = id;
                serverSampleCounter = id;

                if (lost > 0) {
                    stats->addLostPackages(lost);
                } else {
                    stats->addLostPackages(0);
                }
            }

            stats->packetReceived();

            lastReceivedY = y;
        }
    }
    else if (typ == PKT_CONFIG) {
        ConfigData cfg = ConfigManager::deserializacja(data);
        ui->interwal_doubleSpinBox_2->setValue(cfg.main.interwal_ms);
        ui->szer_okna_spinBox_2->setValue(cfg.main.szerokosc_okna);
        updateTimeStep();
    }
    else if (typ == PKT_COMMAND) {
        if (kontroler) kontroler->reset();
        simulationTime = 0.0;
        clientSampleCounter = 0;
        serverSampleCounter = 0;
        lastReceivedServerSample = -1;
        currentPacketId = 0;
        lastReceivedY = 0.0;
        if (series_zadana) series_zadana->clear();
        if (series_regulowana) series_regulowana->clear();
        if (series_uchyb) series_uchyb->clear();
        if (series_sterowanie) series_sterowanie->clear();
        if (series_P) series_P->clear();
        if (series_I) series_I->clear();
        if (series_D) series_D->clear();
    }
    else if (typ == PKT_SYNC) {
        processSyncPacket(data);
    }
}

void MainWindow::onServerDataReceived(quint8 typ, QByteArray data)
{
    if (typ == PKT_CONTROL) {
        stats->packetReceived();
        int id; double t, sterowanie, zadana, uchyb, p, i, d;

        if (deserializeControl(data, id, t, sterowanie, zadana, uchyb, p, i, d)) {

            consecutiveMissedPackets = 0;

            if (bilateralMode) {
                int lost = 0;

                if (clientSampleCounter != 0 && id > clientSampleCounter + 1) {
                    lost = id - clientSampleCounter - 1;
                }

                clientSampleCounter = id;

                if (lost > 0) {
                    stats->addLostPackages(lost);
                } else {
                    stats->addLostPackages(0);
                }

                lastReceivedU = sterowanie;
                lastReceivedZadana = zadana;
                lastReceivedUchyb = uchyb;
                lastReceivedP = p;
                lastReceivedI = i;
                lastReceivedD = d;
            } else {
                simulationTime = t;
                double y = kontroler->get_uar()->get_model()->symuluj(sterowanie);

                QByteArray outData = serializeOutput(id, t, y, sterowanie);
                serwer->wyslijDane(PKT_OUTPUT, outData);
                stats->packetSent();

                updateCharts(zadana, y, uchyb, sterowanie, p, i, d);
            }
        }
    }
    else if (typ == PKT_CONFIG) {
        ConfigData cfg = ConfigManager::deserializacja(data);
        ui->interwal_doubleSpinBox_2->setValue(cfg.main.interwal_ms);
        ui->szer_okna_spinBox_2->setValue(cfg.main.szerokosc_okna);
        updateTimeStep();
    }
    else if (typ == PKT_COMMAND) {
        if (kontroler) kontroler->reset();

        if (serverTimer && serverTimer->isActive()) {
            serverTimer->stop();
        }
        is_running = false;
        lastReceivedU = 0.0;

        simulationTime = 0.0;
        serverSampleCounter = 0;
        lastReceivedY = 0.0;
        if (series_zadana) series_zadana->clear();
        if (series_regulowana) series_regulowana->clear();
        if (series_uchyb) series_uchyb->clear();
        if (series_sterowanie) series_sterowanie->clear();
        if (series_P) series_P->clear();
        if (series_I) series_I->clear();
        if (series_D) series_D->clear();
    }
    else if (typ == PKT_SYNC) {
        processSyncPacket(data);
    }
}


void MainWindow::sendConfig()
{
    ConfigData cfg;
    cfg.main.interwal_ms = ui->interwal_doubleSpinBox_2->value();
    cfg.main.szerokosc_okna = ui->szer_okna_spinBox_2->value();
    cfg.regulator.typ = "PID";

    cfg.regulator.pid.k = ui->wzmocnienie_doubleSpinBox_3->value();
    cfg.regulator.pid.ti = ui->calka_doubleSpinBox_3->value();
    cfg.regulator.pid.td = ui->rozniczka_doubleSpinBox_3->value();

    if (ui->przed_radioButton->isChecked()) cfg.regulator.pid.tryb_calki = "stala_przed_suma";
    else cfg.regulator.pid.tryb_calki = "stala_w_sumie";

    ProstyUAR* uar = kontroler ? kontroler->get_uar() : nullptr;
    if (uar && uar->get_model()) {
        cfg.model.A = uar->get_model()->get_wspolczynniki_a();
        cfg.model.B = uar->get_model()->get_wspolczynniki_b();
        cfg.model.opoznienie = uar->get_model()->get_opoznienie();
    }
    if (ui->sin_syg_radioButton->isChecked()) cfg.signal.typ = "sinusoidalny";
    else cfg.signal.typ = "prostokatny";

    QByteArray json = ConfigManager::serializacja(cfg);

    if (klient && klient->isConnected()) {
        klient->wyslijDane(PKT_CONFIG, json);
    }
    else if (serwer && serwer->isListening()) {
        serwer->wyslijDane(PKT_CONFIG, json);
    }
}

void MainWindow::onNetworkTimer()
{
    if (!kontroler || !klient) return;
    if (!series_zadana) return;

    if (aktualnyTryb != SieciowyKlientJednostronny && aktualnyTryb != SieciowyKlientObustronny)
        return;

    if (!klient->isConnected()) return;

    if (serverSampleCounter > 10){
        consecutiveMissedPackets++;
        if (consecutiveMissedPackets >= 4) {
            isManualDisconnect = true;
            zresetujSiec();
            ustawStanRozlaczony();
            QMessageBox::critical(this, "Błąd krytyczny", "Utracono 4 próbki z rzędu. Połączenie zerwane!");
            return;
        }
    }else
    {
        consecutiveMissedPackets = 0;
    }

    updateTimeStep();
    simulationTime += time_step;
    clientSampleCounter++;

    double setpoint = kontroler->get_generator()->oblicz_wartosc(simulationTime);

    double error = setpoint - lastReceivedY;

    double u = kontroler->get_uar()->get_pid()->symuluj(error);
    double p = kontroler->get_skladowa_p();
    double i = kontroler->get_skladowa_i();
    double d = kontroler->get_skladowa_d();

    updateCharts(setpoint, lastReceivedY, error, u, p, i, d);

    pendingPackets[clientSampleCounter] = QDateTime::currentDateTime();
    stats->packetSent();

    QByteArray data;
    if (bilateralMode) {
        data = serializeControl(clientSampleCounter, simulationTime, u, setpoint, error, p, i, d);
    } else {
        data = serializeControl(currentPacketId, simulationTime, u, setpoint, error, p, i, d);
    }

    klient->wyslijDane(PKT_CONTROL, data);

    if (!bilateralMode) currentPacketId++;

    if (bilateralMode) checkSynchronization();
}



void MainWindow::updateNetworkStats(int pps, double avgLat, int lost)
{
    ui->lblPPS->setText(QString("Pakiety/s: %1").arg(pps));
    ui->lblLatency->setText(QString("Opóźnienie śr.: %1 ms").arg(avgLat, 0, 'f', 1));
    ui->lblLost->setText(QString("Straty: %1").arg(lost));

    QString kolorLampki;
    QString kolorObramowania;


    if (lost > 5 || avgLat > 150.0) {
        kolorLampki = "#ff3333";
        kolorObramowania = "#aa0000";
    }
    else if (lost > 1 || avgLat > 80.0) {
        kolorLampki = "#ffcc00";
        kolorObramowania = "#b38f00";
    }
    else {
        kolorLampki = "#33cc33";
        kolorObramowania = "#228822";
    }
    if (pps == 0 && lost == 0 && avgLat == 0) {
        kolorLampki = "#888888";
        kolorObramowania = "#555555";
    }
    netLampka->setStyleSheet(QString(
                                 "background-color: %1; "
                                 "border-radius: 8px; "
                                 "border: 1px solid %2;"
                                 ).arg(kolorLampki).arg(kolorObramowania));
}

QByteArray MainWindow::serializeControl(int id, double t, double u, double zadana, double uchyb, double p, double i, double d) {
    if (currentSerialization == SERIAL_JSON) return ControlPacketJSON::toJson(id, t, u, zadana, uchyb, p, i, d);
    else return ControlPacketBinary::toBinary(id, t, u, zadana, uchyb, p, i, d);
}

bool MainWindow::deserializeControl(const QByteArray &data, int &id, double &t, double &u, double &zadana, double &uchyb, double &p, double &i, double &d) {
    if (currentSerialization == SERIAL_JSON) return ControlPacketJSON::fromJson(data, id, t, u, zadana, uchyb, p, i, d);
    else return ControlPacketBinary::fromBinary(data, id, t, u, zadana, uchyb, p, i, d);
}

QByteArray MainWindow::serializeOutput(int id, double t, double y, double sterowanie) {
    if (currentSerialization == SERIAL_JSON) return OutputPacketJSON::toJson(id, t, y, sterowanie);
    else return OutputPacketBinary::toBinary(id, t, y, sterowanie);
}

bool MainWindow::deserializeOutput(const QByteArray &data, int &id, double &t, double &y, double &sterowanie) {
    if (currentSerialization == SERIAL_JSON) return OutputPacketJSON::fromJson(data, id, t, y, sterowanie);
    else return OutputPacketBinary::fromBinary(data, id, t, y, sterowanie);
}

void MainWindow::sendSyncPacket()
{
    if (!bilateralMode) return;

    QByteArray data = SyncPacket::toBinary(
        clientSampleCounter,
        serverSampleCounter,
        simulationTime,
        kontroler->get_aktualny_czas(),
        ui->interwal_doubleSpinBox_2->value(),
        networkTimer->isActive()
        );

    if (aktualnyTryb == SieciowyKlientObustronny && klient->isConnected()) {
        klient->wyslijDane(PKT_SYNC, data);
    } else if (aktualnyTryb == SieciowySerwerObustronny && serwer && serwer->isListening()) {
        serwer->wyslijDane(PKT_SYNC, data);
    }
}

void MainWindow::processSyncPacket(const QByteArray &data)
{
    int clientId, serverId;
    double clientTime, serverTime, interval;
    bool running;

    if (SyncPacket::fromBinary(data, clientId, serverId, clientTime, serverTime, interval, running)) {
        if (aktualnyTryb == SieciowyKlientObustronny) {
            serverSampleCounter = serverId;
        } else {
            clientSampleCounter = clientId;
        }

        int diff = clientSampleCounter - serverSampleCounter;
        int absDiff = (diff > 0) ? diff : -diff;

        if (absDiff > 5) {
            oknoSieciowe->pokazDesynchronizacje(diff);
        } else {
            oknoSieciowe->pokazSynchronizacje();
        }

        if (aktualnyTryb == SieciowySerwerObustronny) {
            if (running && !serverTimer->isActive()) {
                serverTimer->start(static_cast<int>(interval));
                is_running = true;
            } else if (!running && serverTimer->isActive()) {
                serverTimer->stop();
                is_running = false;
            }
        }

        if (interval != ui->interwal_doubleSpinBox_2->value()) {
            ui->interwal_doubleSpinBox_2->setValue(interval);
            updateTimeStep();

            if (aktualnyTryb == SieciowySerwerObustronny && running && serverTimer) {
                serverTimer->setInterval(static_cast<int>(interval));
            }
            if (aktualnyTryb == SieciowyKlientObustronny && running && networkTimer) {
                networkTimer->setInterval(static_cast<int>(interval));
            }
        }
    }
}

void MainWindow::sendHeartbeat()
{
    if (!bilateralMode) return;
    sendSyncPacket();
}

void MainWindow::checkSynchronization()
{
    if (!bilateralMode) return;
    if (networkTimer->isActive() && lastReceivedServerSample < clientSampleCounter - 10) {
        qDebug() << "Duże opóźnienie! Ostatni odebrany:" << lastReceivedServerSample
                 << "Aktualny:" << clientSampleCounter;
    }
}

void MainWindow::onServerTimer()
{
    if (!kontroler || !serwer) return;
    if (!series_zadana) return;

    if (aktualnyTryb != SieciowySerwerObustronny) return;

    if (serverSampleCounter > 10){
        consecutiveMissedPackets++;
        if (consecutiveMissedPackets >= 4) {
            isManualDisconnect = true;
            zresetujSiec();
            ustawStanRozlaczony();
            QMessageBox::critical(this, "Błąd krytyczny", "Utracono 4 próbki z rzędu. Połączenie zerwane!");
            return;
        }
    }else
    {
        consecutiveMissedPackets = 0;
    }

    updateTimeStep();
    simulationTime += time_step;
    serverSampleCounter++;

    double y = kontroler->get_uar()->get_model()->symuluj(lastReceivedU);

    QByteArray outData = serializeOutput(serverSampleCounter, simulationTime, y, lastReceivedU);
    serwer->wyslijDane(PKT_OUTPUT, outData);
    stats->packetSent();

    updateCharts(lastReceivedZadana, y, lastReceivedUchyb, lastReceivedU,
                 lastReceivedP, lastReceivedI, lastReceivedD);

    if (bilateralMode) {
        sendSyncPacket();
    }
}
void MainWindow::zresetujSiec()
{
    if(serwer) serwer->zatrzymajSerwer();
    if(klient) klient->rozlaczZSerwerem();
    if(networkTimer) networkTimer->stop();
    if(serverTimer) serverTimer->stop();
}
