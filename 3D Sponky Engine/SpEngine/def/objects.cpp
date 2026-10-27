#include "../SpConsole.h"

namespace capp {

#pragma region Textures

color Texture::sample(float u, float v) {
    u = min(max(u, 0.0f), 1.0f);
    v = min(max(v, 0.0f), 1.0f);

    int x = int(u * (width - 1));
    int y = int((1.0f - v) * (height - 1));

    return data[y * width + x];
}

void Texture::loadColorArray(const std::vector<std::vector<color>> &d, bool bind_texture) {
    width = d.size() > 0 ? d[0].size() : 0;
    height = d.size();
    data.resize(width * height);

    for(int j = 0; j < height; j++) for(int i = 0; i < width; i++) {
        data[j * width + i] = d[j][i];
    }

    using_texture = bind_texture;
}

#pragma endregion
#pragma region Meshe

void Mesh::createNormals() {
    normals.resize(vertices.size(), vec3(0,0,0));

    for (size_t i = 0; i < indices.size(); i += 3) {
        uint32_t i0 = indices[i];
        uint32_t i1 = indices[i+1];
        uint32_t i2 = indices[i+2];

        vec3 p0 = vertices[i0];
        vec3 p1 = vertices[i1];
        vec3 p2 = vertices[i2];

        vec3 face_normal =
            (p1 - p0).cross(p2 - p0).normalized();

        normals[i0] = normals[i0] + face_normal;
        normals[i1] = normals[i1] + face_normal;
        normals[i2] = normals[i2] + face_normal;
    }

    for (auto &n : normals)
        n = n.normalized();
}

Mesh Mesh::cubeMesh() {
    Mesh m;
    
    m.vertices = {
        {-1,-1,-1}, {1,-1,-1}, {1,1,-1}, {-1,1,-1},
        {-1,-1, 1}, {1,-1, 1}, {1,1, 1}, {-1,1, 1},
        {-1,-1,-1}, {-1,1,-1}, {-1,1, 1}, {-1,-1, 1},
        {1,-1,-1}, {1,1,-1}, {1,1, 1}, {1,-1, 1},
        {-1,1,-1}, {1,1,-1}, {1,1, 1}, {-1,1, 1},
        {-1,-1,-1}, {1,-1,-1}, {1,-1, 1}, {-1,-1, 1}};
    m.indices = {
        0,1,2,  0,2,3,
        4,6,5,  4,7,6,
        8,9,10,  8,10,11,
        12,14,13, 12,15,14,
        16,17,18, 16,18,19,
        20,22,21, 20,23,22};
    m.uvs = {
        {0,0},{1,0},{1,1},{0,1},
        {0,0},{1,0},{1,1},{0,1},
        {0,0},{1,0},{1,1},{0,1},
        {0,0},{1,0},{1,1},{0,1},
        {0,0},{1,0},{1,1},{0,1},
        {0,0},{1,0},{1,1},{0,1}};

    m.createNormals();

    return m;
}

Mesh Mesh::sphereMesh(float radius, uint8_t stacks, uint8_t slices) {
    Mesh m;

    for (int i = 0; i <= stacks; i++) {
        float v = (float)i / stacks;
        float phi = v * Math_PI;

        for (int j = 0; j <= slices; j++) {
            float u = (float)j / slices;
            float theta = u * 2.0f * Math_PI;

            float x = radius * sin(phi) * cos(theta);
            float y = radius * cos(phi);
            float z = radius * sin(phi) * sin(theta);

            m.vertices.push_back({ x, y, z });
            m.uvs.push_back({ u, 1.0f - v }); 
        }
    }

    for (int i = 0; i < stacks; i++) {
        for (int j = 0; j < slices; j++) {
            int first  = i * (slices + 1) + j;
            int second = first + slices + 1;

            m.indices.push_back(first);
            m.indices.push_back(second);
            m.indices.push_back(first + 1);

            m.indices.push_back(second);
            m.indices.push_back(second + 1);
            m.indices.push_back(first + 1);
        }
    }

    m.createNormals();
    return m;
}

Mesh Mesh::cylinderMesh(float radius, float height, uint8_t slices) {
    Mesh m;

    float halfH = height * 0.5f;

    for (int i = 0; i <= slices; ++i) {
        float u = (float)i / slices;
        float theta = u * 2.0f * Math_PI;

        float x = cos(theta) * radius;
        float z = sin(theta) * radius;

        m.vertices.push_back({ x, -halfH, z });
        m.uvs.push_back({ u, 0.0f });

        m.vertices.push_back({ x, +halfH, z });
        m.uvs.push_back({ u, 1.0f });
    }

    for (int i = 0; i < slices; ++i) {
        int base = i * 2;

        m.indices.push_back(base);
        m.indices.push_back(base + 2);
        m.indices.push_back(base + 1);
        m.indices.push_back(base + 1);
        m.indices.push_back(base + 2);
        m.indices.push_back(base + 3);
    }

    int topCenterIndex = m.vertices.size();
    m.vertices.push_back({ 0, +halfH, 0 });
    m.uvs.push_back({ 0.5f, 0.5f });

    int topStart = m.vertices.size();

    for (int i = 0; i <= slices; ++i) {
        float u = (float)i / slices;
        float theta = u * 2.0f * Math_PI;

        float x = cos(theta) * radius;
        float z = sin(theta) * radius;

        m.vertices.push_back({ x, +halfH, z });
        m.uvs.push_back({
            0.5f + x / (2 * radius),
            0.5f + z / (2 * radius)
        });
    }

    for (int i = 0; i < slices; ++i) {
        m.indices.push_back(topCenterIndex);
        m.indices.push_back(topStart + i);
        m.indices.push_back(topStart + i + 1);
    }

    int bottomCenterIndex = m.vertices.size();
    m.vertices.push_back({ 0, -halfH, 0 });
    m.uvs.push_back({ 0.5f, 0.5f });

    int bottomStart = m.vertices.size();

    for (int i = 0; i <= slices; ++i) {
        float u = (float)i / slices;
        float theta = u * 2.0f * Math_PI;

        float x = cos(theta) * radius;
        float z = sin(theta) * radius;

        m.vertices.push_back({ x, -halfH, z });
        m.uvs.push_back({
            0.5f + x / (2 * radius),
            0.5f + z / (2 * radius)
        });
    }

    for (int i = 0; i < slices; ++i) {
        m.indices.push_back(bottomCenterIndex);
        m.indices.push_back(bottomStart + i + 1);
        m.indices.push_back(bottomStart + i);
    }

    m.createNormals();
    return m;
}

#pragma endregion
#pragma region Transform

mat4 Transform::getModel() {
    mat4 model;
    model = mat4::translate(position.x, position.y, position.z) *
            (mat4::rotateZ(rotation.z) *
            mat4::rotateY(rotation.y) * 
            mat4::rotateX(rotation.x)) *
            mat4::scale(scalation.x, scalation.y, scalation.z);
    return model;
}

void Transform::translate(vec3 v)
    {position = position + v;}
void Transform::rotate(vec3 v)
    {rotation = rotation + v;}
void Transform::scale(vec3 v) 
    {scalation = scalation + v;}
void Transform::newTranslate(vec3 v) 
    {position = v;}
void Transform::newRotate(vec3 v) 
    {rotation = v;}
void Transform::newScale(vec3 v)
    {scalation = v;}

#pragma endregion
#pragma region Camera

void Camera::update() {}

void Camera::setProjectionMatrix(int width, int height) {
    proj = mat4::perspective(
        fov,
        width / (float)height,
        znear,
        zfar
    );
}

mat4 Camera::getViewMatrix() const {
    vec3 f = rotateVecByQuat({0, 0, -1},rotation);
    vec3 r = rotateVecByQuat({1, 0, 0},rotation);
    vec3 u = rotateVecByQuat({0, 1, 0},rotation);

    mat4 view = mat4::identity();
    
    view.matrix[0][0] =  r.x;
    view.matrix[0][1] =  r.y;
    view.matrix[0][2] =  r.z;

    view.matrix[1][0] =  u.x;
    view.matrix[1][1] =  u.y;
    view.matrix[1][2] =  u.z;

    view.matrix[2][0] = -f.x;
    view.matrix[2][1] = -f.y;
    view.matrix[2][2] = -f.z;

    view.matrix[0][3] = -(r * position);
    view.matrix[1][3] = -(u * position);
    view.matrix[2][3] =  (f * position);

    return view;
}

vec4 Camera::quatFromAxisAngle(vec3 axis, float angle) {
    axis = axis.normalized();
    float s = sinf(angle * 0.5f);

    return {
        axis.x * s,
        axis.y * s,
        axis.z * s,
        cosf(angle * 0.5f)
    };
}

vec3 Camera::rotateVecByQuat(const vec3& v, const vec4& q) const {
    vec3 u = { q.x, q.y, q.z };
    float s = q.w;

    return
        u * (2.0f * (u * v)) +
        vec3{v.x * (s*s - (u * u)), v.y * (s*s - (u * u)), v.z * (s*s - (u * u))} +
        (u.cross(v)) * (2.0f * s);
}

void Camera::rotate(float dYaw, float dPitch) {
    yaw += dYaw;
    pitch += dPitch;
    vec4 qYaw = quatFromAxisAngle({0,1,0}, dYaw);
    vec4 qPitch = quatFromAxisAngle({1,0,0}, dPitch);

    rotation = qYaw.quat_mul(rotation);
    rotation = rotation.quat_mul(qPitch);
    rotation = rotation.normalized();
}

void Camera::move(vec3 dir, float speed) {
    position = position + dir * speed;
}

void Camera::moveForward(float speed, float d)
{move(rotateVecByQuat({0, 0, d},rotation).normalized(), speed);}
void Camera::moveRight(float speed, float d)
{move(rotateVecByQuat({d, 0, 0},rotation).normalized(), speed);}
void Camera::moveUp(float speed, float d)
{move(rotateVecByQuat({0, d, 0},rotation).normalized(), speed);}
void Camera::firstPersonRotation(float sensitivity, float mouse_delta_x, float mouse_delta_y)
{rotate(-mouse_delta_x * sensitivity, -mouse_delta_y * sensitivity);}
void Camera::firstPersonRotation(float sensitivity_x, float sensitivity_y, float mouse_delta_x,
float mouse_delta_y){rotate(-mouse_delta_x * sensitivity_x, -mouse_delta_y * sensitivity_y);}
void Camera::firstPersonRotation(vec2 sensitivity, vec2 mouse_deltas)
{rotate(-mouse_deltas.x * sensitivity.x, -mouse_deltas.y * sensitivity.y);}

#pragma endregion
#pragma region Collider

bool Collider::collidesWith(const Collider &collider) {
    if(collider.type == SPHERECOLLIDER && type == SPHERECOLLIDER) {
        vec3 delta = this->position - collider.position;
        delta *= delta;
        float distance = sqrtf(delta.x + delta.y + delta.z);
        return distance <= (as + collider.as);
    }
}

#pragma endregion

};