#ifndef PUYO_SCORING_H
#define PUYO_SCORING_H

inline int countPoints(int groupSize, int phase)
{
    return 100 + (phase > 0 ? phase - 1 : 0) * 5000;
}

inline int countNeutrals(int groupSize, int phase)
{
    return (phase > 1 ? 8 : 0) + groupSize - 3;
}

#endif
