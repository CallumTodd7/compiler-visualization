//
// Created by Callum Todd on 2020/04/17.
//

#include "Tween.h"
#include <common/Vector.h>

template<>
void Tween<love::Vector2>::_update(double t) {
  current.x = lerp(t, initial.x, dest.x);
  current.y = lerp(t, initial.y, dest.y);
}

template<>
void Tween<love::Vector3>::_update(double t) {
  current.x = lerp(t, initial.x, dest.x);
  current.y = lerp(t, initial.y, dest.y);
  current.z = lerp(t, initial.z, dest.z);
}

template<>
void Tween<love::Vector4>::_update(double t) {
  current.x = lerp(t, initial.x, dest.x);
  current.y = lerp(t, initial.y, dest.y);
  current.z = lerp(t, initial.z, dest.z);
  current.w = lerp(t, initial.w, dest.w);
}
