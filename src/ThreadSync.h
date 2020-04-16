//
// Created by Callum Todd on 2020/02/21.
//

#ifndef COMPILER_VISUALIZATION_THREADSYNC_H
#define COMPILER_VISUALIZATION_THREADSYNC_H

#include <mutex>
#include <functional>
#include <thread>
#include <utility>
#include <deque>
#include "Data.h"

#define THREAD_DEBUG_MSG 0

class ThreadTerminateException : public std::exception {
public:
  [[nodiscard]] const char* what() const _NOEXCEPT override {
    return "Thread terminate exception";
  }
};

class ThreadSync {
private:
  bool isActive;
  bool queueData;

  std::mutex mtx;
  std::condition_variable cv;

  bool isWorkerDone = false;
  bool isMainReady = false;
  bool isWorkerReady = false;

  bool shouldTerminate = false;

  Data data;
  std::queue<Data> dataQueue;

  bool threadStarted = false;
  std::thread compilationThread;
  int workerExitCode = 0;
  std::function<int(const std::function<void(const Data&)>&)> worker;

public:
  explicit ThreadSync(bool isActive,
                      bool queueData,
                      std::function<int(const std::function<void(Data)>&)> worker)
    : isActive(isActive), queueData(queueData), worker(std::move(worker)) {
  }

  void getData(const std::function<void(const Data&)>& callback);

  void terminateThread();

  void runWorker();
  void join();
  int getWorkerExitCode();

private:
  // Only used when not queued
  void workerReady();
  // Only used when not queued
  void mainReady(const std::function<void(const Data&)>& callback);

  void threadExit();
  bool isThreadDone();

};

#endif //COMPILER_VISUALIZATION_THREADSYNC_H
