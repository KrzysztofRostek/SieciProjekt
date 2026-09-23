#include "RegulatorPID.h"
#include <cmath>
#include <limits>

RegulatorPID::RegulatorPID()
    : m_k(0.5)
    , m_ti(0.0)
    , m_td(0.0)
    , m_suma_uchybow(0.0)
    , m_poprzedni_uchyb(0.0)
    , m_tryb_calki(tryb_calki::StalaPodSuma)
    , m_poprzednia_i(0.0)
{}

RegulatorPID::RegulatorPID(double i_k)
    : m_k(i_k)
    , m_ti(0.0)
    , m_td(0.0)
    , m_suma_uchybow(0.0)
    , m_poprzedni_uchyb(0.0)
    , m_tryb_calki(tryb_calki::StalaPodSuma)
    , m_poprzednia_i(0.0)
{}

RegulatorPID::RegulatorPID(double i_k, double i_ti)
    : m_k(i_k)
    , m_ti(i_ti)
    , m_td(0.0)
    , m_suma_uchybow(0.0)
    , m_poprzedni_uchyb(0.0)
    , m_tryb_calki(tryb_calki::StalaPodSuma)
    , m_poprzednia_i(0.0)
{}

RegulatorPID::RegulatorPID(double i_k, double i_ti, double i_td)
    : m_k(i_k)
    , m_ti(i_ti)
    , m_td(i_td)
    , m_suma_uchybow(0.0)
    , m_poprzedni_uchyb(0.0)
    , m_tryb_calki(tryb_calki::StalaPodSuma)
    , m_poprzednia_i(0.0)
{}

void RegulatorPID::reset()
{
    m_suma_uchybow = 0.0;
    m_poprzedni_uchyb = 0.0;
    m_poprzednia_i = 0.0;
}

void RegulatorPID::set_nastawy(double k, double ti, double td)
{
   m_k = k;
    setStalaCalk(ti);
    m_td = td;
}

void RegulatorPID::setStalaCalk(double i_ti)
{
    if (i_ti == 0.0 && m_ti != 0.0) {
        if (m_tryb_calki == tryb_calki::StalaPodSuma) {
            m_suma_uchybow = 0.0;
        } else {
            m_suma_uchybow = 0.0;
            m_poprzednia_i = 0.0;
        }
    }

    m_ti = i_ti;
}

void RegulatorPID::set_tryb_calki(tryb_calki tryb)
{
    if (tryb == m_tryb_calki) {
        return;
    }

    if (tryb == tryb_calki::StalaPodSuma) {
        if (m_ti != 0.0) {
            m_suma_uchybow = m_poprzednia_i * m_ti;
        } else {
            m_suma_uchybow = 0.0;
        }
    } else {
        m_suma_uchybow = m_poprzednia_i;
    }
    m_tryb_calki = tryb;
}



double RegulatorPID::symuluj(double uchyb)
{
    double skladowa_p = m_k * uchyb;
    m_ostatnia_skladowa_p = skladowa_p;

    double skladowa_d = m_td * (uchyb - m_poprzedni_uchyb);
    m_poprzedni_uchyb = uchyb;
    m_ostatnia_skladowa_d = skladowa_d;

    double i = 0.0;

    if (m_ti > 0.0) {
        if (m_tryb_calki == tryb_calki::stala_przed_suma) {
            m_suma_uchybow += (1.0 / m_ti) * uchyb;
            i = m_suma_uchybow;
        } else {
            m_suma_uchybow += uchyb;
            i = (1.0 / m_ti) * m_suma_uchybow;
        }
    } else {
        i = 0.0;
        m_suma_uchybow = 0.0;
    }

    m_poprzednia_i = i;
    m_ostatnia_skladowa_i = i;

    double u_wyjscie = skladowa_p + i + skladowa_d;

    return u_wyjscie;
}
