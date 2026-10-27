#include "../SpConsole.h"

namespace capp {
    mat4 mat4::identity() {
        mat4 res{};
        for(int i = 0; i < 4; i++) res.matrix[i][i] = 1.0f;
        return res;
    }

    mat4 mat4::operator*(const mat4 &a) {
        mat4 r = {};

        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                for (int k = 0; k < 4; k++)
                    r.matrix[i][j] += matrix[i][k] * a.matrix[k][j];
        
        return r;
    }

    vec4 mat4::operator*(const vec4 &v) const {
        return {
            matrix[0][0]*v.x + matrix[0][1]*v.y + matrix[0][2]*v.z + matrix[0][3]*v.w,
            matrix[1][0]*v.x + matrix[1][1]*v.y + matrix[1][2]*v.z + matrix[1][3]*v.w,
            matrix[2][0]*v.x + matrix[2][1]*v.y + matrix[2][2]*v.z + matrix[2][3]*v.w,
            matrix[3][0]*v.x + matrix[3][1]*v.y + matrix[3][2]*v.z + matrix[3][3]*v.w
        };
    }

    mat4 mat4::rotateX(float angle) {
        mat4 r = identity();
        float c = cos(angle), s = sin(angle);

        r.matrix[1][1] = c;
        r.matrix[1][2] = -s;
        r.matrix[2][1] = s;
        r.matrix[2][2] = c;
        return r;
    }

    mat4 mat4::rotateY(float angle) {
        mat4 r = identity();
        float c = cos(angle), s = sin(angle);

        r.matrix[0][0] = c;
        r.matrix[0][2] = s;
        r.matrix[2][0] = -s;
        r.matrix[2][2] = c;
        return r;
    }

    mat4 mat4::rotateZ(float angle) {
        mat4 r = identity();
        float c = cos(angle), s = sin(angle);

        r.matrix[0][0] = c;
        r.matrix[0][1] = -s;
        r.matrix[1][0] = s;
        r.matrix[1][1] = c;
        return r;
    }

    mat4 mat4::perspective(float fov, float aspect, float znear, float zfar) {
        float f = 1.0f / tan(fov * 0.5f);

        mat4 r = {};
        r.matrix[0][0] = f / aspect;
        r.matrix[1][1] = f;
        r.matrix[2][2] = (zfar + znear) / (znear - zfar);
        r.matrix[2][3] = (2 * zfar * znear) / (znear - zfar);
        r.matrix[3][2] = -1.0f;
        r.matrix[3][3] = 0.0f;
        return r;
    }

    mat4 mat4::translate(float x, float y, float z) {
        mat4 r = identity();
        r.matrix[0][3] = x;
        r.matrix[1][3] = y;
        r.matrix[2][3] = z;
        return r;
    }

    mat4 mat4::scale(float sx, float sy, float sz) {
        mat4 r{};
        r.matrix[0][0] = sx;
        r.matrix[1][1] = sy;
        r.matrix[2][2] = sz;
        r.matrix[3][3] = 1.0f;
        return r;
    }

    vec3 mat3::operator*(const vec3 &v) {
        return {
            matrix[0][0]*v.x + matrix[0][1]*v.y + matrix[0][2]*v.z,
            matrix[1][0]*v.x + matrix[1][1]*v.y + matrix[1][2]*v.z,
            matrix[2][0]*v.x + matrix[2][1]*v.y + matrix[2][2]*v.z
        };
    }

    mat3::mat3(const mat4 &mat) {
        for(int i=0;i<3;i++)
            for(int j=0;j<3;j++)
                matrix[i][j] = mat.matrix[i][j];
    }

    mat4 mat4::transpose() {
        mat4 r{};
        for(int i=0;i<3;i++)
            for(int j=0;j<3;j++)
                r.matrix[i][j] = matrix[i][j];
        return r;
    }

    mat4 mat4::inverse() {
        mat4 inv = identity();
        mat4 a = *this;

        for(int i=0;i<4;i++) {
            float diag = a.matrix[i][i];
            if (fabs(diag) < 1e-8f) continue;

            float invDiag = 1.0f / diag;
            for(int j=0;j<4;j++) {
                a.matrix[i][j] *= invDiag;
                inv.matrix[i][j] *= invDiag;
            }

            for(int r=0;r<4;r++) {
                if (r==i) continue;
                float f = a.matrix[r][i];
                for(int c=0;c<4;c++) {
                    a.matrix[r][c] -= f * a.matrix[i][c];
                    inv.matrix[r][c] -= f * inv.matrix[i][c];
                }
            }
        }
        return inv;
    }
};