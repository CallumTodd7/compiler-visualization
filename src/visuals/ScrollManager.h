//
// Created by Callum Todd on 2020/04/19.
//

#ifndef COMPILER_VISUALIZATION_SCROLLMANAGER_H
#define COMPILER_VISUALIZATION_SCROLLMANAGER_H


#include <common/Vector.h>
#include <iostream>
#include "Tween.h"

class ScrollManager {
private:
  love::Vector2 desiredOffset;
  Tween<love::Vector2> scrollOffset;

public:
  void update(double dt) {
    scrollOffset.update(dt);
  }

  love::Vector2
  getOffset(const love::Vector2& frameSize, const love::Vector2& contentSize, const love::Vector2& focusPoint) {
    love::Vector2 newDesiredOffset = getDesiredOffset(frameSize, contentSize, focusPoint);

    if (desiredOffset != newDesiredOffset) {
      desiredOffset = newDesiredOffset;

      scrollOffset
          .startAtCurrent({0, 0})
          .goTo(desiredOffset, 1.0f)
          .finish();
    }

    return scrollOffset.get();
  }

private:
  static love::Vector2 max(float a, const love::Vector2& b) {
    if (b.x >= a && b.y >= a) {
      return b;
    }

    love::Vector2 copy = b;
    if (b.x < a) copy.x = a;
    if (b.y < a) copy.y = a;
    return copy;
  }

  static love::Vector2 min(const love::Vector2& a, const love::Vector2& b) {
    love::Vector2 copy = a;
    if (b.x < a.x) copy.x = b.x;
    if (b.y < a.y) copy.y = b.y;
    return copy;
  }

  static love::Vector2 getDesiredOffset(const love::Vector2& frameSize,
                                        const love::Vector2& contentSize,
                                        const love::Vector2& focusPoint) {
    love::Vector2 desiredFocusPointLocation = {frameSize.x / 3 * 2, frameSize.y / 2};
    return -max(0, min(focusPoint - desiredFocusPointLocation, contentSize - frameSize));
  }


};


#endif //COMPILER_VISUALIZATION_SCROLLMANAGER_H
