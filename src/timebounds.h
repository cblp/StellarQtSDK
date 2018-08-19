#ifndef TIMEBOUNDS_H
#define TIMEBOUNDS_H
#include "xdr/stellartransaction.h"

/**
 * <p>TimeBounds represents the time interval that a transaction is valid.</p>
 * @see Transaction
 */
class TimeBounds
{
    qint64 m_minTime;
    qint64 m_maxTime;
public:
    /**
     * @param minTime 64bit Unix timestamp
     * @param maxTime 64bit Unix timestamp
     */
    TimeBounds(qint64 minTime, qint64 maxTime);


    qint64 getMinTime() const;

    qint64 getMaxTime() const;

    static TimeBounds* fromXdr(stellar::TimeBounds& timeBounds);

    stellar::TimeBounds toXdr();

    bool equals(TimeBounds * other) const;
    bool operator==(const TimeBounds &other) const;
};

#endif // TIMEBOUNDS_H
