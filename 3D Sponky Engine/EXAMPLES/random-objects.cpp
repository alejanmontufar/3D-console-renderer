#include <iostream>
#include <string>
#include <conio.h>
#include <chrono>
#include <thread>

#include "../SpEngine/SpConsole.h"

using namespace capp;

int main() {
    Graphics gfx;
    Cursor c;
    GraphicsUI ui(&gfx, c);

    Object obj;
    obj.mesh = Mesh::cubeMesh();

    Object obj2;
    obj2.mesh = Mesh::cubeMesh();

    Object sphere;
    sphere.mesh = Mesh::sphereMesh(3, 64, 128);
    sphere.collider.create(SPHERECOLLIDER, 3);
    sphere.rigidbody.mass = 1;

    Object sphere5;
    sphere5.mesh = Mesh::sphereMesh(2,8,16);
    sphere5.collider.create(SPHERECOLLIDER, 2);
    sphere5.rigidbody.mass = 0.5f;

    Object sphere2;
    sphere2.mesh = Mesh::cylinderMesh(1);

    Object sphere3;
    sphere3.mesh = Mesh::sphereMesh(3, 64, 64);
    
    gfx.Objects.add(obj, "cube1");
    gfx.Objects.add(obj2, "cube2");
    gfx.Objects.add(sphere, "s");
    gfx.Objects.add(sphere2, "s2");
    gfx.Objects.add(sphere5, "sex");

    float camera_x = 0;
    float camera_z = -10;
    float view_x = 0;
    float sensitivy = 0.0005;

    obj.scale = {1,1,1};
    obj2.scale = {10,2,10};
    obj2.transform.newTranslate({0, -2, 0});
    sphere.transform.newTranslate({20, 1, 20});
    sphere2.transform.newTranslate({20, 1, 30});
    sphere3.transform.newTranslate({20, 1, 40});
    sphere5.transform.newTranslate({30, 1, 30});

    while (true) {
        gfx.clearBuffer();
        obj.transform.rotate({0.5f * Time::deltaTime, 0.5f * Time::deltaTime, 0});
        ui.normalLabel({0.05, 0.05, -1}, "fps: " + std::to_string((int)Time::fps) +
            " |");
        ui.normalLabel({0.01, 0.01, -1}, "Acceleration: " + std::to_string(sphere5.rigidbody.acceleration.magnitude()));

        if(Input::keyDown(Input::Keys::right_mouse_button)) {
            Input::lockMouse(true);
            gfx.MainCamera.firstPersonRotation(0.0005, Input::getMouseDeltaX(), Input::getMouseDeltaY());
        } if(Input::keyUp(Input::Keys::right_mouse_button)) Input::lockMouse(false);

        float speed = 10 * Time::deltaTime;

        if(Input::keyDown(Input::Keys::W)) gfx.MainCamera.moveForward(speed);
        if(Input::keyDown(Input::Keys::S)) gfx.MainCamera.moveForward(speed, 1);
        if(Input::keyDown(Input::Keys::D)) gfx.MainCamera.moveRight(speed);
        if(Input::keyDown(Input::Keys::A)) gfx.MainCamera.moveRight(speed, -1);
        if(Input::keyDown(Input::Keys::F)) sphere5.applyForceAcc({-100,0,-100});

        ui.normalLabel({0.05, 0.1, 1}, "triangles: " + std::to_string(gfx.renderObjects().triangles));
        ui.normalLabel({0.05, 0.9, 1}, "Render ms: " + std::to_string(Time::deltaTime * 1000));
        gfx.constructFrame();
    }
    return 0;
}