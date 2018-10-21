#ifndef AGG_STROKE_VERTEX_SEQUENCE_INCLUDED
#define AGG_STROKE_VERTEX_SEQUENCE_INCLUDED

#include "agg_vertex_sequence.h"

namespace agg
{
    template<class T, unsigned S=6>
    class stroke_vertex_sequence : public vertex_sequence<T, S>
    {
    public:
        typedef vertex_sequence<T, S> base_type;

        void add(const T& val);
        void close(bool remove_flag);

    private:
        const unsigned m_min_size = 2;
    };

    template<class T, unsigned S>
    void stroke_vertex_sequence<T, S>::add(const T& val)
    {
        unsigned size = base_type::size();
        if(size > 1)
        {
            if(!(*this)[size - 2]((*this)[size - 1]) && size > m_min_size)
            {
                base_type::remove_last();
            }
        }
        base_type::add(val);
    }

    template<class T, unsigned S>
    void stroke_vertex_sequence<T, S>::close(bool closed)
    {
        unsigned size = base_type::size();
        while(size > 1)
        {
            if((*this)[size - 2]((*this)[size - 1])
                || size <= m_min_size) break;
            T t = (*this)[size - 1];
            base_type::remove_last();
            base_type::modify_last(t);
            size = base_type::size();
        }

        if(closed)
        {
            size = base_type::size();
            while(size > 1)
            {
                if((*this)[base_type::size() - 1]((*this)[0])
                    || base_type::size() <= m_min_size) break;
                base_type::remove_last();
                size = base_type::size();
            }
        }
    }

    struct stroke_vertex_dist : vertex_dist
    {
        using vertex_dist::vertex_dist;

        bool operator () (const stroke_vertex_dist& val)
        {
            return (dist = calc_distance(x, y, val.x, val.y)) > vertex_dist_epsilon;
        }
    };
}

#endif
