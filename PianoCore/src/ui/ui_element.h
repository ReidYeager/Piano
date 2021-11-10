
#ifndef PIANO_UI_UI_ELEMENT_H_
#define PIANO_UI_UI_ELEMENT_H_

#include "defines.h"
#include "math/vector.h"

class UiElement
{
private:
  void* theActualElement;

public:
  vec2 position;
  float rotation;
  vec2 scale;

public:
  void AddToScreen();
  void SetText();

};

#endif // !PIANO_UI_UI_ELEMENT_H_
