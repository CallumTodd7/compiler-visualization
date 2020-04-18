//
// Created by Callum Todd on 2020/04/17.
//

#include "Tween.h"
#include <common/Vector.h>

template<>
void Tween<love::Vector2>::_update(double t, love::Vector2& start, love::Vector2& end) {
  current.x = lerp(t, start.x, end.x);
  current.y = lerp(t, start.y, end.y);
}

template<>
void Tween<love::Vector3>::_update(double t, love::Vector3& start, love::Vector3& end) {
  current.x = lerp(t, start.x, end.x);
  current.y = lerp(t, start.y, end.y);
  current.z = lerp(t, start.z, end.z);
}

template<>
void Tween<love::Vector4>::_update(double t, love::Vector4& start, love::Vector4& end) {
  current.x = lerp(t, start.x, end.x);
  current.y = lerp(t, start.y, end.y);
  current.z = lerp(t, start.z, end.z);
  current.w = lerp(t, start.w, end.w);
}

