#ifndef GENERATOR_H
#define GENERATOR_H

#include <cmath>

class Generator {
public:
    enum TypSygnalu {
        SINUS,
        PROSTOKAT,
        BRAK
    };

    Generator();

    void ustaw_typ(TypSygnalu typ);
    
    void ustaw_parametry_sinus(double amplituda, double okres, double offset);

    void ustaw_parametry_prostokat(double amplituda, double okres, double offset, double wypelnienie);

    double oblicz_wartosc(double czas);
    void set_czas_aktywacji(double czas);

    TypSygnalu get_typ() const { return m_typ; }
    double get_amplituda() const { return m_amplituda; }
    double get_okres() const { return m_okres; }
    double get_offset() const { return m_offset; }
    double get_wypelnienie() const { return m_wypelnienie; }

private:
    TypSygnalu m_typ;
    
    double m_amplituda;
    double m_okres;
    double m_offset;
    double m_wypelnienie;
    double m_czas_aktywacji = 0.0;
};

#endif // GENERATOR_H
