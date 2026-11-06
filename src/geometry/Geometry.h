#pragma once


#include <vector>
#include <algorithm>
#include <utility>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <cmath>
#include <limits>
#include <QVector>
#include "Point.h"


namespace predicate
{
    template <typename T> inline T geom_abs(const T& v) { return v < T(0) ? -v : v; }
    
    
    template <typename T>
    inline int orientation(const Point<T>& a, const Point<T>& b, const Point<T>& c, const T& epsilon = T(0))
    {
        T cross = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);

        if (cross >  epsilon) return  1;
        if (cross < -epsilon) return -1;
        return 0;
    }

    inline const char* orientationNameRu(int o)
    {
        switch (o) {
        case  1: return "Против часовой стрелки (левый поворот)";
        case -1: return "По часовой стрелке (правый поворот)";
        case  0: return "Коллинеарны";
        }
        return "Неизвестно";
    }



    template <typename T>
inline int orientationSegment(const Point<T>& a, const Point<T>& b, const Point<T>& c, const T& epsilon = T(0))
{
    const int flag = orientation(a, b, c, epsilon);
    if (flag != 0) {
        return flag;
    }

    const T minx = (a.x < b.x ? a.x : b.x) - epsilon;
    const T maxx = (a.x > b.x ? a.x : b.x) + epsilon;
    const T miny = (a.y < b.y ? a.y : b.y) - epsilon;
    const T maxy = (a.y > b.y ? a.y : b.y) + epsilon;

    if (minx <= c.x && c.x <= maxx && miny <= c.y && c.y <= maxy) {
        return -2; // Точка лежит на отрезке.
    }

    return 0;
}

    template <typename T>
inline int orientationConvexHull(const QVector<Point<T>>& pts, const Point<T> dot, const T& epsilon = T(0))
{
    const int n = pts.size();
    if (n < 3) {
        return 0;
    }

    auto onSegment = [&](const Point<T>& a, const Point<T>& b, const Point<T>& p) {
        const T minx = (a.x < b.x ? a.x : b.x) - epsilon;
        const T maxx = (a.x > b.x ? a.x : b.x) + epsilon;
        const T miny = (a.y < b.y ? a.y : b.y) - epsilon;
        const T maxy = (a.y > b.y ? a.y : b.y) + epsilon;
        return minx <= p.x && p.x <= maxx && miny <= p.y && p.y <= maxy;
    };

    int sign = 0;
    bool onBoundary = false;

    for (int i = 0; i < n; ++i) {
        const Point<T>& a = pts[i];
        const Point<T>& b = pts[(i + 1) % n];

        // Пропускаем выроженные нулевые рёбра.
        if (a == b) {
            continue;
        }

        const int o = orientation(a, b, dot, epsilon);

        if (o == 0) {
            if (onSegment(a, b, dot)) {
                onBoundary = true;
            }
            continue;
        }

        if (sign == 0) {
            sign = o;
        } else if (o != sign) {
            return -1;
        }
    }

    if (onBoundary) {
        return 0;
    }

    if (sign == 0) {
        return 0;
    }

    return 1;
}
}


namespace segment_intersection
{
    //При условии предикат = 0
    template <typename T>
    inline bool on_segment(const Point<T>& a, const Point<T>& b, const Point<T>& c, const T& epsilon = T(0))
    {
        const T minx = (a.x < b.x ? a.x : b.x) - epsilon;
        const T maxx = (a.x > b.x ? a.x : b.x) + epsilon;
        const T miny = (a.y < b.y ? a.y : b.y) - epsilon;
        const T maxy = (a.y > b.y ? a.y : b.y) + epsilon;

        return minx <= c.x && c.x <= maxx && miny <= c.y && c.y <= maxy;
    }



    template <typename T>
    inline int orientation_segment(const Point<T>& a, const Point<T>& b, const Point<T>& c, const Point<T>& d, const T& epsilon = T(0))
    {
        const int oabc = predicate::orientation(a, b, c, epsilon);
        const int oabd = predicate::orientation(a, b, d, epsilon);
        const int ocda = predicate::orientation(c, d, a, epsilon);
        const int ocdb = predicate::orientation(c, d, b, epsilon);


        if ((oabc != oabd) && (ocda != ocdb)) return 0;

        if ((oabc == 0) && on_segment(a, b, c, epsilon)) return 0;
        if ((oabd == 0) && on_segment(a, b, d, epsilon)) return 0;
        if ((ocda == 0) && on_segment(c, d, a, epsilon)) return 0;
        if ((ocdb == 0) && on_segment(c, d, b, epsilon)) return 0;

        if ((oabc == 0) && (oabd == 0) && (ocda == 0) && (ocdb == 0)) {
            const T min_ab_x = (a.x < b.x ? a.x : b.x);
            const T max_ab_x = (a.x > b.x ? a.x : b.x);
            const T min_cd_x = (c.x < d.x ? c.x : d.x);
            const T max_cd_x = (c.x > d.x ? c.x : d.x);

            const T min_ab_y = (a.y < b.y ? a.y : b.y);
            const T max_ab_y = (a.y > b.y ? a.y : b.y);
            const T min_cd_y = (c.y < d.y ? c.y : d.y);
            const T max_cd_y = (c.y > d.y ? c.y : d.y);

            const bool overlap_x = (min_ab_x <= max_cd_x + epsilon) && (min_cd_x <= max_ab_x + epsilon);
            const bool overlap_y = (min_ab_y <= max_cd_y + epsilon) && (min_cd_y <= max_ab_y + epsilon);


            if (overlap_x && overlap_y) return 2;
            return 1;
        }

        return -1;
    }


    template <typename T>
    inline Point<T> intersection_line(const Point<T>& a, const Point<T>& b, const Point<T>& c, const Point<T>& d, const T& epsilon = T(0))
    {
        T cross1 = a.x*b.y - a.y*b.x;
        T cross2 = c.x*d.y - c.y*d.x;

        T den = (a.x - b.x)*(c.y - d.y) - (a.y - b.y)*(c.x - d.x);

        if (predicate::geom_abs(den) <= epsilon)
            throw std::runtime_error("Прямые плохо обусловлены");

        T x = (cross1*(c.x - d.x) - cross2*(a.x - b.x))/den;
        T y = (cross1*(c.y - d.y) - cross2*(a.y - b.y))/den;

        return Point<T>(x, y);
    }
}


namespace convex_hull {


template <typename T>
inline QVector<int> algorithm_Graham(const QVector<Point<T>>& pts, const T& epsilon = T(0))
{
    const int n = pts.size();
    if (n == 0) {
        return QVector<int>();
    }

    if (n == 1) {
        QVector<int> res;
        res.push_back(0);
        return res;
    }

    std::vector<int> order;
    order.reserve(static_cast<size_t>(n));
    for (int i = 0; i < n; ++i) {
        order.push_back(i);
    }

    // Лексикографическая сортировка: сначала x, затем y.
    auto lessCoord = [&pts](int a, int b) {
        if (pts[a].x < pts[b].x) return true;
        if (pts[b].x < pts[a].x) return false;
        return pts[a].y < pts[b].y;
    };

    std::sort(order.begin(), order.end(), lessCoord);

    // Убираем дубликаты точек по координатам.
    auto equalCoord = [&pts](int a, int b) {
        return !(pts[a].x < pts[b].x) && !(pts[b].x < pts[a].x) &&
               !(pts[a].y < pts[b].y) && !(pts[b].y < pts[a].y);
    };

    std::vector<int> unique;
    unique.reserve(order.size());
    for (int idx : order) {
        if (unique.empty() || !equalCoord(idx, unique.back())) {
            unique.push_back(idx);
        }
    }

    if (unique.empty()) {
        return QVector<int>();
    }

    if (unique.size() == 1) {
        QVector<int> res;
        res.push_back(unique[0]);
        return res;
    }

    if (unique.size() == 2) {
        QVector<int> res;
        res.push_back(unique[0]);
        res.push_back(unique[1]);
        return res;
    }


    auto isRightOrCollinearTurn = [&pts, &epsilon](int a, int b, int c) {
        return predicate::orientation(pts[a], pts[b], pts[c], epsilon) != 1;
    };

    std::vector<int> lower;
    for (int idx : unique) {
        while (lower.size() >= 2 &&
               isRightOrCollinearTurn(lower[lower.size() - 2], lower.back(), idx)) {
            lower.pop_back();
        }
        lower.push_back(idx);
    }

    std::vector<int> upper;
    for (auto it = unique.rbegin(); it != unique.rend(); ++it) {
        const int idx = *it;
        while (upper.size() >= 2 &&
               isRightOrCollinearTurn(upper[upper.size() - 2], upper.back(), idx)) {
            upper.pop_back();
        }
        upper.push_back(idx);
    }

    // Стыкуем нижнюю и верхнюю цепочки.
    if (!lower.empty()) lower.pop_back();
    if (!upper.empty()) upper.pop_back();

    lower.insert(lower.end(), upper.begin(), upper.end());

    QVector<int> ans;
    ans.reserve(static_cast<int>(lower.size()));
    for (int idx : lower) {
        if (ans.isEmpty() || ans.last() != idx) {
            ans.push_back(idx);
        }
    }

    if (ans.size() > 1 && ans.first() == ans.last()) {
        ans.pop_back();
    }

    return ans;
}

}


namespace delaunay {

struct Triangle {
    int a;
    int b;
    int c;
};

inline double orient2d(const Point<double>& a,
                       const Point<double>& b,
                       const Point<double>& c)
{
    return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}


inline bool inCircumcircle(const Point<double>& a,
                           const Point<double>& b,
                           const Point<double>& c,
                           const Point<double>& p,
                           double eps)
{
    const double ax = a.x - p.x;
    const double ay = a.y - p.y;
    const double bx = b.x - p.x;
    const double by = b.y - p.y;
    const double cx = c.x - p.x;
    const double cy = c.y - p.y;

    const double a2 = ax * ax + ay * ay;
    const double b2 = bx * bx + by * by;
    const double c2 = cx * cx + cy * cy;

    double det = a2 * (bx * cy - by * cx)
               - b2 * (ax * cy - ay * cx)
               + c2 * (ax * by - ay * bx);

    if (orient2d(a, b, c) < 0.0) {
        det = -det;
    }

    return det > eps;
}


inline QVector<Triangle> triangulate(const QVector<Point<double>>& pts, double eps = 1e-12)
{
    const int inputCount = pts.size();
    if (inputCount < 3) {
        return QVector<Triangle>();
    }

    double minX = pts[0].x;
    double maxX = pts[0].x;
    double minY = pts[0].y;
    double maxY = pts[0].y;

    for (const Point<double>& p : pts) {
        if (p.x < minX) minX = p.x;
        if (p.x > maxX) maxX = p.x;
        if (p.y < minY) minY = p.y;
        if (p.y > maxY) maxY = p.y;
    }

    const double spanX = maxX - minX;
    const double spanY = maxY - minY;
    double span = (spanX > spanY) ? spanX : spanY;
    if (span < 1.0) span = 1.0;

    const double span2 = span * span;
    const double areaEps = 1e-12 * ((span2 > 1.0) ? span2 : 1.0);
    const double circleEps = eps * ((span2 > 1.0) ? (span2 * span2) : 1.0);


    QVector<Point<double>> uniquePts;
    QVector<int> originalIndex;
    uniquePts.reserve(inputCount);
    originalIndex.reserve(inputCount);

    for (int i = 0; i < inputCount; ++i) {
        bool duplicate = false;
        for (int j = 0; j < uniquePts.size(); ++j) {
            if (pts[i].x == uniquePts[j].x && pts[i].y == uniquePts[j].y) {
                duplicate = true;
                break;
            }
        }

        if (!duplicate) {
            uniquePts.push_back(pts[i]);
            originalIndex.push_back(i);
        }
    }

    if (uniquePts.size() < 3) {
        return QVector<Triangle>();
    }

    const int n = uniquePts.size();
    const double midX = (minX + maxX) * 0.5;
    const double midY = (minY + maxY) * 0.5;

    // Точки + три вершины
    QVector<Point<double>> P = uniquePts;
    P.push_back(Point<double>(midX - 20.0 * span, midY - span));
    P.push_back(Point<double>(midX + 20.0 * span, midY - span));
    P.push_back(Point<double>(midX, midY + 20.0 * span));

    std::vector<Triangle> triangles;
    triangles.push_back(Triangle{n, n + 1, n + 2});

    for (int i = 0; i < n; ++i) {
        std::vector<Triangle> bad;
        std::vector<Triangle> kept;
        bad.reserve(triangles.size());
        kept.reserve(triangles.size());

        for (const Triangle& t : triangles) {
            if (inCircumcircle(P[t.a], P[t.b], P[t.c], P[i], circleEps)) {
                bad.push_back(t);
            } else {
                kept.push_back(t);
            }
        }


        QVector<std::pair<int, int>> boundary;
        for (const Triangle& t : bad) {
            int verts[3] = {t.a, t.b, t.c};

            for (int e = 0; e < 3; ++e) {
                int u = verts[e];
                int v = verts[(e + 1) % 3];
                if (u > v) std::swap(u, v);

                const std::pair<int, int> edge(u, v);

                int found = -1;
                for (int k = 0; k < boundary.size(); ++k) {
                    if (boundary[k] == edge) {
                        found = k;
                        break;
                    }
                }

                if (found >= 0) {
                    boundary.remove(found);
                } else {
                    boundary.push_back(edge);
                }
            }
        }

        triangles = std::move(kept);

        for (const std::pair<int, int>& edge : boundary) {
            Triangle t{edge.first, edge.second, i};

            if (orient2d(P[t.a], P[t.b], P[t.c]) < 0.0) {
                std::swap(t.a, t.b);
            }

            if (predicate::geom_abs(orient2d(P[t.a], P[t.b], P[t.c])) > areaEps) {
                triangles.push_back(t);
            }
        }
    }

    QVector<Triangle> ans;
    for (const Triangle& t : triangles) {
        if (t.a >= n || t.b >= n || t.c >= n) {
            continue;
        }

        const double o = orient2d(P[t.a], P[t.b], P[t.c]);
        if (predicate::geom_abs(o) <= areaEps) {
            continue;
        }

        Triangle out = t;
        if (o < 0.0) {
            std::swap(out.a, out.b);
        }

        ans.push_back(Triangle{originalIndex[out.a], originalIndex[out.b], originalIndex[out.c]});
    }

    return ans;
}



inline long long cdtEdgeKey(int u, int v) {
    if (u > v) std::swap(u, v);
    return (static_cast<long long>(u) << 32) | static_cast<unsigned int>(v);
}

inline std::pair<int, int> cdtEdgeFromKey(long long key) {
    const int u = static_cast<int>(key >> 32);
    const int v = static_cast<int>(key & 0xffffffffLL);
    return {u, v};
}

inline double cdtSpan(const QVector<Point<double>>& P) {
    if (P.isEmpty()) return 1.0;

    double minX = P[0].x;
    double maxX = P[0].x;
    double minY = P[0].y;
    double maxY = P[0].y;

    for (const auto& p : P) {
        if (p.x < minX) minX = p.x;
        if (p.x > maxX) maxX = p.x;
        if (p.y < minY) minY = p.y;
        if (p.y > maxY) maxY = p.y;
    }

    double spanX = maxX - minX;
    double spanY = maxY - minY;
    double span = (spanX > spanY) ? spanX : spanY;
    if (span < 1.0) span = 1.0;
    return span;
}

inline double cdtAreaEpsilon(const QVector<Point<double>>& P) {
    const double span = cdtSpan(P);
    return 1e-12 * span * span;
}

inline double cdtCircleEpsilon(const QVector<Point<double>>& P, double eps) {
    const double span = cdtSpan(P);
    const double span2 = span * span;
    const double e = (eps > 0.0) ? eps : 1e-12;
    return e * ((span2 > 1.0) ? (span2 * span2) : 1.0);
}

inline void cdtEnsureCCW(Triangle& t, const QVector<Point<double>>& P) {
    if (orient2d(P[t.a], P[t.b], P[t.c]) < 0.0) {
        std::swap(t.a, t.b);
    }
}

inline bool cdtTriangleHasEdge(const Triangle& t, int u, int v) {
    const int verts[3] = {t.a, t.b, t.c};
    for (int i = 0; i < 3; ++i) {
        const int a = verts[i];
        const int b = verts[(i + 1) % 3];
        if ((a == u && b == v) || (a == v && b == u)) return true;
    }
    return false;
}

inline int cdtOppositeVertex(const Triangle& t, int u, int v) {
    if ((t.a == u && t.b == v) || (t.a == v && t.b == u)) return t.c;
    if ((t.b == u && t.c == v) || (t.b == v && t.c == u)) return t.a;
    if ((t.c == u && t.a == v) || (t.c == v && t.a == u)) return t.b;
    return -1;
}

inline std::unordered_map<long long, std::vector<int>>
cdtBuildEdgeMap(const std::vector<Triangle>& tris) {
    std::unordered_map<long long, std::vector<int>> edgeMap;

    for (int i = 0; i < static_cast<int>(tris.size()); ++i) {
        const Triangle& t = tris[i];
        const int verts[3] = {t.a, t.b, t.c};

        for (int e = 0; e < 3; ++e) {
            int a = verts[e];
            int b = verts[(e + 1) % 3];
            if (a == b) continue;

            edgeMap[cdtEdgeKey(a, b)].push_back(i);
        }
    }

    return edgeMap;
}

inline bool cdtEdgeExists(const std::vector<Triangle>& tris, int u, int v) {
    if (u == v) return true;

    for (const Triangle& t : tris) {
        if (cdtTriangleHasEdge(t, u, v)) return true;
    }

    return false;
}

inline bool cdtSegmentsCrossProperly(const Point<double>& a,
                                     const Point<double>& b,
                                     const Point<double>& c,
                                     const Point<double>& d,
                                     double eps)
{
    const double tol = (eps > 0.0) ? eps : 1e-12;

    const int o1 = predicate::orientation(a, b, c, tol);
    const int o2 = predicate::orientation(a, b, d, tol);
    const int o3 = predicate::orientation(c, d, a, tol);
    const int o4 = predicate::orientation(c, d, b, tol);

    return (o1 != 0) && (o2 != 0) && (o3 != 0) && (o4 != 0) &&
           (o1 != o2) && (o3 != o4);
}

inline bool cdtCanFlipEdge(const QVector<Point<double>>& P,
                           int a, int b, int c, int d,
                           double areaEps)
{
    if (a == b || c == d || a == c || a == d || b == c || b == d) {
        return false;
    }


    const int o1 = predicate::orientation(P[a], P[b], P[c], areaEps);
    const int o2 = predicate::orientation(P[a], P[b], P[d], areaEps);
    if (o1 == 0 || o2 == 0 || o1 == o2) return false;


    const int o3 = predicate::orientation(P[c], P[d], P[a], areaEps);
    const int o4 = predicate::orientation(P[c], P[d], P[b], areaEps);
    if (o3 == 0 || o4 == 0 || o3 == o4) return false;

    const double dx = P[c].x - P[d].x;
    const double dy = P[c].y - P[d].y;
    if (dx * dx + dy * dy <= 1e-24) return false;

    return true;
}

inline void cdtFlipEdge(std::vector<Triangle>& tris,
                        int i1, int i2,
                        int a, int b, int c, int d,
                        const QVector<Point<double>>& P)
{
    tris[i1] = Triangle{c, d, a};
    tris[i2] = Triangle{c, d, b};

    cdtEnsureCCW(tris[i1], P);
    cdtEnsureCCW(tris[i2], P);
}

inline std::vector<std::pair<int, int>> cdtPrepareConstraintEdges(
    const QVector<Point<double>>& P,
    const QVector<std::pair<int, int>>& constraints,
    double eps)
{
    std::vector<std::pair<int, int>> out;

    const int n = P.size();
    const double zeroTol = 1e-12;
    const double tol = (eps > 0.0) ? eps : zeroTol;
    const double span = cdtSpan(P);
    const double sortTol = zeroTol * span * span;

    for (const auto& constraint : constraints) {
        int u = constraint.first;
        int v = constraint.second;

        if (u < 0 || u >= n || v < 0 || v >= n || u == v) continue;

        const double dx = P[v].x - P[u].x;
        const double dy = P[v].y - P[u].y;
        const double len2 = dx * dx + dy * dy;
        if (len2 <= zeroTol * zeroTol) continue;


        std::vector<int> verts;

        const double minx = ((P[u].x < P[v].x) ? P[u].x : P[v].x) - tol;
        const double maxx = ((P[u].x > P[v].x) ? P[u].x : P[v].x) + tol;
        const double miny = ((P[u].y < P[v].y) ? P[u].y : P[v].y) - tol;
        const double maxy = ((P[u].y > P[v].y) ? P[u].y : P[v].y) + tol;

        for (int k = 0; k < n; ++k) {
            const bool endpoint = (k == u || k == v);
            const bool collinear =
                (predicate::orientation(P[u], P[v], P[k], tol) == 0);
            const bool insideBox =
                (minx <= P[k].x && P[k].x <= maxx &&
                 miny <= P[k].y && P[k].y <= maxy);

            if (endpoint || (collinear && insideBox)) {
                verts.push_back(k);
            }
        }

        if (verts.size() < 2) continue;

        auto projection = [&](int idx) {
            return (P[idx].x - P[u].x) * dx + (P[idx].y - P[u].y) * dy;
        };

        std::sort(verts.begin(), verts.end(), [&](int lhs, int rhs) {
            const double pl = projection(lhs);
            const double pr = projection(rhs);
            if (std::abs(pl - pr) > sortTol) return pl < pr;
            return lhs < rhs;
        });

        std::vector<int> ordered;
        ordered.reserve(verts.size());
        for (int idx : verts) {
            if (ordered.empty() || ordered.back() != idx) {
                ordered.push_back(idx);
            }
        }

        for (size_t i = 0; i + 1 < ordered.size(); ++i) {
            int a = ordered[i];
            int b = ordered[i + 1];
            if (a == b) continue;

            const double ex = P[b].x - P[a].x;
            const double ey = P[b].y - P[a].y;
            if (ex * ex + ey * ey <= zeroTol * zeroTol) continue;

            if (a > b) std::swap(a, b);
            out.push_back({a, b});
        }
    }

    std::sort(out.begin(), out.end(), [](const auto& lhs, const auto& rhs) {
        if (lhs.first != rhs.first) return lhs.first < rhs.first;
        return lhs.second < rhs.second;
    });
    out.erase(std::unique(out.begin(), out.end()), out.end());

    return out;
}

inline bool cdtInsertConstraintEdge(std::vector<Triangle>& tris,
                                    const QVector<Point<double>>& P,
                                    int u, int v,
                                    const std::unordered_set<long long>& protectedEdges,
                                    double eps,
                                    double areaEps)
{
    if (u == v) return true;
    if (cdtEdgeExists(tris, u, v)) return true;

    const int maxIterations = 10000 + static_cast<int>(tris.size()) * 8;

    for (int iter = 0; iter < maxIterations; ++iter) {
        if (cdtEdgeExists(tris, u, v)) return true;

        const auto edgeMap = cdtBuildEdgeMap(tris);
        std::unordered_set<long long> checked;
        bool flipped = false;


        for (const Triangle& t : tris) {
            const int verts[3] = {t.a, t.b, t.c};

            for (int e = 0; e < 3; ++e) {
                int a = verts[e];
                int b = verts[(e + 1) % 3];

                if (a == b || a == u || a == v || b == u || b == v) continue;

                const long long key = cdtEdgeKey(a, b);
                if (!checked.insert(key).second) continue;
                if (protectedEdges.count(key)) continue;

                if (!cdtSegmentsCrossProperly(P[u], P[v], P[a], P[b], eps)) {
                    continue;
                }

                const auto it = edgeMap.find(key);
                if (it == edgeMap.end() || it->second.size() != 2) continue;

                const int i1 = it->second[0];
                const int i2 = it->second[1];

                if (i1 == i2 || i1 < 0 || i2 < 0 ||
                    i1 >= static_cast<int>(tris.size()) ||
                    i2 >= static_cast<int>(tris.size())) {
                    continue;
                }

                const int c = cdtOppositeVertex(tris[i1], a, b);
                const int d = cdtOppositeVertex(tris[i2], a, b);

                if (c < 0 || d < 0 || c == d) continue;
                if (!cdtCanFlipEdge(P, a, b, c, d, areaEps)) continue;

                cdtFlipEdge(tris, i1, i2, a, b, c, d, P);
                flipped = true;
                break;
            }

            if (flipped) break;
        }

        if (!flipped) {
            return cdtEdgeExists(tris, u, v);
        }
    }

    return cdtEdgeExists(tris, u, v);
}

inline void cdtLegalize(std::vector<Triangle>& tris,
                        const QVector<Point<double>>& P,
                        const std::unordered_set<long long>& protectedEdges,
                        double areaEps,
                        double eps)
{
    const double circleEps = cdtCircleEpsilon(P, eps);
    const int maxIterations = 10000 + static_cast<int>(tris.size()) * 8;


    for (int iter = 0; iter < maxIterations; ++iter) {
        const auto edgeMap = cdtBuildEdgeMap(tris);
        bool flipped = false;

        for (const auto& kv : edgeMap) {
            if (protectedEdges.count(kv.first)) continue;
            if (kv.second.size() != 2) continue;

            const int i1 = kv.second[0];
            const int i2 = kv.second[1];

            if (i1 == i2 || i1 < 0 || i2 < 0 ||
                i1 >= static_cast<int>(tris.size()) ||
                i2 >= static_cast<int>(tris.size())) {
                continue;
            }

            const auto edge = cdtEdgeFromKey(kv.first);
            const int a = edge.first;
            const int b = edge.second;

            const int c = cdtOppositeVertex(tris[i1], a, b);
            const int d = cdtOppositeVertex(tris[i2], a, b);

            if (c < 0 || d < 0 || c == d) continue;

            const long long newKey = cdtEdgeKey(c, d);
            if (protectedEdges.count(newKey)) continue;
            if (!cdtCanFlipEdge(P, a, b, c, d, areaEps)) continue;

            const bool bad1 = inCircumcircle(P[a], P[b], P[c], P[d], circleEps);
            const bool bad2 = inCircumcircle(P[a], P[b], P[d], P[c], circleEps);

            if (!bad1 && !bad2) continue;

            cdtFlipEdge(tris, i1, i2, a, b, c, d, P);
            flipped = true;
            break;
        }

        if (!flipped) break;
    }
}

inline QVector<Triangle> triangulateConstrained(
    const QVector<Point<double>>& pts,
    const QVector<std::pair<int, int>>& constraints,
    double eps = 1e-12)
{
    const int inputCount = pts.size();
    if (inputCount < 3) return QVector<Triangle>();


    QVector<Point<double>> uniquePts;
    QVector<int> originalIndex;
    std::vector<int> originalToUnique;

    uniquePts.reserve(inputCount);
    originalIndex.reserve(inputCount);
    originalToUnique.reserve(static_cast<size_t>(inputCount));

    for (int i = 0; i < inputCount; ++i) {
        int found = -1;

        for (int j = 0; j < static_cast<int>(uniquePts.size()); ++j) {
            if (pts[i].x == uniquePts[j].x && pts[i].y == uniquePts[j].y) {
                found = j;
                break;
            }
        }

        if (found < 0) {
            found = static_cast<int>(uniquePts.size());
            uniquePts.push_back(pts[i]);
            originalIndex.push_back(i);
        }

        originalToUnique.push_back(found);
    }

    if (uniquePts.size() < 3) return QVector<Triangle>();

    const int n = uniquePts.size();


    QVector<std::pair<int, int>> mappedConstraints;
    mappedConstraints.reserve(constraints.size());

    for (const auto& c : constraints) {
        if (c.first < 0 || c.first >= inputCount ||
            c.second < 0 || c.second >= inputCount) {
            continue;
        }

        const int u = originalToUnique[c.first];
        const int v = originalToUnique[c.second];

        if (u == v) continue;

        if (u >= 0 && u < n && v >= 0 && v < n) {
            mappedConstraints.push_back({u, v});
        }
    }


    const QVector<Triangle> base = triangulate(uniquePts, eps);

    std::vector<Triangle> tris;
    tris.reserve(static_cast<size_t>(base.size()));
    for (const Triangle& t : base) {
        tris.push_back(t);
    }


    const std::vector<std::pair<int, int>> constraintEdges =
        cdtPrepareConstraintEdges(uniquePts, mappedConstraints, eps);

    std::unordered_set<long long> protectedEdges;
    protectedEdges.reserve(constraintEdges.size() * 2 + 1);

    for (const auto& e : constraintEdges) {
        protectedEdges.insert(cdtEdgeKey(e.first, e.second));
    }

    const double areaEps = cdtAreaEpsilon(uniquePts);


    for (const auto& e : constraintEdges) {
        cdtInsertConstraintEdge(tris, uniquePts, e.first, e.second,
                                protectedEdges, eps, areaEps);
    }


    cdtLegalize(tris, uniquePts, protectedEdges, areaEps, eps);


    QVector<Triangle> ans;
    ans.reserve(static_cast<int>(tris.size()));

    for (const Triangle& t : tris) {
        if (t.a < 0 || t.a >= n ||
            t.b < 0 || t.b >= n ||
            t.c < 0 || t.c >= n) {
            continue;
        }

        if (t.a == t.b || t.b == t.c || t.c == t.a) continue;

        const double o = orient2d(uniquePts[t.a], uniquePts[t.b], uniquePts[t.c]);
        if (predicate::geom_abs(o) <= areaEps) continue;

        Triangle out = t;
        if (o < 0.0) std::swap(out.a, out.b);

        out.a = originalIndex[out.a];
        out.b = originalIndex[out.b];
        out.c = originalIndex[out.c];

        ans.push_back(out);
    }

    return ans;
}

}

namespace point_in_polygon
{
template <typename T>
inline bool parityIn(const Point<T>& p, const QVector<QVector<Point<T>>>& contours)
{
    int cnt = 0;
    for (const auto& c : contours) {
        const int n = c.size();
        for (int i = 0; i < n; ++i) {
            const Point<T>& a = c[i];
            const Point<T>& b = c[(i + 1) % n];
            if ((a.y > p.y) != (b.y > p.y)) {
                const T lhs = a.x * (b.y - a.y) + (p.y - a.y) * (b.x - a.x);
                const T rhs = p.x * (b.y - a.y);
                const bool xintGreater = (b.y > a.y) ? (lhs > rhs) : (lhs < rhs);
                if (xintGreater) ++cnt;
            }
        }
    }
    return (cnt & 1) == 1;
}

template <typename T>
inline bool onBoundary(const Point<T>& p,
                       const QVector<QVector<Point<T>>>& contours,
                       const T& epsilon = T(0))
{
    for (const auto& c : contours) {
        const int n = c.size();
        for (int i = 0; i < n; ++i) {
            const Point<T>& a = c[i];
            const Point<T>& b = c[(i + 1) % n];
            if (predicate::orientation(a, b, p, epsilon) == 0 &&
                segment_intersection::on_segment(a, b, p, epsilon)) {
                return true;
            }
        }
    }
    return false;
}
}
