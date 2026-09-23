#pragma once
#include <vector>
#include <math.h>
#include <random>

using namespace std;
class ModelARX {
public:

    ModelARX(
        const vector<double>& a,
        const vector<double>& b,
        int k = 1,
        double y_min = -10.0,
        double y_max = 10.0,
        bool y_min_max = true,
        bool u_min_max = true,
        double odchylenie=0.0

        );

    //setery
    void set_wspolczynniki_a(const vector<double>& a);
    void set_wspolczynniki_b(const vector<double>& b);

    void set_u_min(double u_min_);
    void set_u_max(double u_max_);
    void set_u_min_max(bool u_min_max_);
    void set_y_min(double y_min_);
    void set_y_max(double y_max_);
    void set_y_min_max(bool y_min_max_);
    //funkcje
    double a_calc();
    double b_calc();
    double symuluj(double u_new);
    void set_odchylenie_standardowe(double odchylenie);
    void set_opoznienie(int k);
    void reset();

    // Getters for Save/Load
    const vector<double>& get_wspolczynniki_a() const { return a_; }
    const vector<double>& get_wspolczynniki_b() const { return b_; }
    int get_opoznienie() const { return k; }
    double get_odchylenie() const { return m_odchylenie; }

private:
    vector<double> a_;
    vector<double> b_;
    vector<double> u_;
    vector<double> y_;
    vector<double> u_buffor_for_k;
    int k;
    double u_min, u_max;
    double y_min, y_max;
    bool y_min_max;
    bool u_min_max;
   
   double m_odchylenie=0.0;
  
    mt19937 generator;

    normal_distribution<double> m_dystrybucja{ 0.0, 1.0 };
};
