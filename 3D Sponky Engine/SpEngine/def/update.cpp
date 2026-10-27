#include "../SpConsole.h"

namespace capp {

Graphics::out_info Graphics::renderObjects() {
    out_info out;
    std::vector<Object*> colliders;
    MainCamera.update();
    Time::update();
    Input::update();

    for (const auto &f : internal_updates)
        f();

    for(auto it = Objects.map.begin(); it != Objects.map.end(); ++it) {
        auto &obj = *it->second;
        if(obj.collider.type != 0) colliders.push_back(&obj);
    }

    /*
        Renders all of the objects in the world

    */

    if(this->resizedWindow()) setAutoBufferSize();

    mat4 proj = MainCamera.proj;
    mat4 view = MainCamera.getViewMatrix();
    
    for(auto it = Objects.map.begin(); it != Objects.map.end(); ++it) {
        auto &obj = *it->second;
        mat4 model = obj.transform.getModel();

        // Physics
        if (!obj.is_kinematic) {
            vec3 totalForce = {0, 0, 0};

            for (auto at = Objects.map.begin(); at != Objects.map.end(); ++at) {
                auto* other = at->second;

                if (other == &obj) continue; // avoid auto-interaction

                vec3 delta = other->position - obj.position;

                // Softening
                float epsilon = 0.001f;
                float distSq = delta.x*delta.x + delta.y*delta.y + delta.z*delta.z + epsilon;

                float distance = sqrtf(distSq);

                // normal dir
                vec3 direction = delta / distance;

                // correct magnittude (1 / r^2)
                float forceMagnitude =
                    Physics::G * obj.rigidbody.mass * other->rigidbody.mass / distSq;

                totalForce += direction * forceMagnitude;
            }

            obj.applyForce(totalForce);

            obj.rigidbody.velocity += obj.rigidbody.acceleration * Time::deltaTime;
            obj.transform.translate(obj.rigidbody.velocity * Time::deltaTime);

            obj.rigidbody.acceleration = {0, 0, 0};
        }

        if(obj.collider.type != 0) for(auto &coll: colliders) {
            if(obj.collidesWith(*coll)) {
                vec3 normal = (obj.transform.position - coll->transform.position).normalized();

                vec3 relativeVelocity = obj.rigidbody.velocity - coll->rigidbody.velocity;
                float velAlongNormal = relativeVelocity.dot(normal);

                if(velAlongNormal > 0) continue;

                float restitution = min(obj.rigidbody.restitution, coll->rigidbody.restitution);

                float j = -(1 + restitution) * velAlongNormal;
                j /= (1/obj.rigidbody.mass + 1/coll->rigidbody.mass);

                // j = -(1 + e)(Vrel * n) / (1/msub1 + 1/msub2)
                vec3 impulse = normal * j;

                if(!obj.is_kinematic) obj.applyImpulse( impulse);
                if(!coll->is_kinematic) coll->applyImpulse(-impulse);
            }
        }

        out.triangles += renderMesh(obj.mesh, model);
        out.vertices += obj.mesh.vertices.size();
        out.indices += obj.mesh.indices.size();
        out.objects++; 
    }

    return out;
}

}