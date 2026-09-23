#pragma once
#include "Regulator.h"

class RegulatorONOFF : public Regulator { 
public:
    //Konstruktor z wartociami domylnymi
    RegulatorONOFF(double i_u_on = 1.0, double i_h = 0.1);

    virtual ~RegulatorONOFF() {} 

    // symulacja kroku 
    virtual double symuluj(double uchyb) override; 

    // Ustawienie nastaw regulatra
    void set_nastawy(double h, double u_on);

    // Reset regulatora
    virtual void reset();

    // Getters for Save/Load
    double get_hysteresis() const { return m_h; }
    double get_u_on() const { return m_u_on; }

private:
    double m_h;  // Poowa szerokoci histerezy 
    double m_u_on; // Warto sterowania w stanie ON 
    bool m_stan_on; // Aktualny stan regulatora 
};
