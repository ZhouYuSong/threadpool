#ifndef THREADPOOL_H_
#define THREADPOOL_H_

#include <iostream>
#include <vector>
#include <queue>
#include <mutex>
#include <memory>
#include <thread>
#include <future>
#include <condition_variable>
#include <functional>
#include <stdexcept>

class ThreadPool {
 public:
  explicit ThreadPool(size_t t) : stop(false) {
    for (int i = 0; i < t; ++i) {
      workers.emplace_back([this] {
        while(1) {
          std::function<void()> task;
          {
            std::unique_lock<std::mutex> lk(queue_mutex);
            cv.wait(lk, [this]{
              return this->stop || !this->tasks.empty();    
            });
            if (this->stop && this->tasks.empty()) return;
            task = this->tasks.front();
            this->tasks.pop();
          }
          task();
        }
      });
    }
  }
  template <typename F, typename... Args>
  auto enqueue(F&& f, Args&&... args)
      ->std::future<typename std::result_of<F(Args...)>::type> {
    using return_type = typename std::result_of<F(Args...)>::type;
    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));  
    auto res = task->get_future();
    {
      std::lock_guard<std::mutex> lk(queue_mutex);
      if (this->stop) {
        throw std::runtime_error("enqueue on stopped threadpool");
      }
      if (this->stop) throw std::runtime_error("enqueue on stopped threadpool");
      this->tasks.emplace([task]{(*task)();});
    }
    cv.notify_one();
    return res;
  }
  ~ThreadPool() {
    {
      std::lock_guard<std::mutex> lk(queue_mutex);
      stop = true;
    } 
    cv.notify_all();
    for (auto& worker : workers) {
      worker.join();
    }
  }
 private:
  std::vector<std::thread> workers;
  std::queue<std::function<void()>> tasks;

  std::mutex queue_mutex;
  std::condition_variable cv;
  bool stop;
};
#endif
