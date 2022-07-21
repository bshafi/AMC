#pragma once

#include <atomic>
#include <optional>

struct HeadTail {
    uint32_t x, y;
};

template <typename T, size_t N>
struct atomic_ring_buffer {
    static_assert(N <= UINT16_MAX);
    static_assert(std::atomic<HeadTail>::is_always_lock_free);
    static_assert(std::is_trivial<T>::value);
    atomic_ring_buffer()
        : values{}, head_tail(HeadTail{ 0, 0 }) {
        assert(head_tail.is_lock_free());
    }
    void push(T val) {
        auto old_head_tail = head_tail.load();
        while (
            !head_tail.compare_exchange_strong(
                old_head_tail,
                increment_tail(old_head_tail)
            )
        ) {}

        uint32_t i = old_head_tail.y == 0 ? N - 1 : old_head_tail.y - 1;
        values[i] = val;
    }
    std::optional<T> pop() {
        auto old_head_tail = head_tail.load();

        if (old_head_tail.x == old_head_tail.y) {
            return std::nullopt;
        }

        if (head_tail.compare_exchange_strong(old_head_tail, increment_head(old_head_tail))) {
            uint32_t i = old_head_tail.x == 0 ? N - 1 : old_head_tail.x - 1;
            return values[i];
        }

        return std::nullopt;
    }
    static HeadTail increment_tail(HeadTail old_head_tail) {
        old_head_tail.y = (old_head_tail.y + 1) % N;
        if (old_head_tail.y == old_head_tail.x) {
            old_head_tail.x = (old_head_tail.x + 1) % N;
        }
        return old_head_tail;
    }
    static HeadTail increment_head(HeadTail old_head_tail) {
        if (old_head_tail.x != old_head_tail.y) {
            old_head_tail.x = (old_head_tail.x + 1) % N;
        }
        return old_head_tail;
    }

    T values[N];
    std::atomic<HeadTail> head_tail;
};

template <typename T, size_t N>
struct atomic_ring_buffer<std::unique_ptr<T>, N> {
    atomic_ring_buffer<T*, N> h_ring_buffer;

    void push(std::unique_ptr<T> &&val) {
        h_ring_buffer.push(val.release());
    }
    std::unique_ptr<T> pop() {
        auto val = h_ring_buffer.pop();
        if (val == std::nullopt) {
            return nullptr;
        } else {
            return std::unique_ptr<T>(*val);
        }
    }
};