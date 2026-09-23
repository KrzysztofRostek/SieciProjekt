#include "Generator.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Generator::Generator()
    : m_typ(BRAK)
    , m_amplituda(0.0)
    , m_okres(1.0)
    , m_offset(0.0)
    , m_wypelnienie(50.0)
{}

void Generator::ustaw_typ(TypSygnalu typ)
{
    m_typ = typ;
}

void Generator::ustaw_parametry_sinus(double amplituda, double okres, double offset)
{
    m_amplituda = amplituda;
    m_okres = okres;
    m_offset = offset;
}

void Generator::ustaw_parametry_prostokat(double amplituda,
                                          double okres,
                                          double offset,
                                          double wypelnienie)
{
    m_amplituda = amplituda;
    m_okres = okres;
    m_offset = offset;
    m_wypelnienie = wypelnienie;
}

double Generator::oblicz_wartosc(double czas)
{
    if (czas < m_czas_aktywacji)
        return 0.0;

    if (m_typ == SINUS) {
        if (m_okres > 0) {
            return m_amplituda * std::sin(2 * M_PI * czas / m_okres) + m_offset;
        }
        return m_offset;
    } else if (m_typ == PROSTOKAT) {
        if (m_okres > 0) {
            double faza = std::fmod(czas, m_okres);
            double limit = m_okres * (m_wypelnienie);

            if (faza < limit) {
                return m_amplituda + m_offset;
            } else {
                return m_offset;
            }
        }
        return m_offset;
    }
    return 0.0;
}
