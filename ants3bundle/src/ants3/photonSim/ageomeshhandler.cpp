#include "ageomeshhandler.h"
#include <sstream>
#include <iomanip>

AGeoMeshHandler::IcosahedronResult AGeoMeshHandler::icosahedron()
{
    double phi = (1.0 + std::sqrt(5.0)) / 2.0;
    std::vector<Vec3> verts =
    {
        {-1, phi, 0}, {1, phi, 0}, {-1, -phi, 0}, {1, -phi, 0},
        {0, -1, phi}, {0, 1, phi}, {0, -1, -phi}, {0, 1, -phi},
        {phi, 0, -1}, {phi, 0, 1}, {-phi, 0, -1}, {-phi, 0, 1}
    };

    double r = vLength(verts[0]);
    for (auto & v : verts) v = vScale(v, 1.0 / r);

    std::vector<Triangle> faces =
    {
        {0, 11, 5}, {0, 5, 1}, {0, 1, 7}, {0, 7, 10}, {0, 10, 11},
        {1, 5, 9}, {5, 11, 4}, {11, 10, 2}, {10, 7, 6}, {7, 1, 8},
        {3, 9, 4}, {3, 4, 2}, {3, 2, 6}, {3, 6, 8}, {3, 8, 9},
        {4, 9, 5}, {2, 4, 11}, {6, 2, 10}, {8, 6, 7}, {9, 8, 1}
    };

    return { verts, faces };
}

std::vector<AGeoMeshHandler::Vec3> AGeoMeshHandler::alignVertexToPole(const std::vector<Vec3>& verts, int vertexIdx) {
    Vec3 target = vNormalize(verts[vertexIdx]);
    Vec3 z = {0.0, 0.0, 1.0};
    Vec3 axisRaw = vCross(target, z);
    double axisLen = vLength(axisRaw);

    Mat3 R;
    if (axisLen < 1e-12) {
        R = {{{1, 0, 0}, {0, 1, 0}, {0, 0, 1}}};
    } else {
        Vec3 axis = vScale(axisRaw, 1.0 / axisLen);
        double angle = std::acos(std::max(-1.0, std::min(1.0, vDot(target, z))));
        double kx = axis[0], ky = axis[1], kz = axis[2];
        Mat3 K = {{
            {0, -kz, ky},
            {kz, 0, -kx},
            {-ky, kx, 0}
        }};
        Mat3 K2 = matMul(K, K);
        double s = std::sin(angle);
        double c = 1.0 - std::cos(angle);

        Mat3 identity = {{{1, 0, 0}, {0, 1, 0}, {0, 0, 1}}};
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                R[i][j] = (i == j ? 1.0 : 0.0) + s * K[i][j] + c * K2[i][j];
            }
        }
    }

    std::vector<Vec3> result;
    result.reserve(verts.size());
    for (const auto& v : verts) {
        result.push_back(matVec(R, v));
    }
    return result;
}

bool AGeoMeshHandler::sphericalTriangleContains(const Vec3& A, const Vec3& B, const Vec3& C, const Vec3& P, double tol) {
    double d1 = vDot(vCross(A, B), P);
    double d2 = vDot(vCross(B, C), P);
    double d3 = vDot(vCross(C, A), P);
    return d1 >= -tol && d2 >= -tol && d3 >= -tol;
}

void AGeoMeshHandler::buildHemisphereMesh(double N, double tol)
{
    vertices.clear();
    triangles.clear();
    faceData.clear();

    v = std::max(2, static_cast<int>(std::round(std::sqrt(N / 10.0) / 2.0) * 2));

    auto [rawVerts, baseFaces] = icosahedron();
    auto baseVerts = alignVertexToPole(rawVerts, 0);

    std::map<std::string, int> vertexMap;
    //std::vector<Vec3> vertices;
    //std::vector<Triangle> triangles;

    auto keyOf = [](const Vec3& p) {
        // Equivalent to 9 decimal fixed string formatting in JS
        std::stringstream ss;
        ss << std::fixed << std::setprecision(9) << p[0] << "," << p[1] << "," << p[2];
        return ss.str();
    };

    auto getIndex = [&](const Vec3& p) {
        std::string key = keyOf(p);
        auto it = vertexMap.find(key);
        if (it == vertexMap.end()) {
            int idx = static_cast<int>(vertices.size());
            vertexMap[key] = idx;
            vertices.push_back(p);
            return idx;
        }
        return it->second;
    };

    //std::vector<FaceData> faceData;
    faceData.reserve(baseFaces.size());

    for (const auto& f : baseFaces)
    {
        Vec3 A = baseVerts[f[0]], B = baseVerts[f[1]], C = baseVerts[f[2]];
        Vec3 e0 = vSub(B, A), e1 = vSub(C, A);
        Vec3 n = vCross(e0, e1);
        double d00 = vDot(e0, e0), d01 = vDot(e0, e1), d11 = vDot(e1, e1);
        double denom = d00 * d11 - d01 * d01;
        std::vector<int> cellIndex(v * v * 2, -1);
        faceData.push_back({A, B, C, e0, e1, n, d00, d01, d11, denom, cellIndex});
    }

    for (size_t faceIdx = 0; faceIdx < baseFaces.size(); ++faceIdx)
    {
        auto& fd = faceData[faceIdx];
        std::map<std::string, Vec3> grid;

        for (int i = 0; i <= v; ++i) {
            for (int j = 0; j <= v - i; ++j) {
                Vec3 p = vAdd(fd.A, vAdd(vScale(vSub(fd.B, fd.A), static_cast<double>(i) / v),
                                         vScale(vSub(fd.C, fd.A), static_cast<double>(j) / v)));
                grid[std::to_string(i) + "," + std::to_string(j)] = vNormalize(p);
            }
        }

        auto emit = [&](const std::string& ia, const std::string& ib, const std::string& ic, int cellI, int cellJ, int half) {
            Vec3 pa = grid[ia];
            Vec3 pb = grid[ib];
            Vec3 pc = grid[ic];
            if (std::min({pa[2], pb[2], pc[2]}) >= -tol) {
                int globalIdx = static_cast<int>(triangles.size());
                triangles.push_back({getIndex(pa), getIndex(pb), getIndex(pc)});
                fd.cellIndex[(cellI * v + cellJ) * 2 + half] = globalIdx;
            }
        };

        for (int i = 0; i < v; ++i) {
            for (int j = 0; j < v - i; ++j) {
                emit(std::to_string(i) + "," + std::to_string(j),
                     std::to_string(i + 1) + "," + std::to_string(j),
                     std::to_string(i) + "," + std::to_string(j + 1), i, j, 0);
                if (j < v - i - 1) {
                    emit(std::to_string(i + 1) + "," + std::to_string(j),
                         std::to_string(i + 1) + "," + std::to_string(j + 1),
                         std::to_string(i) + "," + std::to_string(j + 1), i, j, 1);
                }
            }
        }
    }

    //return MeshResult{ vertices, triangles, v, faceDataList };
}

int AGeoMeshHandler::findTriangleIndex(const Vec3& point, double tol, bool requireOnHemisphere) const
{
    double len = vLength(point);
    if (len < 1e-12) return -1;
    Vec3 P = vScale(point, 1.0 / len);
    if (requireOnHemisphere && P[2] < -tol) return -1;

    int faceIdx = -1;
    for (size_t f = 0; f < faceData.size(); ++f) {
        if (sphericalTriangleContains(faceData[f].A, faceData[f].B, faceData[f].C, P, tol))
        {
            faceIdx = static_cast<int>(f);
            break;
        }
    }
    if (faceIdx == -1) return -1;

    const auto& fd = faceData[faceIdx];
    double t = vDot(fd.n, fd.A) / vDot(fd.n, P);
    Vec3 Q = vScale(P, t);
    Vec3 v2 = vSub(Q, fd.A);
    double d20 = vDot(v2, fd.e0);
    double d21 = vDot(v2, fd.e1);
    double beta = (fd.d11 * d20 - fd.d01 * d21) / fd.denom;
    double gamma = (fd.d00 * d21 - fd.d01 * d20) / fd.denom;

    double x = beta * v;
    double y = gamma * v;
    int i0 = static_cast<int>(std::floor(x + 1e-9));
    int j0 = static_cast<int>(std::floor(y + 1e-9));
    i0 = std::min(std::max(i0, 0), v - 1);
    j0 = std::min(std::max(j0, 0), v - 1 - i0);
    double fx = x - i0;
    double fy = y - j0;
    int half = (fx + fy <= 1.0) ? 0 : 1;

    return fd.cellIndex[(i0 * v + j0) * 2 + half];
}

int AGeoMeshHandler::findTriangleIndexBruteForce(const Vec3& point, double tol, bool requireOnHemisphere) const
{
    double len = vLength(point);
    if (len < 1e-12) return -1;
    Vec3 P = vScale(point, 1.0 / len);
    if (requireOnHemisphere && P[2] < -tol) return -1;

    for (size_t idx = 0; idx < triangles.size(); ++idx) {
        const auto& tri = triangles[idx];
        if (sphericalTriangleContains(vertices[tri[0]], vertices[tri[1]], vertices[tri[2]], P, tol)) {
            return static_cast<int>(idx);
        }
    }
    return -1;
}

AGeoMeshHandler::Vec3 AGeoMeshHandler::pointFromLatLon(double latRad, double lonRad) {
    double cosLat = std::cos(latRad);
    return { cosLat * std::cos(lonRad), cosLat * std::sin(lonRad), std::sin(latRad) };
}

AGeoMeshHandler::EdgeStats AGeoMeshHandler::edgeLengthStats()
{
    std::vector<double> lens;
    lens.reserve(triangles.size() * 3);

    for (const auto& tri : triangles) {
        Vec3 p0 = vertices[tri[0]];
        Vec3 p1 = vertices[tri[1]];
        Vec3 p2 = vertices[tri[2]];
        lens.push_back(vLength(vSub(p1, p0)));
        lens.push_back(vLength(vSub(p2, p1)));
        lens.push_back(vLength(vSub(p0, p2)));
    }

    double sum = std::accumulate(lens.begin(), lens.end(), 0.0);
    double mean = sum / lens.size();

    double sqSum = std::accumulate(lens.begin(), lens.end(), 0.0, [mean](double acc, double val) {
        return acc + (val - mean) * (val - mean);
    });
    double variance = sqSum / lens.size();

    auto [minIt, maxIt] = std::minmax_element(lens.begin(), lens.end());

    return { mean, std::sqrt(variance), *minIt, *maxIt };
}
