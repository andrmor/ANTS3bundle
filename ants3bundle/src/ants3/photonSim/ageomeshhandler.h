#ifndef AGEOMESHHANDLER_H
#define AGEOMESHHANDLER_H
#pragma once

#include <vector>
#include <array>
#include <string>
#include <map>
#include <cmath>
#include <algorithm>
#include <numeric>

class AGeoMeshHandler
{
public:
    // --- Types ---
    using Vec3 = std::array<double, 3>;
    using Mat3 = std::array<std::array<double, 3>, 3>;
    using Triangle = std::array<int, 3>;

    struct IcosahedronResult {
        std::vector<Vec3> verts;
        std::vector<Triangle> faces;
    };

    struct FaceData {
        Vec3 A;
        Vec3 B;
        Vec3 C;
        Vec3 e0;
        Vec3 e1;
        Vec3 n;
        double d00;
        double d01;
        double d11;
        double denom;
        std::vector<int> cellIndex; // Sized v * v * 2
    };

    struct EdgeStats {
        double mean;
        double std;
        double min;
        double max;
    };

    // --- Vector Operations ---
    static inline Vec3 vAdd(const Vec3& a, const Vec3& b) {
        return { a[0] + b[0], a[1] + b[1], a[2] + b[2] };
    }

    static inline Vec3 vSub(const Vec3& a, const Vec3& b) {
        return { a[0] - b[0], a[1] - b[1], a[2] - b[2] };
    }

    static inline Vec3 vScale(const Vec3& a, double s) {
        return { a[0] * s, a[1] * s, a[2] * s };
    }

    static inline double vLength(const Vec3& a) {
        return std::sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);
    }

    static inline Vec3 vNormalize(const Vec3& a) {
        double len = vLength(a);
        return { a[0] / len, a[1] / len, a[2] / len };
    }

    static inline Vec3 vCross(const Vec3& a, const Vec3& b) {
        return {
            a[1] * b[2] - a[2] * b[1],
            a[2] * b[0] - a[0] * b[2],
            a[0] * b[1] - a[1] * b[0]
        };
    }

    static inline double vDot(const Vec3& a, const Vec3& b) {
        return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
    }

    // --- Matrix Operations ---
    static inline Mat3 matMul(const Mat3& A, const Mat3& B) {
        Mat3 out = {{{0,0,0}, {0,0,0}, {0,0,0}}};
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                for (int k = 0; k < 3; ++k) {
                    out[i][j] += A[i][k] * B[k][j];
                }
            }
        }
        return out;
    }

    static inline Vec3 matVec(const Mat3& A, const Vec3& v) {
        return {
            A[0][0] * v[0] + A[0][1] * v[1] + A[0][2] * v[2],
            A[1][0] * v[0] + A[1][1] * v[1] + A[1][2] * v[2],
            A[2][0] * v[0] + A[2][1] * v[1] + A[2][2] * v[2]
        };
    }

    // --- Core Logic ---
    static IcosahedronResult icosahedron();
    static std::vector<Vec3> alignVertexToPole(const std::vector<Vec3>& verts, int vertexIdx = 0);
    static bool sphericalTriangleContains(const Vec3& A, const Vec3& B, const Vec3& C, const Vec3& P, double tol = 1e-9);

    void buildHemisphereMesh(double N, double tol = 1e-7);

    static Vec3 pointFromLatLon(double latRad, double lonRad);
    EdgeStats edgeLengthStats();


    std::vector<Vec3> vertices;
    std::vector<Triangle> triangles;
    int v;
    std::vector<FaceData> faceData;

    int findTriangleIndex(const Vec3& point, double tol = 1e-9, bool requireOnHemisphere = true) const;
    int findTriangleIndexBruteForce(const Vec3& point, double tol = 1e-9, bool requireOnHemisphere = true) const;
};

#endif // AGEOMESHHANDLER_H
