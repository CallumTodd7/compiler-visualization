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

  love::Vector2 getOffset(const love::Vector2& frameSize,
                          const love::Vector2& contentSize,
                          const love::Vector2& focusPoint,
                          const love::Vector4& framePadding = {}) {
    love::Vector2 newDesiredOffset = getDesiredOffset({
                                                          frameSize.x - (framePadding.z),
                                                          frameSize.y - (framePadding.w),
                                                      },
                                                      contentSize,
                                                      focusPoint,
                                                      {framePadding.x, framePadding.x});

    if (desiredOffset != newDesiredOffset) {
      desiredOffset = newDesiredOffset;

      scrollOffset
          .startAtCurrent({0, 0})
          .goTo(desiredOffset, 1.0f)
          .finish();
    }

    return scrollOffset.get();
  }

  love::Vector2 getOffset(const love::Vector2& frameSize,
                          const love::Vector2& contentSize,
                          const love::Vector4& focusArea,
                          const love::Vector4& framePadding = {}) {
    love::Vector2 focusPoint = {focusArea.x, focusArea.y};
    focusPoint.x += focusArea.z * (focusArea.x / (frameSize.x - focusArea.z));
    focusPoint.y += focusArea.w * (focusArea.y / (frameSize.y - focusArea.w));
    return getOffset(frameSize, contentSize, focusPoint, framePadding);
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

  static love::Vector2 max(const love::Vector2& a, const love::Vector2& b) {
    love::Vector2 copy = b;
    if (b.x < a.x) copy.x = a.x;
    if (b.y < a.y) copy.y = a.y;
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
                                        const love::Vector2& focusPoint,
                                        const love::Vector2& padding) {
    love::Vector2 desiredFocusPointLocation = {frameSize.x / 3 * 2, frameSize.y / 2};
    return -max(-padding, min(focusPoint - desiredFocusPointLocation, contentSize - frameSize));
//    return -max(0, min(focusPoint, contentSize - frameSize));
  }


};


#endif //COMPILER_VISUALIZATION_SCROLLMANAGER_H
