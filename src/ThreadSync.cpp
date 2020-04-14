//
// Created by Callum Todd on 2020/02/21.
//

#include "ThreadSync.h"
#if THREAD_DEBUG_MSG
#include <iostream>
#endif

void ThreadSync::workerReady() {
  // If not running in UI mode, silently skip method
  if (!this->isActive) return;

  if (this->shouldTerminate) {
    throw ThreadTerminateException();
  }

  // set ready flag
  // - allowing main thread to copy data
  isWorkerReady = true;
  cv.notify_all();
#if THREAD_DEBUG_MSG
  std::cout << "isWorkerReady = true; notify" << std::endl;
#endif

  // wait for main's ready flag
  // - meaning data has been copied by main thread
#if THREAD_DEBUG_MSG
  std::cout << "worker about to wait" << std::endl;
#endif
  {
    std::unique_lock<std::mutex> lck(mtx);
    while (!(isMainReady || isThreadDone())) {
      cv.wait(lck, [&] { return isMainReady || isThreadDone(); });
    }
  }
#if THREAD_DEBUG_MSG
  std::cout << "worker processing" << std::endl;
#endif

  // reset ready flags
#if THREAD_DEBUG_MSG
  std::cout << "reset flags" << std::endl;
#endif
  isWorkerReady = false;
  isMainReady = false;
}

void ThreadSync::mainReady(const std::function<void(const Data&)>& callback) {
  assert(this->isActive);

  // If worker is not stated, silently skip method
  if (!threadStarted) return;
  // If worker is finished, silently skip method
  if (this->isThreadDone()) return;

  // wait for worker's ready flag
  // - will only happen when the data is already saved
#if THREAD_DEBUG_MSG
  std::cout << "main about to wait" << std::endl;
#endif
  {
    std::unique_lock<std::mutex> lck(mtx);
    while (!(isWorkerReady || isThreadDone())) {
      cv.wait(lck, [&] { return isWorkerReady || isThreadDone(); });
    }
  }
#if THREAD_DEBUG_MSG
  std::cout << "main processing" << std::endl;
#endif

  // allow data to be copied
  if (!this->isThreadDone()) {
    callback(data);
  }

  // set ready flag
  // - allowing worker thread to continue
  isMainReady = true;
  cv.notify_all();
#if THREAD_DEBUG_MSG
  std::cout << "isMainReady = true; notify" << std::endl;
#endif
}

void ThreadSync::threadExit() {
  isWorkerDone = true;
  cv.notify_all();
}

bool ThreadSync::isThreadDone() {
  return isWorkerDone || shouldTerminate;
}

void ThreadSync::terminateThread() {
  shouldTerminate = true;
  cv.notify_all();
}

void ThreadSync::runWorker() {
  threadStarted = true;
  compilationThread = std::thread([=]{
    workerExitCode = 0;
    try {
      workerExitCode = worker([&](Data newData) {
        data = newData;
        this->workerReady();
      });
    } catch (ThreadTerminateException&) {
      // Thread has been told to terminate; exit
    }
    threadExit();
  });
}

void ThreadSync::join() {
  if (threadStarted) {
    compilationThread.join();
  }
}

int ThreadSync::getWorkerExitCode() {
  return workerExitCode;
}
