#pragma once

#include <thread>
#include <chrono>
#include <vector>

#include "../src/atomic_ring_buffer.hpp"

#include "gtest/gtest.h"

TEST(atomic_ring_buffer, PushMulti) {
    using namespace std::chrono_literals;

    for (size_t h_i = 0; h_i < 1000; ++h_i) {
        atomic_ring_buffer<intptr_t, 1000> rbuf{};

        std::vector<std::thread> threads;
        const auto MAX_THREADS = 8;
        for (size_t i = 0; i < MAX_THREADS; ++i) {
            threads.emplace_back(std::thread(
                [&rbuf](){
                    for (int i = 0; i < 50; ++i) {
                        rbuf.push(i);
                    }
                }
            ));
        }

        for (size_t i = 0; i < threads.size(); ++i) {
            threads[i].join();
        }
        auto head_tail = rbuf.head_tail.load();
        ASSERT_EQ(head_tail.y, MAX_THREADS * 50);
        ASSERT_EQ(head_tail.x, 0);
    }
}

TEST(atomic_ring_buffer, PopMulti) {
    using namespace std::chrono_literals;

    for (size_t h_i = 0; h_i < 1000; ++h_i) {
        atomic_ring_buffer<intptr_t, 100> buf;

        for (int i = 0; i < 50; ++i) {
            buf.push(i);
        }

        std::vector<std::thread> threads;
        const auto MAX_THREADS = std::thread::hardware_concurrency();
        std::atomic<int> count(0);
        for (int i = 0; i < MAX_THREADS; ++i) {
            threads.emplace_back(std::thread(
                [&buf, &count](int t_id){
                    auto start = std::chrono::steady_clock::now();
                    for (int i = 0; i < 1000; ++i) {
                        auto data = buf.pop();
                        if (data != std::nullopt) {
                            count.fetch_add(1, std::memory_order_relaxed);
                        }
                    }
                },
                i
            ));
        }

        for (int i = 0; i < MAX_THREADS; ++i) {
            threads[i].join();
        }
        ASSERT_EQ(50, count.load());
    }
}

TEST(atomic_ring_buffer, PopPushMulti) {
    for (size_t h_i = 0; h_i < 1000; ++h_i) {
        const auto MAX_THREADS = 8;
        atomic_ring_buffer<intptr_t, 1000> buf;
        atomic_ring_buffer<intptr_t, 1000> wbuf;

        std::vector<std::thread> threads;
        std::atomic<int> count(0);
        for (int i = 0; i < MAX_THREADS; ++i) {
            if (i % 2 == 0) {
                threads.emplace_back(std::thread(
                    [&buf](int t_id) {
                        for (int i = 0; i < 50; ++i) {
                            buf.push((t_id / 2) * 50 + i);
                        }
                    },
                    i
                ));
            } else {
                threads.emplace_back(std::thread(
                    [&buf, &count, &wbuf](int t_id){
                        for (int i = 0; i < 1000; ++i) {
                            auto data = buf.pop();
                            if (data != std::nullopt) {
                                count.fetch_add(1, std::memory_order_relaxed);
                                wbuf.push(*data);
                            }
                        }
                    },
                    i
                ));
            }
        }

        for (size_t i = 0; i < threads.size(); ++i) {
            threads[i].join();
        }

        ASSERT_EQ((MAX_THREADS / 2) * 50, count.load());

        auto wbuf_head_tail = wbuf.head_tail.load();

        std::set<intptr_t> buf_data;
        for (int t_id = 0; t_id < MAX_THREADS; ++t_id) {
            if (t_id % 2 == 0) {
                for (int i = 0; i < 50; ++i) {
                    buf_data.insert((t_id / 2) * 50 + i);
                }
            }
        }

        std::set<intptr_t> wbuf_data;
        for (int i = wbuf_head_tail.x; i != wbuf_head_tail.y; i = (i + 1) % 1000) {
            wbuf_data.insert(wbuf.values[i]);
        }

        ASSERT_EQ(buf_data, wbuf_data);
    }
}
