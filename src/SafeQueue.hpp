// SafeQueue.hpp

#ifndef SAFEQUEUE_HPP
#define SAFEQUEUE_HPP

#include <mutex>
#include <list>
#include <condition_variable>
namespace kv_base {

// A thread-safe queue
template <class T>
class SafeQueue {
public:
    SafeQueue();
    int size();
    void enqueue(const T &item);
    void dequeue_blocking(T &item);
    void clear();
private:
    std::mutex mutex;
    std::condition_variable empty;
    std::list<T> queue;
};

template <class T>
SafeQueue<T>::SafeQueue() {
    // nothing to do
}

template <class T>
int SafeQueue<T>::size() {
    std::lock_guard<std::mutex> locker(mutex);
    return queue.size();
}

template <class T>
void SafeQueue<T>::dequeue_blocking(T &item) {
    std::lock_guard<std::mutex> locker(mutex);
    while (queue.empty())
        empty.wait(mutex);
    item = queue.front();
    queue.pop_front();
}

template <class T>
void SafeQueue<T>::enqueue(const T &itme) {
    std::lock_guard<std::mutex> locker(mutex);
    queue.push_back(item);
    empty.notify_one();
}

template <class T>
void SafeQueue<T>::clear() {
    std::lock_guard<std::mutex> locker(mutex);
    queue.clear();
}

} // namespace kv_base

#endif // SAFEQUEUE_HPP