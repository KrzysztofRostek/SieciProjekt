#include "sieciowka.h"
#include "ui_sieciowka.h"

Sieciowka::Sieciowka(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Sieciowka)
{
    ui->setupUi(this);

    ui->poleIP->setText("127.0.0.1");
    ui->polePort->setValue(5555);

    connect(ui->btnPolacz, &QPushButton::clicked, this, &Sieciowka::on_btnPolacz_clicked);
    connect(ui->btnSerwer, &QPushButton::clicked, this, &Sieciowka::on_btnSerwer_clicked);
    connect(ui->btnRozlacz, &QPushButton::clicked, this, &Sieciowka::on_btnRozlacz_clicked);

    connect(ui->radioObustronne, &QRadioButton::toggled, this, &Sieciowka::zmianaTrybuTaktowania);
    connect(ui->radioJson, &QRadioButton::toggled, this, &Sieciowka::zmianaSerializacji);

    ustawStanRozlaczony();
}

Sieciowka::~Sieciowka()
{
    delete ui;
}


void Sieciowka::on_btnPolacz_clicked()
{
    emit zadaniePolaczenia(ui->poleIP->text(), ui->polePort->value());
}

void Sieciowka::on_btnSerwer_clicked()
{
    emit zadanieSerwera(ui->polePort->value());
}

void Sieciowka::on_btnRozlacz_clicked()
{
    emit zadanieRozlaczenia();
}

bool Sieciowka::czyTaktowanieObustronne() const
{
    return ui->radioObustronne->isChecked();
}

bool Sieciowka::czySerializacjaJson() const
{
    return ui->radioJson->isChecked();
}


void Sieciowka::ustawStanRozlaczony()
{
    ui->poleIP->setEnabled(true);
    ui->polePort->setEnabled(true);
    ui->btnPolacz->setEnabled(true);
    ui->btnSerwer->setEnabled(true);
    ui->btnRozlacz->setEnabled(false);

    ui->radioJson->setEnabled(true);
    ui->radioBinary->setEnabled(true);
    ui->radioJednostronne->setEnabled(true);
    ui->radioObustronne->setEnabled(true);

    ui->lblStatus->setText("Status: ROZLACZONO");
    ui->lblStatus->setStyleSheet("color: red; font-weight: bold;");
}

void Sieciowka::ustawStanOczekiwania()
{
    ui->btnSerwer->setEnabled(false);
    ui->btnPolacz->setEnabled(false);
    ui->btnRozlacz->setEnabled(true);

    ui->radioJednostronne->setEnabled(false);
    ui->radioObustronne->setEnabled(false);

    ui->lblStatus->setText("Status: OCZEKIWANIE...");
    ui->lblStatus->setStyleSheet("color: orange; font-weight: bold;");
}

void Sieciowka::ustawStanPolaczony(bool jakoKlient)
{
    ui->poleIP->setEnabled(false);
    ui->polePort->setEnabled(false);
    ui->btnPolacz->setEnabled(false);
    ui->btnSerwer->setEnabled(false);
    ui->btnRozlacz->setEnabled(true);

    ui->radioJson->setEnabled(false);
    ui->radioBinary->setEnabled(false);
    ui->radioJednostronne->setEnabled(false);
    ui->radioObustronne->setEnabled(false);

    ui->lblStatus->setText("Status: POLACZONO");
    ui->lblStatus->setStyleSheet("color: green; font-weight: bold;");
}

void Sieciowka::pokazDesynchronizacje(int diff)
{
    ui->lblStatus->setText(QString("Status: DESYNCHRONIZACJA (diff: %1)").arg(diff));
    ui->lblStatus->setStyleSheet("color: orange; font-weight: bold;");
}

void Sieciowka::pokazSynchronizacje()
{
    ui->lblStatus->setText("Status: POLACZONO (obustronne)");
    ui->lblStatus->setStyleSheet("color: green; font-weight: bold;");
}
