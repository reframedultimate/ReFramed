#pragma once

#include "uh/config.hpp"

namespace uh {

class DamageCalculator;

class UH_PUBLIC_API KnockbackCalculator
{
public:
    KnockbackCalculator(
            double youPercent,
            double opponentPercent,
            double opponentWeight,
            bool is1v1);

    struct Move
    {
        Move(int id, double damage, double bkb, double kbg, bool isShorthop)
            : id(id), dmg(damage), bkb(bkb), kbg(kbg), isShorthop(isShorthop) {}

        int id;
        double dmg;
        double bkb;
        double kbg;
        bool isShorthop;
    };

    double addMove(const Move& move);
    double opponentPercent() const
        { return percent_; }

private:
    bool isInQueue(int id);
    void addToQueue(int id);
    double stalenessOf(int id);

private:
    double rage_;
    double percent_;
    double weight_;
    double mul1v1_;
    int staleQueue_[9];
    static double staleTable[9];
};

}
