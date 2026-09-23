#include "ProstyUAR.h"

ProstyUAR::ProstyUAR()
    : m_aktualne_y(0.0)
    , m_aktualne_u(0.0)
    , m_aktualne_e(0.0)
    , m_aktualna_zadana(0.0)
    , m_aktualny_czas(0.0)
    , m_typ_regulatora(TypRegulatora::PID)
{
    std::vector<double> A = {0.0, 0.0, 0.0};
    std::vector<double> B = {0.0, 0.0, 0.0};
    m_model = new ModelARX(A, B);
    m_pid = new RegulatorPID(1.0, 1.0, 0.0);

}

ProstyUAR::~ProstyUAR()
{
    delete m_model;
    delete m_pid;

}

void ProstyUAR::symuluj_krok(double krok_czasu, double wartosc_zadana)
{
    m_aktualna_zadana = wartosc_zadana;
    m_aktualne_e = m_aktualna_zadana - m_aktualne_y;

    double u = 0.0;
    switch (m_typ_regulatora) {
    case TypRegulatora::PID:
        u = m_pid->symuluj(m_aktualne_e);
        break;
    case TypRegulatora::BRAK:
        u = 0.0;
        break;
    case TypRegulatora::ONOFF:
        break;
    }
    m_aktualne_u = u;
    m_aktualne_y = m_model->symuluj(m_aktualne_u);
    m_aktualny_czas += krok_czasu;
}

void ProstyUAR::ustaw_aktywny_regulator(TypRegulatora typ)
{
    m_typ_regulatora = typ;
}

void ProstyUAR::reset()
{
    m_aktualne_y = 0.0;
    m_aktualne_u = 0.0;
    m_aktualne_e = 0.0;
    m_aktualna_zadana = 0.0;
    m_aktualny_czas = 0.0;

    if (m_pid)
        m_pid->reset();
    if (m_model)
        m_model->reset();
}

double ProstyUAR::get_skladowa_p() const
{
    if (m_pid)
        return m_pid->get_wzmocnienie();
    return 0.0;
}

double ProstyUAR::get_skladowa_i() const
{
    if (m_pid)
        return m_pid->get_calka();
    return 0.0;
}

double ProstyUAR::get_skladowa_d() const
{
    if (m_pid)
        return m_pid->get_td();
    return 0.0;
}
