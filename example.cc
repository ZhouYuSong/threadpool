#include "ThreadPool.h"

#include <chrono>

int main() {
  ThreadPool pool(4);
  std::vector<std::future<int>> results;
  for (int i = 0; i < 8; ++i) {
    results.emplace_back(
      pool.enqueue([i]{
          std::cout << "thread " << i << " begin." << std::endl;
          std::this_thread::sleep_for(std::chrono::seconds(1));
          std::cout << "thread " << i << " end."  << std::endl;
          return i * i;
        }) 
    );
  }

  for (auto& res : results) {
    std::cout << res.get() << std::endl;
  }
  return 0;
}
