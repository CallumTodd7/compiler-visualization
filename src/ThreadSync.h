//
// Created by Callum Todd on 2020/02/21.
//

#ifndef COMPILER_VISUALIZATION_THREADSYNC_H
#define COMPILER_VISUALIZATION_THREADSYNC_H

#include <mutex>
#include <functional>
#include <thread>
#include <utility>

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

  std::mutex mtx;
  std::condition_variable cv;

  bool isWorkerDone = false;
  bool isMainReady = false;
  bool isWorkerReady = false;

  bool shouldTerminate = false;

  std::thread compilationThread;
  int workerExitCode = 0;
  std::function<int(const std::function<void()>&)> worker;

public:
  explicit ThreadSync(bool isActive, std::function<int(const std::function<void()>&)>  worker)
    : isActive(isActive), worker(std::move(worker)) {
  }

  void workerReady();
  void mainReady(const std::function<void()>& callback);

  void threadExit();
  bool isThreadDone();

  void terminateThread();

  void runWorker();
  void join();
  int getWorkerExitCode();

private:

};

#endif //COMPILER_VISUALIZATION_THREADSYNC_H
