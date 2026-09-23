#pragma once

class Regulator {
public:
    virtual ~Regulator() {}

    virtual double symuluj(double uchyb) = 0;

    virtual void reset() = 0;
};
