#ifndef SIECIOWKA_H
#define SIECIOWKA_H

#include <QDialog>
#include <QString>

namespace Ui {
class Sieciowka;
}

class Sieciowka : public QDialog
{
    Q_OBJECT

public:
    explicit Sieciowka(QWidget *parent = nullptr);
    ~Sieciowka();

    void ustawStanRozlaczony();
    void ustawStanOczekiwania();
    void ustawStanPolaczony(bool jakoKlient);

    void pokazDesynchronizacje(int diff);
    void pokazSynchronizacje();

    bool czyTaktowanieObustronne() const;
    bool czySerializacjaJson() const;

signals:
    void zadaniePolaczenia(QString ip, quint16 port);
    void zadanieSerwera(quint16 port);
    void zadanieRozlaczenia();
    void zmianaTrybuTaktowania(bool obustronne);
    void zmianaSerializacji(bool isJson);

private:
    Ui::Sieciowka *ui;

private slots:
    void on_btnPolacz_clicked();
    void on_btnSerwer_clicked();
    void on_btnRozlacz_clicked();
};

#endif // SIECIOWKA_H
