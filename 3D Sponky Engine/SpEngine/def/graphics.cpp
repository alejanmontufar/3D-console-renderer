#include "../SpConsole.h"

#pragma region Base functions

namespace capp {

#ifdef _WIN32
Graphics::Graphics() {
    setAutoBufferSize();
    DWORD mode;
    HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);

    // Basic console optimization and config
    std::cout<<"\033[?25l";
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);
    
    GetConsoleMode(hInput, &mode);
    mode |= ENABLE_EXTENDED_FLAGS;
    mode &= ~ENABLE_QUICK_EDIT_MODE;
    SetConsoleMode(hInput, mode);

    Debug::Log("Started program with buffer resolution of " + std::to_string(columns)
    + " x " + std::to_string(pix_height));
}

void Graphics::setAutoBufferSize() {
    // Get Screen Width and height
    // This step ONLY works on windows, just there
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    if (GetConsoleScreenBufferInfo(hConsole, &csbi)) {
        columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        rows = csbi.srWindow.Bottom - csbi.srWindow.Top;
    } else std::cerr << "Error getting console screen buffer info." << std::endl;


    pix_height = 2 * rows;

    frame_buffer.resize(columns * pix_height * 3);
    z_buffer.resize(columns * pix_height);
    char_buffer.resize(columns * rows);
    std::fill(z_buffer.begin(), z_buffer.end(), 1e9f);
    
    MainCamera.setProjectionMatrix(columns, rows);
}

bool Graphics::resizedWindow() {
    HANDLE hin = GetStdHandle(STD_INPUT_HANDLE);

    DWORD numEvents = 0;
    GetNumberOfConsoleInputEvents(hin, &numEvents);

    if (numEvents == 0) return false;

    INPUT_RECORD record;
    DWORD events_read;

    ReadConsoleInputW(hin, &record, 1, &events_read);

    return record.EventType == WINDOW_BUFFER_SIZE_EVENT;
}

#else
Graphics::Graphics(int width, int height) {
    // Set this up, in a way that the programmer has to absolutely specify w and h
    columns = width, rows = height, pix_height = 2 * rows;
    frame_buffer.resize(columns * pix_height * 3);
    z_buffer.resize(columns * pix_height);

    // Basic console optimization and config
    std::cout<<"\033[?25l";
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);
}
#endif

void Graphics::pixel(
    int x, int y, float z,
    unsigned char r, unsigned char g, unsigned char b
) {
    if (x < 0 || x >= columns || y < 0 || y >= pix_height) return;
    int i = y * columns + x;

    if (z > z_buffer[i]) return;
    z_buffer[i] = z;

    int fbi = i * 3;
    frame_buffer[fbi + 0] = r;
    frame_buffer[fbi + 1] = g;
    frame_buffer[fbi + 2] = b;
}

#pragma endregion
#pragma region Construct frame

/*

    This function is just an algorithm to pass all pixels and
    colors from the frame buffer, into a giant string to show
    in std::out, if there was an output for every pixel, it
    would be so slow, and it would have been worst inside the
    console!!

*/

void Graphics::constructFrame() {
    std::vector<char> out;
    size_t pos = 0;

    out.resize((size_t)columns * rows * 48 + 64);
    out[pos++] = '\033';
    out[pos++] = '[';
    out[pos++] = 'H';

    // Lambda for efficiency    
    auto byteToStr = [](unsigned char v, char* dest)->void {
        dest[0] = '0' + (v / 100);
        int t = v % 100;
        dest[1] = '0' + (t / 10);
        dest[2] = '0' + (t % 10);
    };

    // Loop for every "Y"
    for(int cy = 0; cy < rows; cy++) {
        // Set this because we have two pixels per char
        int pix_up = cy * 2, pix_down = pix_up + 1;

        // Loop for every "X"
        for(int x = 0; x < columns; x++) {
            int index_up = (pix_up * columns + x) * 3;
            int index_down = (pix_down * columns + x) * 3;


            unsigned char r1=frame_buffer[index_up],g1=frame_buffer[index_up+1],
            b1=frame_buffer[index_up+2];
            unsigned char r2=frame_buffer[index_down],g2=frame_buffer[index_down+1],
            b2=frame_buffer[index_down+2];

            out[pos++] = '\033'; out[pos++] = '['; out[pos++] = '3'; out[pos++] = '8';
            out[pos++] = ';'; out[pos++] = '2'; out[pos++] = ';';

            byteToStr(r1, &out[pos]); pos+=3; out[pos++]=';';
            byteToStr(g1, &out[pos]); pos+=3; out[pos++]=';';
            byteToStr(b1, &out[pos]); pos+=3; out[pos++]='m';

            // ESC[48;2;RRR;GGG;BBB m
            out[pos++] = '\033'; out[pos++] = '['; out[pos++] = '4'; out[pos++] = '8';
            out[pos++] = ';'; out[pos++] = '2'; out[pos++] = ';';

            byteToStr(r2, &out[pos]); pos+=3; out[pos++]=';';
            byteToStr(g2, &out[pos]); pos+=3; out[pos++]=';';
            byteToStr(b2, &out[pos]); pos+=3; out[pos++]='m';

            char_pix char_b = char_buffer[columns * cy + x];

            if(using_ui && char_b.u) out[pos++] = char_b.c;
            else out[pos++] = char(223);
        }
        out[pos++] = '\033';
        out[pos++] = '[';
        out[pos++] = '0';
        out[pos++] = 'm';
        out[pos++] = '\n';
    }

    fwrite(out.data(), 1, pos, stdout);
    std::fflush(stdout);
}

#pragma endregion
#pragma region Fill Triangle

/*

    Here starts the real magic

    This function is where the triangle is being written in the
    framebuffer, but this is not rendering yet, just passing from
    a function into buffer

*/


int Graphics::fillTriangle(
    std::array<pos2, 3> verts,
    std::array<float, 3> vertz,        // z/w
    std::array<float, 3> inv_w,        // 1/w
    std::array<vec3, 3> col_over_w,     // color/w  
    std::array<vec2, 3> uv_over_w,
    std::array<vec3, 3> normal_over_w,
    const vec3 &normal_view
) {
    /*
    Here I use barycentric coordinates
    It's a bit easier to understand due to math <3
    */
    
    auto edge_cross = [](pos2 a, pos2 b, pos2 p)->float {
        pos2 ab = pos2 { b.x - a.x, b.y - a.y };
        pos2 ap = pos2 {p.x - a.x, p.y - a.y };
        return (ab.x * ap.y) - (ab.y * ap.x);
    };

    auto clamp8 = [](float v)->unsigned char {
        if (v <= 0.0f) return 0;
        if (v >= 255.0f) return 255;
        return (unsigned char)(v + 0.5f);
    };

    int minX = min(verts[0].x, min(verts[1].x, verts[2].x));
    int minY = min(verts[0].y, min(verts[1].y, verts[2].y));
    int maxX = max(verts[0].x, max(verts[1].x, verts[2].x));
    int maxY = max(verts[0].y, max(verts[1].y, verts[2].y));

    float area = edge_cross(verts[0], verts[1], verts[2]);
    if (area == 0.0f) return 0;

    // clamp
    if (minX < 0) minX = 0;
    if (minY < 0) minY = 0;
    if (maxX >= columns) maxX = columns - 1;
    if (maxY >= pix_height) maxY = pix_height - 1;

    for (int y = minY; y < maxY; y++) {
        for (int x = minX; x < maxX; x++) {
            pos2 p(x, y);
            float w0 = edge_cross(verts[1], verts[2], p);
            float w1 = edge_cross(verts[2], verts[0], p);
            float w2 = edge_cross(verts[0], verts[1], p);

            // NOT TO n / 0 for health 
            if (w0 < 0 || w1 < 0 || w2 < 0) continue;
            w0 /= area; w1 /= area; w2 /= area;

            float denom = w0 * inv_w[0] + w1 * inv_w[1] + w2 * inv_w[2];
            if (denom == 0.0f) continue;
            float recip = 1.0f / denom;

            // depth_over_w interpolation (sum lambda * (z/w))
            float depth_over_w_interp = w0 * vertz[0] + w1 * vertz[1] + w2 * vertz[2];
            float zz = (depth_over_w_interp * recip) * 0.5f + 0.5f; // final depth

            // color_over_w interpolation and divide by invW
            float r_over_w = w0 * col_over_w[0].x + w1 * col_over_w[1].x + w2 * col_over_w[2].x;
            float g_over_w = w0 * col_over_w[0].y + w1 * col_over_w[1].y + w2 * col_over_w[2].y;
            float b_over_w = w0 * col_over_w[0].z + w1 * col_over_w[1].z + w2 * col_over_w[2].z;

            float u_over_w = w0 * uv_over_w[0].x + w1 * uv_over_w[1].x + w2 * uv_over_w[2].x;
            float v_over_w = w0 * uv_over_w[0].y + w1 * uv_over_w[1].y + w2 * uv_over_w[2].y;

            vec3 n_over_w = normal_over_w[0] * w0 + normal_over_w[1] * w1 + normal_over_w[2] * w2;
            vec3 normal = (n_over_w * recip);

            // Some calcs
            float r = r_over_w * recip;
            float g = g_over_w * recip;
            float b = b_over_w * recip;
            float u = u_over_w * recip;
            float v = v_over_w * recip;

            color texel = {255, 255, 255};
            if(texture.using_texture) texel = texture.sample(u, v);

            float lr = clamp8(r) * texel.r / 255;
            float lg = clamp8(g) * texel.g / 255;
            float lb = clamp8(b) * texel.b / 255;

            float difuse = max(0.0f, normal * -light_dir_view);
            float light = ambient + difuse;

            if(using_light) lr *= light, lg *= light, lb *= light;

            lr = clamp8(lr); lg = clamp8(lg); lb = clamp8(lb);


            pixel(x, y, zz,
                lr, lg, lb
            );
        }
    }
    return 1;
}

#pragma endregion
#pragma region Renderer

/*

    Now, here is where the things start to get mathy, because
    here you will use perpective, view, model, you start converting
    those positions into a 2d space for the rastering function to
    show them to buffer. Now this is actually called "rendering".

*/

int Graphics::renderInBuffer(mat4 model, mat4 view, mat4 proj) {

    int triangles_to_draw = indices.size() / 3;
    int triangle_count = 0;

    auto safe_idx = [&](uint32_t idx)->bool {
        return idx < vertices.size();
    };

    auto screenize = [&](const vec4 &v)->pos2 {
        float ndc_x = v.x / v.w;
        float ndc_y = v.y / v.w;

        return {
            columns * (ndc_x + 1.0f) * 0.5f,
            rows    * (1.0f - ndc_y) * 0.5f
        };
    };

    auto insideNear = [](const vec3& v, float znear)->bool {
        return v.z <= -znear;
    };

    auto intersectNear = [](const vec3& a, const vec3& b, float znear)->vec3 {
        float da = a.z + znear;
        float db = b.z + znear;
        float t = da / (da - db);
        return a + (b - a) * t;
    };

    auto clipTriangleNear =
    [&](const vec3& v0, const vec3& v1, const vec3& v2,
        float znear, vec3 out[6])->int {

        vec3 in[3] = { v0, v1, v2 };
        bool inside[3] = {
            insideNear(v0, znear),
            insideNear(v1, znear),
            insideNear(v2, znear)
        };

        int count = inside[0] + inside[1] + inside[2];

        if (count == 0) return 0;

        if (count == 3) {
            out[0]=v0; out[1]=v1; out[2]=v2;
            return 1;
        }

        if (count == 1) {
            int i = inside[0]?0:(inside[1]?1:2);
            vec3 a = in[i];
            vec3 b = intersectNear(a, in[(i+1)%3], znear);
            vec3 c = intersectNear(a, in[(i+2)%3], znear);
            out[0]=a; out[1]=b; out[2]=c;
            return 1;
        }

        int o = !inside[0]?0:(!inside[1]?1:2);
        vec3 a = in[(o+1)%3];
        vec3 b = in[(o+2)%3];
        vec3 c = intersectNear(a, in[o], znear);
        vec3 d = intersectNear(b, in[o], znear);

        out[0]=a; out[1]=b; out[2]=c;
        out[3]=b; out[4]=d; out[5]=c;
        return 2;
    };

    const bool has_colors  = colors.size()  == vertices.size();
    const bool has_uvs     = uvs.size()     == vertices.size();
    const bool has_normals = normals.size() == vertices.size();

    light_dir_view = (view * vec4(light_dir, 0)).xyz().normalized();
    mat3 normal_mat = mat3((view * model).inverse().transpose());

    for (int i = 0; i < triangles_to_draw; ++i) {
        uint32_t i0 = indices[i*3+0];
        uint32_t i1 = indices[i*3+1];
        uint32_t i2 = indices[i*3+2];

        vec3 p0 = vertices[i0];
        vec3 p1 = vertices[i1];
        vec3 p2 = vertices[i2];

        vec4 v0v4 = view * (model * vec4(p0,1));
        vec4 v1v4 = view * (model * vec4(p1,1));
        vec4 v2v4 = view * (model * vec4(p2,1));

        vec3 v0v = v0v4.xyz();
        vec3 v1v = v1v4.xyz();
        vec3 v2v = v2v4.xyz();

        vec3 normal_view = (v1v-v0v).cross(v2v-v0v).normalized();
        vec3 view_dir = (-(v0v+v1v+v2v)/3.0f).normalized();
        if ((normal_view * view_dir) >= 0.0f) {continue;}

        vec3 clipped[6];
        int triCount = clipTriangleNear(v0v, v1v, v2v, MainCamera.znear, clipped);
        if(triCount == 0) {continue;}

        for (int t = 0; t < triCount; ++t) {
            vec3 cv0 = clipped[t*3+0];
            vec3 cv1 = clipped[t*3+1];
            vec3 cv2 = clipped[t*3+2];

            vec4 c0 = proj * vec4(cv0,1);
            vec4 c1 = proj * vec4(cv1,1);
            vec4 c2 = proj * vec4(cv2,1);

            if (c0.w<=0||c1.w<=0||c2.w<=0) continue;

            float iw0 = 1.0f/c0.w, iw1 = 1.0f/c1.w, iw2 = 1.0f/c2.w;
            std::array<float,3> inv_w = { iw0, iw1, iw2 };

            std::array<vec3,3> cols_over_w = { NEUTRAL, NEUTRAL, NEUTRAL };
            if (has_colors)
                cols_over_w = {
                    vec3(colors[i0])*iw0,
                    vec3(colors[i1])*iw1,
                    vec3(colors[i2])*iw2
                };

            std::array<vec2,3> uv_over_w = { vec2(0,0),vec2(0,0),vec2(0,0) };
            if (has_uvs)
                uv_over_w = {
                    uvs[i0]*iw0,
                    uvs[i1]*iw1,
                    uvs[i2]*iw2
                };

            std::array<vec3,3> normals_over_w = {
                normal_view*iw0,
                normal_view*iw1,
                normal_view*iw2
            };

            if (has_normals)
                normals_over_w = {
                    (normal_mat*normals[i0]).normalized()*iw0,
                    (normal_mat*normals[i1]).normalized()*iw1,
                    (normal_mat*normals[i2]).normalized()*iw2
                };

            triangle_count += fillTriangle(
                { screenize(c0), screenize(c1), screenize(c2) },
                { c0.z*iw0, c1.z*iw1, c2.z*iw2 },
                inv_w, cols_over_w, uv_over_w, normals_over_w, normal_view
            );
        }
    }

    return triangle_count;
}

int Graphics::renderMesh(const Mesh &mesh, mat4 model) {
    vertices = mesh.vertices;
    indices = mesh.indices;
    colors = mesh.colors;
    uvs = mesh.uvs;
    texture = mesh.texture;

    return renderInBuffer(
        model,
        MainCamera.getViewMatrix(),
        MainCamera.proj
    );
}

#pragma endregion
};