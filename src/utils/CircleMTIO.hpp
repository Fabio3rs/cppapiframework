#pragma once
#ifndef CIRCLEMTIO_HPP
#define CIRCLEMTIO_HPP
#include <atomic>
#include <mutex>
#include <array>

template<class T>
struct  __attribute__((aligned(64))) singlevarnofsh {
    T a;
};

template<size_t num, class T>
class  __attribute__((aligned(64))) CircleMTIO
{
    std::array<T, num>                                          elements;
    std::array<singlevarnofsh<std::atomic<bool>>, num>          elements_ready;

    std::atomic<size_t>                                         reading_point;
    std::atomic<size_t>                                         writing_point;

public:
    typedef T value_type;
    typedef std::pair<value_type, size_t> pair_t;

    std::pair<T*, size_t> new_write()
    {
        size_t a = writing_point.fetch_add(1);

        if (writing_point >= num)
        {
            writing_point = 0;
        }

        /*if (a >= num)
        {
            //throw std::runtime_error("atomic operation bug ");
        }*/

        return std::pair<T*, size_t>(&elements[a], a);
    }

    std::pair<T*, bool> next()
    {
        size_t r = reading_point;

        if (!elements_ready[r].a)
        {
            return std::pair<T*, bool>(nullptr, false);
        }

        if (r == writing_point)
        {
            return std::pair<T*, bool>(nullptr, false);
        }

        reading_point++;

        if (reading_point >= num)
        {
            reading_point = 0;
        }

        elements_ready[r].a = false;

        return std::pair<T*, bool>(&elements[r], true);
    }

    void set_ready(size_t a)
    {
        elements_ready[a].a = true;
    }

    CircleMTIO()
    {
        reading_point = 0;
        writing_point = 0;

        for (auto& b : elements_ready)
        {
            b.a = false;
        }
    }
};


#endif

