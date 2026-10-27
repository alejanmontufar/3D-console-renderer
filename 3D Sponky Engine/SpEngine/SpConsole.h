/*

    Sponky's engine lib

    This is a mini engine (3d engine but has other nice features) that works
    mostly in Windows console, but with some configurations it can run in
    any terminal, Linux, macOs, every one of them unless they cannot read
    ANSI escape.

    This lib has lots of features to enjoy!

*/


#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <array>
#include <chrono>
#include <fstream>
#include <functional>
#include <math.h>
#include <windows.h>
#include <wincon.h>
#include <cstdio>
#include <cstdlib>
#include <tchar.h>

#ifndef CONSOLE_APPLICATION_SPONKY
#define CONSOLE_APPLICATION_SPONKY

namespace capp {
/*
    Sponky 2019 copyright *
    "capp" stands for: Console Application
*/


#ifndef ENABLE_BACKFACE_CULLING
    #define ENABLE_BACKFACE_CULLING true
#endif


#ifndef Math_PI
    #define Math_PI 3.14159265359
#endif
#ifndef min
    #define min(a, b) ((a) < (b) ? (a) : (b))
#endif
#ifndef max
    #define max(a, b) ((a) > (b) ? (a) : (b))
#endif
#ifndef clamp
    #define clamp(v, lo, hi) ((v < lo) ? lo : (v > hi) ? hi : v)
#endif

#define BOXCOLLIDER 1
#define SPHERECOLLIDER 2
#define COLLIDERBOX 1
#define COLLIDERSPHERE 2
#define CURSOR_NORMAL 0
#define CURSOR_HOVER 1
#define CURSOR_CLICK 2
#define CURSOR_UNCLICKABLE 3

    struct color {
        unsigned char r, g, b;
        color() : r(0), g(0), b(0) {}
        color(unsigned char r, unsigned char g, unsigned char b) : r(r), g(g), b(b) {}
        color(unsigned char c) : r(c), g(c), b(c) {}
        unsigned char operator[](const int &i){switch(i){case 0:return r;break;case 1:
            return g;break;case 2:return b; break; default: return 0;}}
    };
    
    struct pos2 {
        int x, y;
        pos2(float x, float y) : x(x), y(y) {}
        pos2(int x, int y) : x(x), y(y) {}
    };

    struct vec2 {
        float x, y;
        vec2() : x(0), y(0) {}
        vec2(float x, float y) : x(x), y(y) {}
        vec2 operator*(float s) {return{x*s, y*s};}
    };

    struct vec3 {
        float x, y, z;
        vec3() : x(0), y(0), z(0) {}
        vec3(float a) : x(a), y(a), z(a) {}
        vec3(float x, float y, float z) : x(x), y(y), z(z) {}
        vec3(color c) : x(c.r), y(c.g), z(c.b) {}
        vec3 operator+(const vec3 &a) const {return{a.x+x, a.y+y, a.z+z};}
        vec3 operator-(const vec3 &a) const {return{x-a.x, y-a.y, z-a.z};}
        vec3 operator-() {return{-x, -y, -z};}
        vec3 operator*(float s) {return{x*s, y*s, z*s};}
        vec3 operator/(float s) const {return{x/s, y/s, z/s};}
        vec3 cross(const vec3& b) const {return {y*b.z-z*b.y,z*b.x-x*b.z,x*b.y-y*b.x};}
        vec3 normalized() const {float m = std::sqrt(x*x + y*y + z*z);if (m == 0) return {0,0,0};
            return {x/m, y/m, z/m};}
        float operator*(const vec3 &a) {return a.x*x+a.y*y+a.z*z;}
        float dot(const vec3 &a) {return operator*(a);}
        float length() const {return sqrtf(x*x + y*y + z*z);}
        float magnitude() const {return length();}
        void operator+=(const vec3 &a) {x += a.x; y += a.y; z += a.z;}
        void operator-=(const vec3 &a) {x -= a.x; y -= a.y; z -= a.z;}
        void operator*=(const vec3 &a) {x *= a.x; y *= a.y; z *= a.z;}
        void operator/=(const vec3 &a) {x /= a.x; y /= a.y; z /= a.z;}
        bool operator<(const float &a) {return x < a && y < a && z < a;}
        bool operator<=(const float &a) {return x<=a && y<= a && z <=a;}
        bool operator>(const float &a) {return !this<a;}
        bool operator>=(const float &a) {return !this<=a;}
    };
    
    struct vec4 {
        float x, y, z, w;
        vec4() : x(0), y(0), z(0), w(1) {}
        vec4(vec3 v, float w=1.0f) : x(v.x), y(v.y), z(v.z), w(w) {}
        vec4(float x, float y, float z, float w=1.0f) : x(x), y(y), z(z), w(w) {}
        vec4 operator+(const vec4 &a) {return{a.x+x,a.y+y,a.z+z,a.w+w};}
        vec4 operator-(const vec4 &a) {return{x-a.x,y-a.y,z-a.z,w-a.w};}
        vec4 operator-() {return{-x,-y,-z,-w};}
        vec4 operator*(float s) {return{x*s, y*s, z*s, w*s};}
        float operator*(const vec4 &a) {return{a.x*x+a.y*y+a.z*z+a.w*w};}
        vec4 quat_mul(vec4 b) {return{w*b.x+x*b.w+y*b.z-z*b.y,w*b.y-x*b.z+y*b.w+z*b.x,
        w*b.z+x*b.y-y*b.x+z*b.w,w*b.w-x*b.x-y*b.y-z*b.z};}
        vec4 normalized() const {float m = sqrtf(x*x + y*y + z*z + w*w);if (m == 0) return {0,0,0,1};
            return {x/m, y/m, z/m, w/m};}
        vec3 xyz() {return{x,y,z};}
    };

    struct mat4 {
        float matrix[4][4];
        mat4 operator*(const mat4 &a);
        vec4 operator*(const vec4 &v) const;
        mat4 transpose();
        mat4 inverse();
        static mat4 identity();
        static mat4 rotateX(float angle);
        static mat4 rotateY(float angle);
        static mat4 rotateZ(float angle);
        static mat4 perspective(float fov, float aspect, float znear, float zfar);
        static mat4 translate(float x, float y, float z);
        static mat4 scale(float sx, float sy, float sz);
    };

    struct mat3 {
        float matrix[3][3];
        mat3() {}
        mat3(const mat4&);
        vec3 operator*(const vec3 &v);
    };
    
    struct Cursor {
        vec2 pos {40, 20};
        float sensitivity = 0.5f;
        unsigned int state = 0;
        int hover = false;
    };

    class Time {
        private:
            friend class Graphics;
            static std::chrono::high_resolution_clock::time_point last_time;
            static void update();
        
        public:
            static float deltaTime;
            static float fps;
            static float best_fps;
            static float worst_fps;
    };

    class Debug {
        private: static int logs;
        public: static void Log(std::string log, std::string title = "INFO");
        public: static void Error(std::string log, int err_code);
        public: static void ErrorEnd(std::string log, int err_code) {Error(log, err_code); exit(err_code);}
    };

    #ifndef _WIN32
        #warning If not using windows, input management might become a harder quest
    #else
    class Input {
        private:
            friend class Graphics;
            static bool prev_keymap[256];
            static bool curr_keymap[256];
            static POINT mouse[3]; /* 0: current, 1: previous, 2: delta */
            static POINT mouse_pos_when_not_centered;
            static int mouse_wheel;
            static bool locked_mouse;
            static bool mouse_centered;
            static void update();  
        
        public: class Keys { public: enum KeyCodes {
            null = 0,
            left_mouse_button, right_mouse_button, middle_mouse_button = 4,
            button4_mouse, button5_mouse,
            backspace = 8, tab, enter = 12,
            shift = 16, ctrl, alt,
            caps_lock = 20, esc = 27, space = 32,
            left_arrow = 25, up_arrow, right_arrow, down_arrow,
            del = 46,
            n0 = 48, n1, n2, n3, n4, n5, n6, n7, n8, n9,
            A = 65, B, C, D, E, F, G, H, I, J, K, L, M,
            N, O, P, Q, R, S, T, U, V, W, X, Y, Z
        };};

        public:
            static bool keyDown(int key);
            static bool keyPress(int key);
            static bool keyUp(int key);
            static bool mouseClick(int button);
            static int getMouseDeltaX();
            static int getMouseDeltaY();
            static int getMouseScroll();
            static void lockMouseCenter(bool enabled);
            static void lockMousePosition(bool enabled, pos2 position);
            static void lockMouse(bool enabled);
            static pos2 getMousePosition();
    };
    #endif

    struct Texture {
        int width {0};
        int height {0};
        bool using_texture {false};
        std::vector<color> data = {};
        void loadColorArray(const std::vector<std::vector<color>> &d, bool bind_texture = true);
        color sample(float u, float v);
    };

    class Mesh {
        private:
            friend class Graphics;
        public:
            std::vector<vec3> vertices;
            std::vector<uint32_t> indices;
            std::vector<color> colors;
            std::vector<vec2> uvs;
            std::vector<vec3> normals;
            Texture texture;
        public:
            void createNormals();

            static Mesh cubeMesh();
            static Mesh sphereMesh(float radius, uint8_t stacks=16, uint8_t slices=32);
            static Mesh cylinderMesh(float radius, float height=2, uint8_t stacks=16);
    };

    class Transform {
        private:
            friend class Graphics;
            vec3 &position;
            vec3 &rotation;
            vec3 &scalation;
        public:
            Transform(vec3& pos, vec3& rot, vec3& scale):position(pos), rotation(rot), scalation(scale) {}
            mat4 getModel();
            void translate(vec3);void newTranslate(vec3);
            void rotate(vec3); void newRotate(vec3);
            void scale(vec3); void newScale(vec3);
    };

    class Collider {
        private:
            friend class Object;
            friend class Graphics;
            vec3 size;
            unsigned int type;
            float as, bs;
        public:
        vec3 &position;

        Collider(vec3 &pos) : position(pos), size(1), type(0) {}
        void create(unsigned int type, float radius=1) {this->type = type; as = radius;}
        void create(unsigned int type, float base, float height)
            {this->type=type, as=base, bs=height;}
        bool collidesWith(const Collider &collider);
        void setSize(vec3 scale, float multiplier=1)
            {size = {scale.x*multiplier, scale.y*multiplier, scale.z*multiplier};}
        
        static const unsigned int Box = 1;
        static const unsigned int Sphere = 2;
    };

    struct RigidBody {
        vec3 velocity={0,0,0}; // m/s
        vec3 acceleration={0,0,0}; // m/(s^2)
        float mass=1; // undefined unit
        bool affected_by_gravity = false;
        float gravity = 9.81f;
        float restitution = 0.5f;
    };

    struct Object {
        vec3 position { 0,0,0 }, rotation { 0,0,0 }, scale { 1,1,1 };

        Mesh mesh;
        Transform transform;
        Collider collider;
        RigidBody rigidbody;
        bool is_kinematic = false;

        Object() : transform(position, rotation, scale), collider(position) {}
        Object(Mesh mesh) : mesh(mesh), transform(position, rotation, scale),
            collider(position) {this->mesh.createNormals();}
        bool collidesWith(const Object &obj) {return collider.collidesWith(obj.collider);}
        bool canCollide(bool set) {collider.type = set ? 2 : 0;}
        void applyForceAcc(const vec3 &force);
        void applyForce(const vec3 &force);
        void applyImpulse(const vec3 &impulse);
    };

    class Camera {
        private:
            friend class Graphics;
            mat4 proj;
            void setProjectionMatrix(int width, int height);
            vec3 rotateVecByQuat(const vec3 &v, const vec4 &q) const;
            vec4 quatFromAxisAngle(vec3 axis, float angle);
            void update();
        public:
            float fov;
            float yaw { 0.0f }, pitch { 0.0f };
            vec3 position = { 0, 0, -10 };
            float znear { .5f }, zfar { 100.f };
            vec4 rotation;
            Camera() : fov(70.0f) {}
            mat4 getViewMatrix() const;
            void rotate(float dYaw, float dPitch);
            void rotate(vec2 yaw_pitch) {rotate(yaw_pitch.x, yaw_pitch.y);}
            void move(vec3 dir, float speed);
            void moveForward(float speed, float d=-1);
            void moveRight(float speed, float d=1);
            void moveUp(float speed, float d=1);
            void firstPersonRotation(float sensitivity, float mouse_delta_x, float mouse_delta_y);
            void firstPersonRotation(float,float,float,float);
            void firstPersonRotation(vec2 sensitivity, vec2 mouse_deltas);
    };

    template <class ObjType>
    class ObjMap {
        private:
            friend class Graphics;
            std::unordered_map<char*, ObjType*> map;
        public:
            void add(ObjType &obj, const char *key) { map.insert({(char*)key, &obj}); }
            void destroy(const char *key) { map.erase((char*)key); }
            ObjType &operator[](const char *key) { return *map.at((char*)key); }
    };

    class Graphics {
        private:
            friend class GraphicsUI;

            struct char_pix {
                char c;
                bool u=false; };
            struct out_info {
                unsigned int vertices=0, indices=0, objects=0, triangles=0;
                int getTriangleCount() {return indices / 3;} };
                
            int columns, rows, pix_height;
            int cycles { 0 };
            std::vector<vec3> vertices;
            std::vector<vec3> normals;
            std::vector<color> colors;
            std::vector<uint32_t> indices;
            std::vector<vec2> uvs;
            std::vector<unsigned char> frame_buffer;
            std::vector<float> z_buffer;
            std::vector<char_pix> char_buffer;
            std::vector<std::function<void()>> internal_updates;

            Texture texture;
            vec3 light_dir_view;
            bool using_ui {false};
            bool using_light {true};

            #ifdef _WIN32
                HANDLE hConsole;
                void setAutoBufferSize();
                bool resizedWindow();
            #endif
            int fillTriangle(std::array<pos2, 3>, std::array<float, 3>,
                std::array<float, 3>, std::array<vec3, 3>, std::array<vec2, 3>,
                std::array<vec3,3>, const vec3 &);
            void fillLine(pos2, pos2, color);
            void drawDebugCircleView(const vec3&,float,int,const mat4&,uint8_t, uint8_t, uint8_t);
            int renderInBuffer(mat4 model, mat4 view, mat4 proj);
            int renderMesh(const Mesh &mesh, mat4 model);
        
        public:
            ObjMap<Object> Objects;
            Camera MainCamera;
            float ambient = 0.05f;
            vec3 light_dir = vec3{0.3f, 1.0f, -0.5f};

            #ifdef _WIN32
                Graphics();
            #else
                Graphics(int width, int height /* In pixels */);
            #endif
            void pixel(int x, int y, float z, unsigned char, unsigned char, unsigned char);
            void clearBuffer() {frame_buffer.assign(columns*pix_height*3,0);
                z_buffer.assign(columns*pix_height, 1e9f);char_buffer.assign(columns*rows,{0,false});}
            void constructFrame();
            out_info renderObjects();
            int getWidth() {return this->columns;}
            int getHeight() {return this->rows;}
    };

    class GraphicsUI {
        private:
            static std::array<std::string, 26> pabc1; // Printable abc
            static std::array<std::string, 10> pnums1; // Printable nums
            static std::vector<std::string> pspecial1; // Printable special chars
            Graphics *gfx;
            void pixel_ui(int x, int y, float z, unsigned char, unsigned char, unsigned char);

        public:
            struct Image {
                int width;
                int height;
                std::vector<color> pixels;
                std::vector<bool> transparency;

                static Image load(std::string src);
            };

            int ui_hot;
            int ui_active;
            Cursor &cursor;

            GraphicsUI(Graphics *graphics, Cursor &cursor) :
                gfx(graphics), cursor(cursor) {gfx->using_ui = true;}
            void rectanglef/* Coordinates like 0.1f, 0.3f*/
                (vec3 position, vec2 size, color col={ 204,204,204 });
            void rectanglei/* Coordinates like 40, 50*/
                (vec3 position, vec2 size, color col={ 204,204,204 });
            bool button(vec3 position, std::string text, float width, float height,
                color col, color bg_color, color hover_color, color active_color);
            bool buttonImg(vec3 position, const Image& img,
                color tint_normal, color tint_hover, color tint_active);
            bool buttonImgLabel(
                vec3 position,
                const Image& img,
                const std::string& text,
                color text_color,
                color tint_normal,
                color tint_hover,
                color tint_active
            );
            void drawImage(vec3 position, const Image &img);
            void label(vec3 position, std::string text,
                color text_c={ 204,204,204 }, int space_between_letters=1);
            void normalLabel(vec3 position, std::string text,
                color text_c={ 204,204,204 });
            void normalLabeli(vec3 position, std::string text,
                color text_c={ 204,204,204 });
            void printChar(vec2 position, char c, color text_c={204,204,204}, float z = 0.1f);
            void Update();
    };

    const color RED = { 255, 0, 0 };
    const color GREEN = { 0, 255, 0 };
    const color BLUE = { 0, 0, 255 };
    const color NEUTRAL = { 204, 204, 204 };

    class Physics {
        public:
            static float G;
    };
};

#endif