#include <iostream>
#include <string>
#include <conio.h>
#include <chrono>
#include <thread>
#include <random>

#include "../SpEngine/SpConsole.h"

using namespace capp;

int main() {
    Graphics gfx;
    Cursor c;
    GraphicsUI ui(&gfx, c);

    pos2 cursor = {gfx.getWidth() / 2, gfx.getHeight() / 2};

    Object a(Mesh::sphereMesh(63));
    a.rigidbody.mass = 1e10;
    a.rigidbody.restitution = 0.0f;
    a.collider.create(SPHERECOLLIDER, 63);

    Object b(Mesh::sphereMesh(17));
    b.rigidbody.mass = 10000;
    b.collider.create(SPHERECOLLIDER, 17);


    Object d(Mesh::sphereMesh(10));
    d.rigidbody.mass = 1e4;
    
    gfx.Objects.add(a, "big");
    gfx.Objects.add(b, "Chiquit");

    float camera_x = 0;
    float camera_z = -10;
    float view_x = 0;
    float sensitivy = 0.0005;

    a.position = {0,0,0};
    b.position = {650, 0, 0};
    d.position = {6, 0, 0};
    d.is_kinematic = true;

    while (true) {
        gfx.clearBuffer();
        ui.normalLabel({0.05, 0.05, -1}, "fps: " + std::to_string((int)Time::fps) +
            " |");
            
        if(Input::keyDown(Input::Keys::right_mouse_button)) {
            Input::lockMouse(true);
            gfx.MainCamera.firstPersonRotation(0.0005, Input::getMouseDeltaX(), Input::getMouseDeltaY());
        } if(Input::keyUp(Input::Keys::right_mouse_button)) Input::lockMouse(false);

        float speed = 100 * Time::deltaTime;

        if(Input::keyDown(Input::Keys::W)) gfx.MainCamera.moveForward(speed);
        if(Input::keyDown(Input::Keys::S)) gfx.MainCamera.moveForward(speed, 1);
        if(Input::keyDown(Input::Keys::D)) gfx.MainCamera.moveRight(speed);
        if(Input::keyDown(Input::Keys::A)) gfx.MainCamera.moveRight(speed, -1);
        if(Input::keyDown(Input::Keys::F)) b.applyForce({-5000,0,-5000});
        if(Input::keyDown(Input::Keys::Q)) a.rigidbody.mass *= 1.1;

        ui.normalLabel({0.05, 0.9, 1}, "Render ms: " + std::to_string(Time::deltaTime * 1000));
        gfx.renderObjects();
        gfx.constructFrame();
    }
    return 0;
}