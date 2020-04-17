//
// Created by Callum Todd on 2020/04/17.
//

#ifndef COMPILER_VISUALIZATION_TWEEN_H
#define COMPILER_VISUALIZATION_TWEEN_H

template <class T>
class Tween {
private:
  T initial;
  T current;
  T dest;
  double currentTime = 0;
  double targetTime = 0;
  bool complete = false;
  bool active = false;

public:
  void tween(T startValue, T endValue, double duration, bool setCurrent) {
    initial = startValue;
    if (setCurrent) {
      current = startValue;
    }

    dest = endValue;

    currentTime = 0;
    targetTime = duration;

    complete = false;
    active = true;
  }

  bool isComplete() {
    bool isComplete = (currentTime >= targetTime);
    if (isComplete != complete) {
      complete = isComplete;
      if (complete) {
        active = false;
      }
    }
    return complete;
  }

  bool isActive() {
    return active;
  }

  bool update(double dt) {
    if (!active) return false;

    currentTime += dt;
    _update(currentTime / targetTime);
    return isComplete();
  }

  T get() {
    return current;
  }

private:
  void _update(double dt);

  float lerp(double t, float a, float b) {
    a *= (float)(1 - t);
    b *= (float)t;
    return a + b;
  }

};


#endif //COMPILER_VISUALIZATION_TWEEN_H
