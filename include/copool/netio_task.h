#ifndef AMANI_NETIO_TASK_H
#define AMANI_NETIO_TASK_H

#include <cstdint>
#include <coroutine>

#include <sys/epoll.h>

enum CO_STATE { CO_RUNNING, CO_IOWAIT};

class netio_task {
public:
    class promise_type {
    public:
		promise_type() : fd(-1), run_state(CO_RUNNING), events(EPOLLIN) {}

        netio_task get_return_object()
        { return {netio_task(std::coroutine_handle<netio_task::promise_type>::from_promise(*this))}; }
        std::suspend_always initial_suspend() { return {}; }  // always suspend at start
        std::suspend_always final_suspend() noexcept { return {}; }
		void return_value(int status) {ret_status = status;}
        void unhandled_exception() { throw; }

    public:
		int fd;
		int ret_status;
		CO_STATE run_state;
		uint32_t events;
    };

public:
    std::coroutine_handle<netio_task::promise_type> handle_;
};

#endif