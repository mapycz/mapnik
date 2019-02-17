/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2015 Artem Pavlenko
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *****************************************************************************/

#ifndef MAPNIK_CONNECTION_POOL_HPP
#define MAPNIK_CONNECTION_POOL_HPP

#include <algorithm>
#include <deque>
#include <memory>
#include <mutex>
#include <condition_variable>

namespace mapnik
{

template <typename Connection, template <typename> class Creator>
class connection_pool
{
    connection_pool(connection_pool const &) = delete;

    struct holder
    {
        holder(std::unique_ptr<Connection> && connection)
            : connection(std::move(connection)), is_free(true)
        {
        }

        std::unique_ptr<Connection> connection;
        bool is_free;
    };

    Creator<Connection> creator_;
    unsigned initial_size_;
    unsigned max_size_;
    std::deque<holder> holders_;
    std::mutex mutex_;
    std::condition_variable cv_;

    void grow(unsigned size)
    {
        for (unsigned i = holders_.size(); i < std::min(size, max_size_); ++i)
        {
            std::unique_ptr<Connection> connection(creator_());
            if (connection->isOK())
            {
                holders_.emplace_back(std::move(connection));
            }
        }
    }

    void give_back(holder & h)
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            h.is_free = true;
        }
        cv_.notify_one();
    }

    holder *find_free()
    {
        for (auto & hold : holders_)
        {
            if (hold.is_free)
            {
                return &hold;
            }
        }

        return nullptr;
    }

public:
    class handle
    {
        friend connection_pool<Connection, Creator>;

        connection_pool & pool_;
        holder & holder_;

        handle(connection_pool & pool, holder & hold)
            : pool_(pool), holder_(hold)
        {
            holder_.is_free = false;
        }

        handle(handle const &) = delete;

    public:
        ~handle()
        {
            pool_.give_back(holder_);
        }

        Connection & get()
        {
            return *holder_.connection;
        }
    };

    friend handle;

    connection_pool(const Creator<Connection>& creator, unsigned initialSize, unsigned maxSize)
        :creator_(creator),
         initial_size_(initialSize),
         max_size_(maxSize)
    {
        grow(initial_size_);
    }

    std::unique_ptr<handle> try_borrow()
    {
        std::lock_guard<std::mutex> lock(mutex_);

        if (holder *hold = find_free())
        {
            if (!hold->connection->isOK())
            {
                hold->connection.reset(creator_());
            }
            return std::unique_ptr<handle>(new handle(*this, *hold));
        }

        // all connection have been taken, check if we allowed to grow pool
        if (holders_.size() < max_size_)
        {
            std::unique_ptr<Connection> connection(creator_());
            holders_.emplace_back(std::move(connection));
            return std::unique_ptr<handle>(new handle(*this, holders_.back()));
        }

        return nullptr;
    }

    std::unique_ptr<handle> borrow()
    {
        if (std::unique_ptr<handle> h = try_borrow())
        {
            return h;
        }

        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this] { return this->find_free(); });

        holder *hold = find_free();

        assert(hold);

        if (!hold->connection->isOK())
        {
            hold->connection.reset(creator_());
        }
        return std::unique_ptr<handle>(new handle(*this, *hold));
    }

    unsigned size() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return holders_.size();
    }

    unsigned max_size() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return max_size_;
    }

    void set_max_size(unsigned size)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        max_size_ = std::max(max_size_, size);
    }

    unsigned initial_size() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return initial_size_;
    }

    void set_initial_size(unsigned size)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (size > initial_size_)
        {
            initial_size_ = size;
            grow(size);
        }
    }
};

}

#endif // MAPNIK_CONNECTION_POOL_HPP
