#pragma once

#include "gacs_common.hpp"

namespace gacs
{
    template<typename T>
    class tsqueue
    {
    protected:
        std::deque<T> queue;
        std::mutex queueMutex;

    public:
        tsqueue() = default;
        tsqueue(const tsqueue<T>&) = delete; // don't allow copy
        virtual ~tsqueue() { clear(); }

        const T& front()
        {
            std::scoped_lock lock(queueMutex);
            return queue.front();
        }

        const T& back()
        {
            std::scoped_lock lock(queueMutex);
            return queue.back();
        }

        void clear()
        {
            std::scoped_lock lock(queueMutex);
            queue.clear();
        }

        T pop_front()
        {
            std::scoped_lock lock(queueMutex);
            /* deque's pop_front only deletes the item, it doesn't return it
             * So we cache the front item before deleting it
             * std::move avoids the copy */
            auto t = std::move(queue.front());
            queue.pop_front();
            return t;
        }

        T pop_back()
        {
            std::scoped_lock lock(queueMutex);
            /* deque's pop_back only deletes the item, it doesn't return it
             * So we cache the back item before deleting it from the deque
             * std::move avoids the copy */
            auto t = std::move(queue.back());
            queue.pop_back();
            return t;
        }

        void push_front(const T& item)
        {
            std::scoped_lock lock(queueMutex);
            queue.emplace_front((std::move(item)));
        }

        void push_back(const T& item)
        {
            std::scoped_lock lock(queueMutex);
            queue.emplace_back(std::move(item));
        }

        bool empty()
        {
            std::scoped_lock lock(queueMutex);
            return queue.empty();
        }

        size_t size()
        {
            std::scoped_lock lock(queueMutex);
            return queue.size();
        }
    };
}