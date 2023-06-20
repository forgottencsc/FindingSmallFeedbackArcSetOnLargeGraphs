#ifndef THREAD_POOL_HPP
#define THREAD_POOL_HPP

#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>

#include <memory>
#include <functional>
#include <queue>

#include <cassert>

namespace util {

    using std::atomic;
    using std::thread;
    using std::mutex;
    using std::unique_lock;
    using std::condition_variable;

    using std::unique_ptr;
    using std::vector;
    using std::function;
    using std::queue;

    using std::forward;

    struct simple_thread_pool {
        int tc;
        struct worker;
        mutex m0;
        condition_variable c0;
        int s0;
        vector<unique_ptr<worker>> ws;

        enum class event_type { null = 0, start = 1, stop = 2, work = 3 };

        struct event {
            event_type t;
            function<void()> fn;
        };

        struct worker {
            mutex m;
            condition_variable c;
            queue<event> q;
            bool s;
            thread t;
            atomic<int> qs;

            worker(mutex& m0, condition_variable& c0, int& s0) : s(0), t([&](){
                event e;
                e.t = event_type::null;
                while (e.t != event_type::stop) {
                    {
                        unique_lock<mutex> l(m);
                        c.wait(l, [&]() {
                            if (q.empty()) return false;
                            qs--;
                            e = q.front();
                            q.pop();
                            return true;
                        });
                    }
                    
                    if (e.t == event_type::start) {
                        unique_lock<mutex> l0(m0);
                        s0++;
                        c0.notify_one();
                    }
                    else if (e.t == event_type::stop) {
                        unique_lock<mutex> l0(m0);
                        s0--;
                        c0.notify_one();
                        return;
                    }
                    else if (e.t == event_type::work) {
                        e.fn();
                    }
                    else
                        assert(false);
                }
            }) {}


            void work(event e) {
                unique_lock<mutex> l(m);
                qs++;
                q.push(e);
                c.notify_one();
            }

            int qlen() {
                return qs.load();
            }
        };

        simple_thread_pool(int tc_ = -1)
        : tc(tc_ == -1 ? thread::hardware_concurrency() : tc_), s0(0) {
            for (int i = 0; i < tc; ++i)
                ws.emplace_back(new worker(m0, c0, s0));
            
            for (int i = 0; i < tc; ++i)
                ws[i]->work({ event_type::start });
            unique_lock<mutex> l0(m0);
            c0.wait(l0, [&](){ return s0 == tc; });
        }

        template<class Fn>
        void work(Fn&& fn) {
            int id = -1, ms;
            for (int i = 0; i < tc; ++i) {
                int qs = ws[i]->qlen();
                if (id != -1 && qs >= ms)
                    continue;
                id = i;
                ms = qs;
            }
            
            ws[id]->work({ event_type::work, forward<Fn>(fn) });
        }

        ~simple_thread_pool() {
            for (int i = 0; i < tc; ++i)
                ws[i]->work({ event_type::stop });
            {
                unique_lock<mutex> l0(m0);
                c0.wait(l0, [&](){ return s0 == 0; });
            }
            for (int i = 0; i < tc; ++i)
                ws[i]->t.join();
        }
    };

}


#endif