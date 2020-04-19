//
// Created by Callum Todd on 2020/04/17.
//

#ifndef COMPILER_VISUALIZATION_TWEEN_H
#define COMPILER_VISUALIZATION_TWEEN_H

#import <vector>
#import <functional>

template <class T>
class Tween {
private:
  struct Stage {
    bool isWait;
    T dest;
    double duration;
    std::function<void()> callback = nullptr;
  };

private:
  T current;
  double currentTime = 0;
  double pastStageTime = 0;

  bool complete = false;
  bool active = false;
  bool usedOnce = false;

  std::vector<Stage> stages;
  int currentStage;

  std::function<void()> stageCompleteCallback = nullptr;

private:
  class Builder {
  private:
    Tween* tween;
    bool useExistingCurrent;
    std::vector<Stage> stages;
  public:
    Builder(Tween* tween, T startValue, bool useCurrent)
        : tween(tween), useExistingCurrent(useCurrent) {
      if (useCurrent && tween->usedOnce) {
        stages.push_back({false, tween->current, 0});
      } else {
        stages.push_back({false, startValue, 0});
      }
    }
  public:
    Builder& wait(double duration) {
      if (duration > 0.0) {
        stages.push_back({
                             .isWait = true,
                             .dest = stages[stages.size() - 1].dest,
                             .duration = duration,
                         });
      }
      return *this;
    }
    Builder& goTo(T stepValue, double duration) {
      stages.push_back({
        .isWait = false,
        .dest = stepValue,
        .duration = duration,
      });
      return *this;
    }
    Builder& callback(std::function<void()> callback) {
      stages.back().callback = callback;
      return *this;
    }
    Builder& callbackOnAll(std::function<void()> callback) {
      tween->stageCompleteCallback = callback;
      return *this;
    }
    void finish() {
      tween->stages = stages;
      tween->currentStage = 1;

      if (!useExistingCurrent) {
        tween->current = tween->stages[0].dest;
      }

      tween->currentTime = 0;
      tween->pastStageTime = 0;

      tween->complete = false;
      tween->active = true;
      tween->usedOnce = true;

      if (tween->stages[0].callback) {
        tween->stages[0].callback();
      }

      // Self destruct
      delete this;
    }
  };

public:
  Builder& startAtCurrent(T defaultValue) {
    return *(new Builder(this, defaultValue, true));
  }
  Builder& startAt(T startValue) {
    return *(new Builder(this, startValue, false));
  }
  Builder& startAt(T startValue, bool useCurrentValueInsteadWherePossible) {
    return *(new Builder(this, startValue, useCurrentValueInsteadWherePossible));
  }

  bool isComplete() {
    return complete;
  }

  bool isActive() {
    return active;
  }

  void stop() {
    active = false;
  }

  bool update(double dt) {
    if (!active) return false;

    currentTime += dt;
    if (!stages[currentStage].isWait) {
      _update((currentTime - pastStageTime) / stages[currentStage].duration,
              stages[currentStage - 1].dest,
              stages[currentStage].dest);
    }

    if (currentTime >= pastStageTime + stages[currentStage].duration) {
      auto callback = stages[currentStage].callback;
      if (currentStage + 1 < stages.size()) {
        pastStageTime += stages[currentStage].duration;
        currentStage++;
      } else {
        complete = true;
        active = false;
      }
      if (callback) {
        callback();
      }
      if (stageCompleteCallback) {
        stageCompleteCallback();
      }
    }

    return complete;
  }

  void set(T value) {
    stop();
    usedOnce = true;
    current = value;
  }
  T get() {
    return current;
  }

private:
  void _update(double dt, T& start, T& end);

  float lerp(double t, float a, float b) {
    a *= (float)(1 - t);
    b *= (float)t;
    return a + b;
  }

};


#endif //COMPILER_VISUALIZATION_TWEEN_H
