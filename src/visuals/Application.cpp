//
// Created by Callum Todd on 2020/03/01.
//

#include "Application.h"

Application::Application(const Platform::GlfwApplication::Arguments& arguments) : GlfwApplication(arguments) {
  /* Set up objects */
//  (*(textLines[0] = new TextLine{1, shader, Color3::from(0x3bd267), _cube, _scene, _drawables}))
//      .rotate(34.0_degf, Vector3(1.0f).normalized())
//      .translate({1.0f, 0.3f, -1.2f});
//  (*(_objects[1] = new PickableObject{2, _shader, 0x2f83cc_rgbf, _sphere, _scene, _drawables}))
//      .translate({-1.2f, -0.3f, -0.2f});
//  (*(_objects[2] = new PickableObject{3, _shader, 0xdcdcdc_rgbf, _plane, _scene, _drawables}))
//      .rotate(278.0_degf, Vector3(1.0f).normalized())
//      .scale(Vector3(0.45f))
//      .translate({-1.0f, 1.2f, 1.5f});
//  (*(_objects[3] = new PickableObject{4, _shader, 0xc7cf2f_rgbf, _sphere, _scene, _drawables}))
//      .translate({-0.2f, -1.7f, -2.7f});
//  (*(_objects[4] = new PickableObject{5, _shader, 0xcd3431_rgbf, _sphere, _scene, _drawables}))
//      .translate({0.7f, 0.6f, 2.2f})
//      .scale(Vector3(0.75f));
//  (*(_objects[5] = new PickableObject{6, _shader, 0xa5c9ea_rgbf, _cube, _scene, _drawables}))
//      .rotate(-92.0_degf, Vector3(1.0f).normalized())
//      .scale(Vector3(0.25f))
//      .translate({-0.5f, -0.3f, 1.8f});

  /* Configure camera */
  cameraObject = new Object2D{&scene};
  camera = new SceneGraph::Camera2D{*cameraObject};
  camera->setAspectRatioPolicy(SceneGraph::AspectRatioPolicy::Extend)
      .setProjectionMatrix(Matrix3::projection(Vector2(GL::defaultFramebuffer.viewport().size())))
      .setViewport(GL::defaultFramebuffer.viewport().size());
}

void Application::drawEvent() {
  GL::defaultFramebuffer.clear(GL::FramebufferClear::Color);

  camera->draw(drawables);

  swapBuffers();
}
