#include "../SpConsole.h"

namespace capp {

#ifdef _WIN32
    bool Input::prev_keymap[256];
    bool Input::curr_keymap[256];
    POINT Input::mouse[3] = {{},{},{}};
    POINT Input::mouse_pos_when_not_centered = {};
    int Input::mouse_wheel = 0;
    bool Input::locked_mouse = false;
    bool Input::mouse_centered = false;

    void Input::update() {
        mouse[1] = mouse[0]; // Previous = Current
        memcpy(prev_keymap, curr_keymap, sizeof(curr_keymap));
        mouse_wheel = 0;

        GetCursorPos(&mouse[0]);

        // Mouse deltas
        mouse[2].x = mouse[0].x - mouse[1].x;
        mouse[2].y = mouse[0].y - mouse[1].y;

        for(int i = 0; i < 256; i++)
            curr_keymap[i] = (GetAsyncKeyState(i) & 0x8000) != 0;

        if(locked_mouse) {
            POINT center;
            if(!mouse_centered) center=mouse_pos_when_not_centered;
            else center.x=GetSystemMetrics(SM_CXSCREEN)/2,center.y=GetSystemMetrics(SM_CYSCREEN)/2;
            SetCursorPos(center.x, center.y);
            mouse[0] = center;
        }
    }

    bool Input::keyDown(int key) {return curr_keymap[key];}
    bool Input::keyPress(int key) {return curr_keymap[key] && !prev_keymap[key];}
    bool Input::keyUp(int key) {return !curr_keymap[key] && prev_keymap[key];}
    
    bool Input::mouseClick(int button) {
        int key_id;

        switch(button) {
            case 1: key_id = 1; break;
            case 2: key_id = 2; break;
            case 3: key_id = 4; break;
            case 4: key_id = 5; break;
            case 5: key_id = 6; break;
            default:
                std::cerr<<"There are only 5 mouse buttons! (1 - 5)";
                return 0;
        }

        return keyPress(key_id);
    }

    int Input::getMouseDeltaX() { return mouse[2].x; }
    int Input::getMouseDeltaY() { return mouse[2].y; }
    int Input::getMouseScroll() { return mouse_wheel; }
    void Input::lockMouseCenter(bool enabled) { locked_mouse = enabled,mouse_centered=true; }
    void Input::lockMousePosition(bool enabled, pos2 pos) {
        locked_mouse = enabled;
        mouse_pos_when_not_centered = {pos.x, pos.y};
        mouse_centered = false;
    }
    void Input::lockMouse(bool enabled) {
        mouse_centered=false;
        if(enabled && !locked_mouse)
            GetCursorPos(&mouse_pos_when_not_centered);
        locked_mouse=enabled;
    }
    pos2 Input::getMousePosition() { return { (int)mouse[0].x, (int)mouse[0].y }; }
#endif

};