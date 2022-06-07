#pragma once

#include "gacs_common.hpp"

namespace gacs {
template <typename T>
class tsqueue {
   protected:
    std::deque<T> queue_;
    std::mutex queueMutex_;

    std::condition_variable cvBlocking_;
    std::mutex cvMutex_;

   public:
    tsqueue() = default;
    tsqueue(const tsqueue<T>&) = delete;  // don't allow copy
    virtual ~tsqueue() { clear(); }

    const T& front() {
        std::scoped_lock lock(queueMutex_);
        return queue_.front();
    }

    const T& back() {
        std::scoped_lock lock(queueMutex_);
        return queue_.back();
    }

    void clear() {
        std::scoped_lock lock(queueMutex_);
        queue_.clear();
    }

    T pop_front() {
        std::scoped_lock lock(queueMutex_);
        /* deque's pop_front only deletes the item, it doesn't return it
         * So we cache the front item before deleting it
         * std::move avoids the copy */
        auto t = std::move(queue_.front());
        queue_.pop_front();
        return t;
    }

    T pop_back() {
        std::scoped_lock lock(queueMutex_);
        /* deque's pop_back only deletes the item, it doesn't return it
         * So we cache the back item before deleting it from the deque
         * std::move avoids the copy */
        auto t = std::move(queue_.back());
        queue_.pop_back();
        return t;
    }

    void push_front(const T& item) {
        std::scoped_lock lock(queueMutex_);
        queue_.emplace_front((std::move(item)));

        std::unique_lock<std::mutex> ul(cvMutex_);
        cvBlocking_.notify_one();
    }

    void push_back(const T& item) {
        std::scoped_lock lock(queueMutex_);
        queue_.emplace_back(std::move(item));

        std::unique_lock<std::mutex> ul(cvMutex_);
        cvBlocking_.notify_one();
    }

    bool empty() {
        std::scoped_lock lock(queueMutex_);
        return queue_.empty();
    }

    size_t size() {
        std::scoped_lock lock(queueMutex_);
        return queue_.size();
    }

    void wait() {
        while (empty()) {
            std::unique_lock<std::mutex> ul(cvMutex_);
            cvBlocking_.wait(ul);
        }
    }
};
}  // namespace gacs