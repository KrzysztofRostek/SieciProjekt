#include "ModelARX.h"
#include <algorithm>
#include <vector>
using namespace std;

ModelARX::ModelARX(const vector<double> &a,
                   const vector<double> &b,

                   int k,

                   double y_min,
                   double y_max,

                   bool y_min_max,
                   bool u_min_max,
                   double m_odchylenie)
{
    this->a_ = a;
    this->b_ = b;

    this->k = (k > 0) ? k : 1;

    this->y_.resize(a_.size(), 0.0);
    this->u_.resize(b_.size(), 0.0);

    this->u_buffor_for_k.resize(this->k, 0.0);

    this->y_min = y_min;
    this->y_max = y_max;

    this->y_min_max = y_min_max;
    this->u_min_max = u_min_max;

    this->m_odchylenie = m_odchylenie;

    mt19937 rd;
    this->generator.seed(rd());
}
void ModelARX::set_odchylenie_standardowe(double odchylenie)
{
    if (odchylenie > 0.0) {
        m_odchylenie = odchylenie;
        m_dystrybucja = normal_distribution<double>(0.0, m_odchylenie);
    } else {
        m_odchylenie = 0.0;
    }
}

double ModelARX::symuluj(double u_new)
{
    double u_limited = u_new;
    double u_delay = u_buffor_for_k[0];
    for (size_t i = 0; i < k - 1; ++i) {
        u_buffor_for_k[i] = u_buffor_for_k[i + 1];
    }
    u_buffor_for_k[u_buffor_for_k.size() - 1] = u_limited;

    if (!u_.empty()) {
        for (size_t i = u_.size() - 1; i > 0; --i) {
            u_[i] = u_[i - 1];
        }
        u_[0] = u_delay;
    }

    double y_new = b_calc() - a_calc();


    if (m_odchylenie > 0.0) {
        y_new += m_dystrybucja(generator);
    }

    if (y_min_max) {
        if (y_new > y_max)
            y_new = y_max;
        else if (y_new < y_min)
            y_new = y_min;
    }

    if (!y_.empty()) {
        for (size_t i = y_.size() - 1; i > 0; --i) {
            y_[i] = y_[i - 1];
        }
        y_[0] = y_new;
    }

    return y_new;
}

void ModelARX::set_u_min_max(bool u_min_max_)
{
    u_min_max = u_min_max_;
}

void ModelARX::set_y_min(double y_min_)
{
    y_min = y_min_;
    if (y_min > y_max) {
        y_max = y_min;
    }
}

void ModelARX::set_y_max(double y_max_)
{
    y_max = y_max_;
    if (y_min > y_max) {
        y_min = y_max;
    }
}

void ModelARX::set_y_min_max(bool y_min_max_)
{
    y_min_max = y_min_max_;
}
double ModelARX::a_calc()
{
    double a_val = 0.0;
    for (size_t i = 0; i < a_.size(); i++) {
        a_val += a_[i] * y_[i];
    }
    return a_val;
}

double ModelARX::b_calc()
{
    double b_val = 0.0;
    for (size_t i = 0; i < b_.size(); i++) {
        b_val += b_[i] * u_[i];
    }
    return b_val;
}

void ModelARX::set_wspolczynniki_a(const vector<double> &a)
{
    a_ = a;
    y_.resize(a_.size(), 0.0);
}

void ModelARX::set_wspolczynniki_b(const vector<double> &b)
{
    b_ = b;
    u_.resize(b_.size(), 0.0);
}

void ModelARX::set_opoznienie(int k_new)
{
    if (k_new < 1)
        k_new = 1;
    if (k_new == k)
        return;

    if (k_new > k) {
        u_buffor_for_k.insert(u_buffor_for_k.begin(), k_new - k, 0.0);
    } else {
        u_buffor_for_k.erase(u_buffor_for_k.begin(), u_buffor_for_k.begin() + (k - k_new));
    }
    k = k_new;
}

void ModelARX::reset() {
    std::fill(u_.begin(), u_.end(), 0.0);
    std::fill(y_.begin(), y_.end(), 0.0);
    std::fill(u_buffor_for_k.begin(), u_buffor_for_k.end(), 0.0);
}
