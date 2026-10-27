#include "../SpConsole.h"

namespace capp {


#pragma region Textures

GraphicsUI::Image GraphicsUI::Image::load(std::string src) {
    using std::vector;
    using std::string;

    Image result = {};

    std::ifstream file(src);
    if (!file.is_open()) {
        Debug::Error(src + " was not found!", 404);
        return result;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    file.close();

    auto trim = [](string s) {
        while (!s.empty() && (s[0] == '\n' || s[0] == ' ' || s[0] == '\r'))
            s.erase(0, 1);
        return s;
    };

    auto split = [](const string& s, char delimiter) {
        vector<string> tokens;
        string token;
        std::istringstream ss(s);
        while (std::getline(ss, token, delimiter))
            tokens.push_back(token);
        return tokens;
    };

    auto safe_stoi = [](const string& s) {
        try { return std::stoi(s); }
        catch (...) { return 0; }
    };

    auto clean_num = [](string s) {
        string r;
        for (char c : s)
            if (isdigit(c) || c == '-') r += c;
        return r;
    };

    auto add_pixel = [&](string pix, vector<color>& out) {
        if (pix.empty()) return;

        if (pix[0] == 'a') {
            int v = safe_stoi(clean_num(pix.substr(1)));
            out.push_back({(uint8_t)v, (uint8_t)v, (uint8_t)v});
        }
        else if (pix[0] == 'c') {
            string data = pix.substr(1);
            auto vals = split(data, ',');
            if (vals.size() >= 3) {
                out.push_back({
                    (uint8_t)safe_stoi(clean_num(vals[0])),
                    (uint8_t)safe_stoi(clean_num(vals[1])),
                    (uint8_t)safe_stoi(clean_num(vals[2]))
                });
            }
        }
    };

    vector<string> parts = split(content, '}');
    if (parts.size() < 3) {
        Debug::Error("Invalid file format (sections)", 100);
        return result;
    }

    for (auto& p : parts) p = trim(p);

    auto s = split(parts[0], ':');
    auto p = split(parts[1], ':');
    auto t = split(parts[2], ':');

    if (s.size() < 2 || p.size() < 2 || t.size() < 2) {
        Debug::Error("Invalid file format (headers)", 100);
        return result;
    }

    auto sizes = split(s[1], ';');
    if (sizes.size() < 2) {
        Debug::Error("Invalid size format", 101);
        return result;
    }

    result.width  = safe_stoi(clean_num(sizes[0]));
    result.height = safe_stoi(clean_num(sizes[1]));

    int expected = result.width * result.height;

    auto pixels = split(p[1], ';');

    for (auto& token : pixels) {
        token = trim(token);
        if (token.empty()) continue;

        if (token[0] == 'x') {
            auto info = split(token.substr(1), '-');
            if (info.size() < 2) continue;

            int count = safe_stoi(clean_num(info[0]));
            string value = info[1];

            for (int i = 0; i < count; i++)
                add_pixel(value, result.pixels);
        } else {
            add_pixel(token, result.pixels);
        }
    }

    if (result.pixels.size() < expected)
        result.pixels.resize(expected, {0,0,0});
    if (result.pixels.size() > expected)
        result.pixels.resize(expected);

    auto transp = split(t[1], ';');

    for (auto& token : transp) {
        token = trim(token);
        if (token.empty()) continue;

        if (token[0] == 'x') {
            auto info = split(token.substr(1), '-');
            if (info.size() < 2) continue;

            int count = safe_stoi(clean_num(info[0]));
            int val   = safe_stoi(clean_num(info[1]));

            for (int i = 0; i < count; i++)
                result.transparency.push_back(val);
        } else {
            result.transparency.push_back(safe_stoi(clean_num(token)));
        }
    }

    if (result.transparency.size() < expected)
        result.transparency.resize(expected, 0);
    if (result.transparency.size() > expected)
        result.transparency.resize(expected);

    return result;
}

#pragma endregion
#pragma region UI Processor

// Unhappily made without ai
std::array<std::string, 26> GraphicsUI::pabc1 = {
    /* A */ "01 --2- -3 --", /* B */ "0-1--2- -3--", /* C */ "01--2-3--'",
    /* D */ "0  -1 --2- -3 --", /* E */ "01---2--3---", /* F */ "0 -1-2--3-'",
    /* G */ "01--2--3 -4--'", /* H */ "0-1--2- -3- -", /* I */ "0-12-3-''",
    /* J */ "01 -2 -3 -4-'", /* K */ "0-1- -2--3- -", /* L */ "0-1-2-3 -'",
    /* M */ "01---2---3- -", /* N */ "01--2- -3- -", /* O */ "01---2- -3---",
    /* P */ "01--2- -3--4-", /* Q */ "01 --2- -3 --4  -", /* R */"01--2-3-'",
    /* S */ "01 --2 -3--", /* T */ "0 - 1---2 - 3 --", /* U */ "01- -2- -3---",
    /* V */ "01- -2- -3 -", /* W */ "01- -2---3---", /* X */ "01- -2 -3- -",
    /* Y */ "01- -2---3  -4 -", /* Z */ "01--2 -3 --"
};

std::array<std::string, 10> GraphicsUI::pnums1 = {
    /* 0 */ "0---1- -2- -3---", /* 1 */ "0 -1--2 -3 -",
    /* 2 */ "0--1  -2--3---", /* 3 */ "0--1 --2  -3 --",
    /* 4 */ "0- -1- -2---3  -", /* 5 */ "0---1-2 --3---",
    /* 6 */ "0---1-2---3---", /* 7 */ "0---1  -2 -3-",
    /* 8 */ "0---1- -2---3---", /* 9 */ "0---1- -2---3  -"
};

std::vector<std::string> GraphicsUI::pspecial1 = {
    /* DEFAULT */ "0---1---2---3---",
    /* . */ "3-''", /* , */ "3-4-''", /* ! */ "0-1-23-''",
    /* ? */ "0---1  -2 --34 -", /* : */ "01-23-''", /* ( */ "0 -1-2-3 -'",
    /* ) */ "0-1 -2 -3-'"
};

void GraphicsUI::pixel_ui(
    int x, int y, float z,
    unsigned char r, unsigned char g, unsigned char b) {
        gfx->pixel(x, y, z, r, g, b);
}

/* Rectangle with FLOAT values that use position 0.0f - 1.0f
   like this: 0 => first pixel, 0.5 => halfway pixel, 1 => last pixel*/
void GraphicsUI::rectanglef(vec3 pos, vec2 size, color col) {
    int to_x = size.x*gfx->getWidth();
    int to_y = size.y*gfx->getHeight()*2;

    for(int x = 0; x < to_x; x++) for(int y = 0; y < to_y; y++)
        pixel_ui((pos.x*gfx->getWidth())+x, (pos.x*gfx->getHeight()*2)+y, pos.z, col.r, col.g, col.b);
}

/* Rectangle with int values and uses position as positions
   in buffer or as if the where pixels */
void GraphicsUI::rectanglei(vec3 pos, vec2 size, color col) {
    for(int x = 0; x < size.x; x++) for(int y = 0; y < size.y; y++)
        pixel_ui(pos.x+x, pos.y+y, pos.z, col.r, col.g, col.b);
}

// Here's a kinda simple text processor
void GraphicsUI::label(vec3 pos, std::string text, color text_c, int space_between_letters) {
    auto ascii_to_special_index = [](char a) -> int {
        switch(a) {
            case 33: return 3;
            case 40: return 6;
            case 41: return 7;
            case 44: return 2;
            case 46: return 1;
            case 58: return 5;
            case 63: return 4;
            default: return 0;
        }
    };

    int start_x = pos.x * gfx->getWidth();
    const int start_y = pos.y * gfx->getHeight() * 2;

    for(char i: text) {
        std::string to_print;
        if(i >= 97 && i <= 122) to_print = pabc1[i - 97];
        else if(i >= 48 && i <= 57)  to_print = pnums1[i - 48];
        else if(i == 32) { start_x += 3 + space_between_letters; continue;}
        else to_print = pspecial1[ascii_to_special_index(i)];
        
        /* Decoding the font to print it */

        /* Actual printing loop */
        int x = 0;
        int y = 0;
        int size_adjust = 0;

        for(char j: to_print) switch(j) {
            case 32:
                x++; break;
            case 39:
                size_adjust++; break;
            case 45:
                pixel_ui(start_x + x, start_y + y,
                pos.z, text_c.r, text_c.g, text_c.b);
                x++; break;

            /* Get Y indices */
            case 48: y = 0; x = 0; break;
            case 49: y = 1; x = 0; break;
            case 50: y = 2; x = 0; break;
            case 51: y = 3; x = 0; break;
            case 52: y = 4; x = 0; break;
        }
        
        // There are some letters that are 1px or 2px wide, so this corrects it all
        start_x += 3 - size_adjust + space_between_letters;
    }
}

void GraphicsUI::normalLabel(vec3 position, std::string text, color text_c) {
    pos2 pos = {
        position.x * gfx->columns,
        position.y * gfx->rows
    };
    
    for(unsigned int i = 0; i < text.size(); i++) {
        char c = text[i];
        gfx->char_buffer[pos.y * gfx->columns + pos.x + i] = {c, true};
        for(uint8_t j = 0; j < 3; j++)
            gfx->frame_buffer[(2 * pos.y * gfx->columns + pos.x + i) * 3 + j] = text_c[j];
        gfx->z_buffer[2 * pos.y * gfx->columns + pos.x + i] = position.z;
    }
}

void GraphicsUI::normalLabeli(vec3 position, std::string text, color text_c) {
    pos2 pos = {
        (int)position.x,
        (int)position.y/2
    };
    
    for(unsigned int i = 0; i < text.size(); i++) {
        char c = text[i];
        gfx->char_buffer[pos.y * gfx->columns + pos.x + i] = {c, true};
        for(uint8_t j = 0; j < 3; j++)
            gfx->frame_buffer[(2 * pos.y * gfx->columns + pos.x + i) * 3 + j] = text_c[j];
        gfx->z_buffer[2 * pos.y * gfx->columns + pos.x + i] = position.z;
    }
}

#pragma region Buttons

bool GraphicsUI::button(
    vec3 position,
    std::string text,
    float width,
    float height,
    color col,
    color bg_color,
    color hover_color,
    color active_color
) {
    int id = 0;
    for (char c : text) id = id * 31 + c;

    float left   = position.x - width / 2;
    float right  = position.x + width / 2;
    float top    = position.y - height / 2;
    float bottom = position.y + height / 2;

    bool hovered =
        cursor.pos.x >= left &&
        cursor.pos.x <= right &&
        cursor.pos.y >= top &&
        cursor.pos.y <= bottom;

    if (hovered) {
        ui_hot = id;
        cursor.hover ++;
    }

    if (ui_hot == id && Input::keyPress(Input::Keys::left_mouse_button)) {
        ui_active = id;
    }

    bool clicked = false;

    if (ui_active == id && Input::keyUp(Input::Keys::left_mouse_button)) {
        if (ui_hot == id) {
            clicked = true;
        }
        ui_active = 0;
    }

    if (Input::keyUp(Input::Keys::left_mouse_button)) {
        ui_active = 0;
    }

    color final_color = bg_color;

    if (ui_active == id) final_color = active_color;
    else if (ui_hot == id) final_color = hover_color;

    rectanglei({left, top*2, position.z}, {width, height*2}, final_color);

    int text_start = position.x - (text.length() / 2);

    for (int i = 0; i < text.length(); i++) {
        printChar({(float)text_start + i, position.y}, text[i], col);
    }

    return clicked;
}

void GraphicsUI::drawImage(vec3 pos, const Image& img) {
    for (int y = 0; y < img.height; y++) {
        for (int x = 0; x < img.width; x++) {
            const color& c = img.pixels[y * img.width + x];

            if (img.transparency[y * img.width + x]) continue;

            pixel_ui(pos.x + x, pos.y + y, pos.z, c.r, c.g, c.b);
        }
    }
}

bool GraphicsUI::buttonImg(
    vec3 position,
    const Image& img,
    color tint_normal,
    color tint_hover,
    color tint_active
) {
    int id = reinterpret_cast<std::uintptr_t>(&img);

    float left = position.x;
    float right = position.x + img.width;
    float top = position.y;
    float bottom = position.y + (img.height / 2);

    bool hovered =
        cursor.pos.x >= left &&
        cursor.pos.x <= right &&
        cursor.pos.y >= top &&
        cursor.pos.y <= bottom;

    if (hovered) ui_hot = id, cursor.hover++;

    if (ui_hot == id && Input::keyPress(Input::Keys::left_mouse_button)) {
        ui_active = id;
    }

    bool clicked = false;

    if (ui_active == id && Input::keyUp(Input::Keys::left_mouse_button)) {
        if (ui_hot == id) clicked = true;
        ui_active = 0;
    }

    color tint = tint_normal;

    if (ui_active == id) tint = tint_active;
    else if (ui_hot == id) tint = tint_hover;
    float drawY = position.y * 2;

    for (int y = 0; y < img.height; y++) {
        for (int x = 0; x < img.width; x++) {
            const color& c = img.pixels[y * img.width + x];
            if (img.transparency[y * img.width + x]) continue;

            pixel_ui(
                position.x + x,
                drawY + y,
                position.z,
                (c.r * tint.r) / 255,
                (c.g * tint.g) / 255,
                (c.b * tint.b) / 255
            );
        }
    }

    return clicked;
}

bool GraphicsUI::buttonImgLabel(
    vec3 position,
    const Image& img,
    const std::string& text,
    color text_color,
    color tint_normal,
    color tint_hover,
    color tint_active
) {
    int id = reinterpret_cast<std::uintptr_t>(&img);

    float width = img.width;
    float height = img.height + 1; // +1 para texto

    float left = position.x;
    float right = position.x + width;
    float top = position.y;
    float bottom = position.y + height;

    bool hovered =
        cursor.pos.x >= left &&
        cursor.pos.x <= right &&
        cursor.pos.y >= top &&
        cursor.pos.y <= bottom;

    if (hovered) {
        ui_hot = id;
        cursor.hover++;
    }

    if (ui_hot == id && Input::keyPress(Input::Keys::left_mouse_button)) {
        ui_active = id;
    }

    bool clicked = false;

    if (ui_active == id && Input::keyUp(Input::Keys::left_mouse_button)) {
        if (ui_hot == id) clicked = true;
        ui_active = 0;
    }

    color tint = tint_normal;
    if (ui_active == id) tint = tint_active;
    else if (ui_hot == id) tint = tint_hover;

    float drawY = position.y * 2;

    for (int y = 0; y < img.height; y++) {
        for (int x = 0; x < img.width; x++) {
            int idx = y * img.width + x;

            if (img.transparency[idx]) continue;

            const color& c = img.pixels[idx];

            pixel_ui(
                position.x + x,
                drawY + y,
                position.z,
                min(255, c.r + tint.r / 4),
                min(255, c.g + tint.g / 4),
                min(255, c.b + tint.b / 4)
            );
        }
    }

    int textX = position.x + (img.width / 2) - (text.length() / 2);
    int textY = position.y + img.height;

    for (int i = 0; i < text.length(); i++) {
        printChar(
            { (float)(textX + i), (float)textY },
            text[i],
            text_color
        );
    }

    return clicked;
}

#pragma endregion
#pragma region Fundamentals

void GraphicsUI::printChar(vec2 pos, char c, color text_c, float z) {
    gfx->char_buffer[pos.y * gfx->columns + pos.x] = {c, true};
    gfx->pixel(pos.x, pos.y * 2, z, text_c.r, text_c.g, text_c.b);
}

void GraphicsUI::Update() {
    ui_hot = 0;
}

#pragma endregion


};