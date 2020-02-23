//
// Created by Callum Todd on 2020/02/21.
//

#ifndef COMPILER_VISUALIZATION_THREADSYNC_H
#define COMPILER_VISUALIZATION_THREADSYNC_H

#include <mutex>
#include <functional>

#define THREAD_DEBUG_MSG 0

class ThreadSync {
private:
  bool isActive;

  std::mutex mtx;
  std::condition_variable cv;

  bool isWorkerDone = false;
  bool isMainReady = false;
  bool isWorkerReady = false;

public:
  explicit ThreadSync(bool isActive)
    : isActive(isActive) {
  }

  void workerReady();
  void mainReady(const std::function<void()>& callback);

  void threadExit();
  bool isThreadDone();
};

#endif //COMPILER_VISUALIZATION_THREADSYNC_H
