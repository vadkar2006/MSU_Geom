#include "DelaunayTask.h"
#include "Geometry.h"

#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QPolygonF>
#include <QColor>
#include <QString>

#include <cmath>
#include <unordered_map>
#include <vector>
#include <algorithm>

void DelaunayTask::draw(QPainter& p, const QRect& rect, const QVector<Point<Quotient_Number>>& points,
                        int draggingIndex, int hoveredIndex, double offsetX, double offsetY, double scale)
{
    // Сетка и фон в экранных координатах.
    drawCoordinateGrid(p, rect, offsetX, offsetY, scale);

    const double invScale = (scale > 1e-12) ? (1.0 / scale) : 1.0;

    // Геометрия рисуется в мировых координатах.
    p.save();
    p.translate(offsetX, offsetY);
    p.scale(scale, -scale); // Y направлен вверх.

    if (points.isEmpty()) {
        p.restore();

        p.save();
        p.resetTransform();
        p.setPen(Qt::darkGray);
        p.setFont(QFont("Arial", 12));
        p.drawText(20, 40, QStringLiteral("Кликните для добавления точек"));
        p.restore();
        return;
    }


    const QVector<Point<double>> dpts = toDoubleVector(points);
    const QVector<delaunay::Triangle> tris = delaunay::triangulate(dpts, 1e-12);


if (!tris.isEmpty()) {
    std::unordered_map<long long, std::vector<int>> edgeToTriangles;

    auto makeEdgeKey = [](int u, int v) -> long long {
        if (u > v) {
            std::swap(u, v);
        }
        return (static_cast<long long>(u) << 32) | static_cast<unsigned int>(v);
    };

    auto addEdge = [&](int triangleIndex, int u, int v) {
        if (u < 0 || u >= points.size() || v < 0 || v >= points.size()) {
            return;
        }
        edgeToTriangles[makeEdgeKey(u, v)].push_back(triangleIndex);
    };

    for (int i = 0; i < tris.size(); ++i) {
        const auto& t = tris[i];
        if (t.a < 0 || t.a >= points.size() ||
            t.b < 0 || t.b >= points.size() ||
            t.c < 0 || t.c >= points.size()) {
            continue;
        }
        addEdge(i, t.a, t.b);
        addEdge(i, t.b, t.c);
        addEdge(i, t.c, t.a);
    }

    std::vector<std::vector<int>> adjacency(tris.size());
    for (const auto& kv : edgeToTriangles) {
        const auto& ids = kv.second;
        for (size_t i = 0; i < ids.size(); ++i) {
            for (size_t j = i + 1; j < ids.size(); ++j) {
                if (ids[i] == ids[j]) {
                    continue;
                }
                adjacency[ids[i]].push_back(ids[j]);
                adjacency[ids[j]].push_back(ids[i]);
            }
        }
    }

    std::vector<int> colorIndex(tris.size(), 0);
    for (int i = 0; i < tris.size(); ++i) {
        bool used[32] = {};
        for (int nb : adjacency[i]) {
            const int c = colorIndex[nb];
            if (c >= 0 && c < 32) {
                used[c] = true;
            }
        }
        int chosen = 0;
        while (chosen < 32 && used[chosen]) {
            ++chosen;
        }
        if (chosen == 32) {
            chosen = 0;
        }
        colorIndex[i] = chosen;
    }

    QVector<QColor> palette;
    palette.reserve(32);


    for (int i = 0; i < 32; ++i) {
        const double hue = std::fmod(0.50 + 0.035 * (i % 12), 1.0);
        const double saturation = 0.42 + 0.06 * ((i / 3) % 3);
        const double lightness = 0.64 + 0.05 * (i % 3);
        palette.push_back(QColor::fromHslF(hue, saturation, lightness, 0.72));
    }

    p.setPen(Qt::NoPen);
    for (int i = 0; i < tris.size(); ++i) {
        const auto& t = tris[i];
        if (t.a < 0 || t.a >= points.size() ||
            t.b < 0 || t.b >= points.size() ||
            t.c < 0 || t.c >= points.size()) {
            continue;
        }
        QPolygonF poly;
        poly << points[t.a].toQPointF()
             << points[t.b].toQPointF()
             << points[t.c].toQPointF();
        p.setBrush(palette[colorIndex[i]]);
        p.drawPolygon(poly);
    }


    auto circumCircle = [&](int ia, int ib, int ic, QPointF& center, double& radius) -> bool {
        const double ax = dpts[ia].x;
        const double ay = dpts[ia].y;
        const double bx = dpts[ib].x;
        const double by = dpts[ib].y;
        const double cx = dpts[ic].x;
        const double cy = dpts[ic].y;

        const double a2 = ax * ax + ay * ay;
        const double b2 = bx * bx + by * by;
        const double c2 = cx * cx + cy * cy;

        const double d = 2.0 * (ax * (by - cy) + bx * (cy - ay) + cx * (ay - by));
        if (std::abs(d) <= 1e-12) {
            return false;
        }

        const double ux = (a2 * (by - cy) + b2 * (cy - ay) + c2 * (ay - by)) / d;
        const double uy = (a2 * (cx - bx) + b2 * (ax - cx) + c2 * (bx - ax)) / d;

        center = QPointF(ux, uy);
        radius = std::hypot(ax - ux, ay - uy);

        return std::isfinite(center.x()) && std::isfinite(center.y()) && std::isfinite(radius);
    };

    const double viewWorld = (rect.width() + rect.height()) * invScale;
    const double maxRadius = viewWorld * 5.0 + 1.0;

    for (int i = 0; i < tris.size(); ++i) {
        const auto& t = tris[i];
        if (t.a < 0 || t.a >= points.size() ||
            t.b < 0 || t.b >= points.size() ||
            t.c < 0 || t.c >= points.size()) {
            continue;
        }

        QPointF center;
        double radius = 0.0;
        if (!circumCircle(t.a, t.b, t.c, center, radius)) {
            continue;
        }

        if (!std::isfinite(radius) || radius <= 0.0 || radius > maxRadius) {
            continue;
        }

        bool violates = false;
        const double r2 = radius * radius;
        const double checkEps = 1e-7 * std::max(1.0, r2 * r2);

        for (int j = 0; j < dpts.size(); ++j) {
            if (j == t.a || j == t.b || j == t.c) {
                continue;
            }
            if (delaunay::inCircumcircle(dpts[t.a], dpts[t.b], dpts[t.c], dpts[j], checkEps)) {
                violates = true;
                break;
            }
        }

        const QColor circleColor = violates ? QColor(210, 70, 70, 160) : QColor(70, 120, 180, 105);
        p.setPen(QPen(circleColor, (violates ? 2.0 : 1.3) * invScale, Qt::DashLine));
        p.setBrush(Qt::NoBrush);
        p.drawEllipse(center, radius, radius);
    }

    // Рёбра треугольников.
    p.setPen(QPen(QColor(40, 90, 180), 2.0 * invScale));
    for (const auto& t : tris) {
        if (t.a < 0 || t.a >= points.size() ||
            t.b < 0 || t.b >= points.size() ||
            t.c < 0 || t.c >= points.size()) {
            continue;
        }
        p.drawLine(points[t.a].toQPointF(), points[t.b].toQPointF());
        p.drawLine(points[t.b].toQPointF(), points[t.c].toQPointF());
        p.drawLine(points[t.c].toQPointF(), points[t.a].toQPointF());
    }
}

    // Epsilon-окрестность, если включена.
    if (m_showEpsilonNeighborhood) {
        const double r = (m_epsilon > 0.0) ? std::sqrt(m_epsilon) : 0.0;
        if (r > 0.0) {
            QPen epsPen(QColor(255, 100, 0), 2.0 * invScale, Qt::DashLine);
            p.setPen(epsPen);
            p.setBrush(Qt::NoBrush);

            for (const auto& pt : points) {
                p.drawEllipse(pt.toQPointF(), r, r);
            }
        }
    }

    // Точки.
    for (int i = 0; i < points.size(); ++i) {
        const Point<double> pt = points[i].toDoublePoint();
        const bool isActive = (i == draggingIndex || i == hoveredIndex);

        p.setBrush(QColor(220, 80, 80));
        p.setPen(isActive ? QPen(QColor(255, 180, 0), 3.0 * invScale)
                          : QPen(Qt::black, 2.0 * invScale));

        p.drawEllipse(pt.toQPointF(),
                      m_pointRadius * invScale,
                      m_pointRadius * invScale);
    }

    p.restore();

    // Подписи индексов точек.
    if (points.size() <= 200) {
        p.save();
        p.resetTransform();

        p.setPen(QColor(50, 50, 50));
        p.setFont(QFont("Arial", 9));

        for (int i = 0; i < points.size(); ++i) {
            const Point<double> pt = points[i].toDoublePoint();
            const QPointF screen(pt.x * scale + offsetX, offsetY - pt.y * scale);
            p.drawText(screen + QPointF(14.0, -10.0), QString::number(i));
        }

        p.restore();
    }

    // Статус.
    p.save();
    p.resetTransform();

    p.setPen(Qt::black);
    p.setFont(QFont("Arial", 14, QFont::Bold));
    p.drawText(20, rect.height() - 60,
               QStringLiteral("Триангуляция Делоне: точек = %1, треугольников = %2")
                   .arg(points.size())
                   .arg(tris.size()));

    p.setFont(QFont("Arial", 10));
    p.setPen(Qt::darkBlue);

    if (useBigNum()) {
        p.drawText(20, rect.height() - 35,
                   QStringLiteral("Координаты хранятся в Quotient_Number; расчёт триангуляции выполняется в double"));
    } else {
        p.drawText(20, rect.height() - 35,
                   QStringLiteral("Режим: double"));
    }

    if (points.size() < 3) {
        p.setPen(Qt::darkGray);
        p.drawText(20, rect.height() - 85,
                   QStringLiteral("Для построения нужно минимум 3 точки"));
    }

    p.restore();
}
