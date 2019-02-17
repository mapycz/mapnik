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

#ifndef POSTGIS_ASYNCRESULTSET_HPP
#define POSTGIS_ASYNCRESULTSET_HPP

#include <mapnik/debug.hpp>
#include <mapnik/datasource.hpp>

#include "connection_manager.hpp"
#include "resultset.hpp"
#include <queue>
#include <memory>

class postgis_processor_context;
using postgis_processor_context_ptr = std::shared_ptr<postgis_processor_context>;

class AsyncResultSet : public IResultSet, private mapnik::util::noncopyable
{
public:
    AsyncResultSet(postgis_processor_context_ptr const& ctx,
                     std::shared_ptr<ConnectionManager::PoolType> const& pool,
                     std::unique_ptr<ConnectionManager::PoolType::handle> && conn_handle,
                     std::string const& sql)
        : ctx_(ctx),
          pool_(pool),
          conn_handle_(std::move(conn_handle)),
          sql_(sql),
          is_closed_(false)
    {
    }

    virtual bool use_connection() { return true; }

    virtual ~AsyncResultSet()
    {
        close();
    }


    void abort()
    {
        if(conn_handle_ && conn_handle_->get().isPending())
        {
            Connection & conn = conn_handle_->get();
            MAPNIK_LOG_DEBUG(postgis) << "AsyncResultSet: aborting pending connection - " << &conn;
            // there is no easy way to abort a pending connection, so we close it : this will ensure that
            // the connection will be recycled in the pool
            conn.close();
        }
    }

    virtual void close()
    {
        if (!is_closed_)
        {
            rs_.reset();
            is_closed_ = true;
            if (conn_handle_)
            {
                if(conn_handle_->get().isPending())
                {
                    abort();
                }
                conn_handle_.reset();
            }
        }
    }

    virtual int getNumFields() const
    {
        return rs_->getNumFields();
    }

    virtual bool next()
    {
        bool next_res = false;
        if (!rs_)
        {
            Connection & conn = conn_handle_->get();
            // Ensure connection is valid
            if (conn_handle_ && conn_handle_->get().isOK())
            {
                rs_ = conn_handle_->get().getAsyncResult();
            }
            else
            {
                throw mapnik::datasource_exception("invalid connection in AsyncResultSet::next");
            }
        }

        next_res = rs_->next();
        if (!next_res)
        {
            rs_.reset();
            rs_ = conn_handle_->get().getNextAsyncResult();
            if (rs_ && rs_->next())
            {
                return true;
            }
            close();
            prepare_next();
        }
        return next_res;
    }

    virtual const char* getFieldName(int index) const
    {
        return rs_->getFieldName(index);
    }

    virtual int getFieldLength(int index) const
    {
        return rs_->getFieldLength(index);
    }

    virtual int getFieldLength(const char* name) const
    {
        return rs_->getFieldLength(name);
    }

    virtual int getTypeOID(int index) const
    {
        return rs_->getTypeOID(index);
    }

    virtual int getTypeOID(const char* name) const
    {
        return rs_->getTypeOID(name);
    }

    virtual bool isNull(int index) const
    {
        return rs_->isNull(index);
    }

    virtual const char* getValue(int index) const
    {
        return rs_->getValue(index);
    }

    virtual const char* getValue(const char* name) const
    {
        return rs_->getValue(name);
    }

private:
    postgis_processor_context_ptr ctx_;
    std::shared_ptr<ConnectionManager::PoolType> pool_;
    std::unique_ptr<ConnectionManager::PoolType::handle> conn_handle_;
    std::string sql_;
    std::shared_ptr<ResultSet> rs_;
    bool is_closed_;

    void prepare()
    {
        conn_handle_ = pool_->borrow();
        Connection & conn = conn_handle_->get();
        if (conn.isOK())
        {
            conn.executeAsyncQuery(sql_, 1);
        }
        else
        {
            throw mapnik::datasource_exception("Postgis Plugin: bad connection");
        }
    }

    void prepare_next();

};


class postgis_processor_context : public mapnik::IProcessorContext
{
public:
    postgis_processor_context()
        : num_async_requests_(0) {}
    ~postgis_processor_context() {}

    void add_request(std::shared_ptr<AsyncResultSet> const& req)
    {
        q_.push(req);
    }

    std::shared_ptr<AsyncResultSet> pop_next_request()
    {
        std::shared_ptr<AsyncResultSet> r;
        if (!q_.empty())
        {
            r = q_.front();
            q_.pop();
        }
        return r;
    }

    int num_async_requests_;

private:
    using async_queue = std::queue<std::shared_ptr<AsyncResultSet> >;
    async_queue q_;

};

inline void AsyncResultSet::prepare_next()
{
    // ensure cnx pool has unused cnx
    std::shared_ptr<AsyncResultSet> next = ctx_->pop_next_request();
    if (next)
    {
        next->prepare();
    }
}

#endif // POSTGIS_ASYNCRESULTSET_HPP
