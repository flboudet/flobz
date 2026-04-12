#ifndef FLOBO_SCORING_H
#define FLOBO_SCORING_H

inline int countPoints(int groupSize, int _phase)
{
    return 100 + (_phase > 0 ? _phase - 1 : 0) * 5000;
}

inline int countNeutrals(int groupSize, int _phase)
{
    return (_phase > 1 ? 8 : 0) + groupSize - 3;
}

#endif
