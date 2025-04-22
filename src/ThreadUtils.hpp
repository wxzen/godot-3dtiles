#ifndef THREAD_UTILS_H
#define THREAD_UTILS_H

#include <condition_variable>
#include <future>
#include <mutex>
#include <queue>
#include <thread>

template <typename T> class threadsafe_queue
{
private:
    mutable std::mutex mut;
    std::queue<T> data_queue;
    std::condition_variable data_cond;

public:
    threadsafe_queue()
    {
    }
    void push( T new_value )
    {
        std::lock_guard<std::mutex> lk( mut );
        data_queue.push( std::move( new_value ) );
        data_cond.notify_one();
    }
    void wait_and_pop( T &value )
    {
        std::unique_lock<std::mutex> lk( mut );
        data_cond.wait( lk, [this] { return !data_queue.empty(); } );
        value = std::move( data_queue.front() );
        data_queue.pop();
    }
    std::shared_ptr<T> wait_and_pop()
    {
        std::unique_lock<std::mutex> lk( mut );
        data_cond.wait( lk, [this] { return !data_queue.empty(); } );
        std::shared_ptr<T> result( std::make_shared<T>( std::move( data_queue.front() ) ) );
        data_queue.pop();
        return result;
    }

    bool try_pop( T &value )
    {
        std::lock_guard<std::mutex> lk( mut );
        if ( data_queue.empty() )
        {
            return false;
        }
        value = std::move( data_queue.front() );
        data_queue.pop();
        return true;
    }

    std::shared_ptr<T> try_pop()
    {
        std::lock_guard<std::mutex> lk( mut );
        if ( data_queue.empty() )
        {
            return std::shared_ptr<T>();
        }
        std::shared_ptr<T> result( std::make_shared<T>( std::move( data_queue.front() ) ) );
        data_queue.pop();
        return result;
    }

    bool empty() const
    {
        std::lock_guard<std::mutex> lk( mut );
        return data_queue.empty();
    }
};

class join_threads
{
    std::vector<std::thread> &threads;

public:
    explicit join_threads( std::vector<std::thread> &threads_ ) : threads( threads_ ) {};
    ~join_threads()
    {
        for ( unsigned long i = 0; i < threads.size(); ++i )
        {
            if ( threads[i].joinable() )
            {
                threads[i].join();
            }
        }
    }
};

class function_wrapper
{
    struct impl_base
    {
        virtual void call() = 0;
        virtual ~impl_base()
        {
        }
    };
    std::unique_ptr<impl_base> impl;
    template <typename F> struct impl_type : impl_base
    {
        F f;
        impl_type( F &&f_ ) : f( std::move( f_ ) )
        {
        }
        void call()
        {
            f();
        }
    };

public:
    template <typename F> function_wrapper( F &&f ) : impl( new impl_type<F>( std::move( f ) ) )
    {
    }
    void operator()()
    {
        impl->call();
    }
    function_wrapper() = default;
    function_wrapper( function_wrapper &&other ) noexcept : impl( std::move( other.impl ) )
    {
    }

    function_wrapper &operator=( function_wrapper &&other ) noexcept
    {
        impl = std::move( other.impl );
        return *this;
    }
    function_wrapper( const function_wrapper & ) = delete;
    function_wrapper( function_wrapper & ) = delete;
    function_wrapper &operator=( const function_wrapper & ) = delete;
};

const int32_t NumberOfWorkerThreadsToSpawn = 4;
/**
*This works well for simple cases like this, where the tasks are independent. But it’s
not so good for situations where the tasks depend on other tasks also submitted to the
thread pool.
**/
class thread_pool
{
    std::atomic_bool done;
    threadsafe_queue<function_wrapper> work_queue;
    std::vector<std::thread> threads;
    join_threads joiner;
    void worker_thread()
    {
        while ( !done )
        {
            function_wrapper task;
            if ( work_queue.try_pop( task ) )
            {
                task();
            }
            else
            {
                std::this_thread::yield();
            }
        }
    }

public:
    thread_pool() : done( false ), joiner( threads )
    {
        initialize( 0 );
    }
    thread_pool( unsigned custom_thread_count ) : done( false ), joiner( threads )
    {
        initialize( custom_thread_count );
    }
    void initialize( unsigned custom_thread_count )
    {
        unsigned const sys_thread_count = std::thread::hardware_concurrency();
        unsigned thread_count;
        if ( custom_thread_count )
        {
            if ( sys_thread_count < 2 )
            {
                thread_count = 1;
            }
            else if ( sys_thread_count > custom_thread_count )
            {
                thread_count = custom_thread_count > 2 ? custom_thread_count : 2;
            }
            else
            {
                thread_count = sys_thread_count;
            }
        }
        else
        {
            thread_count = NumberOfWorkerThreadsToSpawn;
        }
        try
        {
            for ( unsigned i = 0; i < thread_count; ++i )
            {
                threads.push_back( std::thread( &thread_pool::worker_thread, this ) );
            }
        }
        catch ( ... )
        {
            done = true;
            throw;
        }
    }
    ~thread_pool()
    {
        done = true;
    }
    template <typename FunctionType> void enqueueWork( FunctionType f )
    {
        work_queue.push( std::move( f ) );
    }
};

#endif