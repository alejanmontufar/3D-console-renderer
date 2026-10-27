#include "../SpConsole.h"

namespace capp {
    int Debug::logs = 0;
    float Time::deltaTime = 0;
    float Time::fps = 0;
    float Time::worst_fps = 1e9f;
    float Time::best_fps = 0;
    std::chrono::high_resolution_clock::time_point Time::last_time = std::chrono::high_resolution_clock::now();

    void Time::update() {
        auto now_time = std::chrono::high_resolution_clock::now();
        deltaTime = std::chrono::duration<float>(now_time-last_time).count();
        last_time = now_time;
        fps = 1 / deltaTime;
        if(fps < worst_fps) worst_fps = fps;
        if(fps > best_fps) best_fps = fps;
    }

    void Debug::Log(std::string log, std::string title) {
        std::ofstream f;
        
        if(logs==0) {
            f.open("Debug.txt", std::ios::out | std::ios::trunc);
            f << "### Application Debug ###\n";
        } else  f.open("Debug.txt", std::ios::app);
        
        f << "\n(Debug, " << logs++ << "): " << title << ": " << log;
        f.close();
    }

    void Debug::Error(std::string log, int err_code) {
        std::ofstream f;
        f.open("Debug.txt", std::ios::app);
        f << "\n(Error!, code(" << err_code << ")): " << log;
        f.close();
    }
};