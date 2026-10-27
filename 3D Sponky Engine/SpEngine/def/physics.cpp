#include "../SpConsole.h"

namespace capp {

float Physics::G = 6.674*1e-11;

#pragma region Force

void Object::applyForceAcc(const vec3 &force) {
    rigidbody.acceleration += force / rigidbody.mass;
}

void Object::applyImpulse(const vec3 &impulse) {
    rigidbody.velocity += impulse / rigidbody.mass;
}

void Object::applyForce(const vec3 &force) {
    rigidbody.velocity += force / rigidbody.mass;
}

#pragma endregion

};