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
    while (!(!isWorkerDone && isMainReady)) {
      cv.wait(lck, [&] { return !isWorkerDone && isMainReady; });
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

void ThreadSync::mainReady(const std::function<void()>& callback) {
  assert(this->isActive);

  // wait for worker's ready flag
  // - will only happen when the data is already saved
#if THREAD_DEBUG_MSG
  std::cout << "main about to wait" << std::endl;
#endif
  {
    std::unique_lock<std::mutex> lck(mtx);
    while (!(!isWorkerDone && isWorkerReady)) {
      cv.wait(lck, [&] { return !isWorkerDone && isWorkerReady; });
    }
  }
#if THREAD_DEBUG_MSG
  std::cout << "main processing" << std::endl;
#endif

  // allow data to be copied
  callback();

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
  return isWorkerDone;
}
