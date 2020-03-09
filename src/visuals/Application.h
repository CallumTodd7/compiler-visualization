//
// Created by Callum Todd on 2020/03/01.
//

#ifndef COMPILER_VISUALIZATION_APPLICATION_H
#define COMPILER_VISUALIZATION_APPLICATION_H

#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/Platform/GlfwApplication.h>
#include <Magnum/SceneGraph/Scene.h>
#include <Magnum/SceneGraph/MatrixTransformation2D.h>
#include <Magnum/SceneGraph/Camera.h>
#include <Magnum/SceneGraph/Drawable.h>

using namespace Magnum;

typedef SceneGraph::Scene<SceneGraph::MatrixTransformation2D> Scene2D;
typedef SceneGraph::Object<SceneGraph::MatrixTransformation2D> Object2D;

class TextLine: public Object2D, SceneGraph::Drawable2D {

};

class Application: public Platform::Application {
private:
  Scene2D scene;
  Object2D* cameraObject;
  SceneGraph::Camera2D* camera;
  SceneGraph::DrawableGroup2D drawables;

  TextLine* textLines[50];

public:
  explicit Application(const Arguments& arguments);

private:
  void drawEvent() override;

};


#endif //COMPILER_VISUALIZATION_APPLICATION_H
