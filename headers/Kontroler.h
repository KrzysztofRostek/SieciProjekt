#pragma once

#include "ProstyUAR.h"
#include "Generator.h"

class Kontroler {
public:
    Kontroler();
    ~Kontroler();

    void symuluj_krok(double krok_czasu);
    ProstyUAR* get_uar() const { return m_uar; }
    Generator* get_generator() const { return m_generator; }

    void reset();
    double get_aktualny_czas() const;
    double get_wyjscie() const;
    double get_sterowanie() const;
    double get_uchyb() const;
    double get_wartosc_zadana() const;

    double get_skladowa_p() const;
    double get_skladowa_i() const;
    double get_skladowa_d() const;

private:
    ProstyUAR* m_uar;
    Generator* m_generator;
};
