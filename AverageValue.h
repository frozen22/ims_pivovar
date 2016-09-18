/**
 * File:   AverageValue.h
 * Author: Frantisek Nemec, xnemec61[at]stud.fit.vutbr.cz
 * Date: 2013-12-05
 */

#ifndef AVERAGEVALUE_H
#define	AVERAGEVALUE_H

class AverageValue 
{
    private:

        int m_count;
        double m_average;

    public:

        AverageValue() : m_count(0), m_average(0)
        {
        }

        void add(double val)
        {
            m_average = (m_average * m_count) + val;
            m_count++;
            m_average /= m_count;
        }

        double get()
        {
            return m_average;
        }
};

#endif	/* AVERAGEVALUE_H */

