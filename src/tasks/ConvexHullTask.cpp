#include "ConvexHullTask.h"
#include "Geometry.h"
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QPolygonF>
#include <QColor>
#include <QString>
#include <cmath>

QVector<int> ConvexHullTask::hullIndices(const QVector<Point<Quotient_Number>>& points,
                                         bool exact,
                                         double epsilon)
{
    if (points.isEmpty()) {
        return QVector<int>();
    }

    if (exact) {
        return convex_hull::algorithm_Graham(points, Quotient_Number(0));
    }

    const QVector<Point<double>> pts = toDoubleVector(points);
    const double eps = (epsilon > 0.0) ? epsilon : 0.0;
    return convex_hull::algorithm_Graham(pts, eps);
}

void ConvexHullTask::updateGhostHull(const QVector<Point<Quotient_Number>>& points)
{
    if (points.size() < 3) {
        return;
    }

    const QVector<int> hull = hullIndices(points, useBigNum(), m_epsilon);
    if (hull.size() < 3) {
        return;
    }

    m_ghostHull.clear();
    for (int idx : hull) {
        if (idx >= 0 && idx < points.size()) {
            m_ghostHull.push_back(points[idx]);
        }
    }
}

void ConvexHullTask::handleMouseRelease(QMouseEvent* event, QVector<Point<Quotient_Number>>& points, int& draggingIndex)
{
    StandardPointTask::handleMouseRelease(event, points, draggingIndex);
    updateGhostHull(points);
}

void ConvexHullTask::draw(QPainter& p, const QRect& rect, const QVector<Point<Quotient_Number>>& points,
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
    m_ghostHull.clear();
        p.restore();

        p.save();
        p.resetTransform();
        p.setPen(Qt::darkGray);
        p.setFont(QFont("Arial", 12));
        p.drawText(20, 40, QStringLiteral("Кликните для добавления точек"));
        p.restore();
        return;
    }

    if (m_ghostHull.isEmpty()) {
        updateGhostHull(points);
    }

    // Блеклый сохранённый контур предыдущей оболочки.
    if (!m_ghostHull.isEmpty()) {
        QPolygonF ghostPoly;
        for (const auto& gp : m_ghostHull) {
            ghostPoly << gp.toQPointF();
        }

        p.setPen(Qt::NoPen);
        p.setBrush(QColor(130, 160, 190, 45));
        p.drawPolygon(ghostPoly);

        p.setPen(QPen(QColor(90, 120, 150, 120), 2.0 * invScale, Qt::DashLine));
        p.drawPolygon(ghostPoly);
    }

    const QVector<int> hull = hullIndices(points, useBigNum(), m_epsilon);

    QVector<char> onHull(points.size(), 0);
    QPolygonF hullPoly;

    for (int idx : hull) {
        if (idx >= 0 && idx < points.size()) {
            onHull[idx] = 1;
            hullPoly << points[idx].toQPointF();
        }
    }

    // Полупрозрачная область внутри оболочки.
    if (hullPoly.size() >= 3) {
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(80, 200, 120, 70));
        p.drawPolygon(hullPoly);
    }

    // Рёбра оболочки.
    if (hull.size() >= 2) {
        p.setPen(QPen(QColor(0, 130, 70), 3.0 * invScale));
        for (int i = 0; i + 1 < hull.size(); ++i) {
            p.drawLine(points[hull[i]].toQPointF(), points[hull[i + 1]].toQPointF());
        }
        if (hull.size() > 2) {
            p.drawLine(points[hull.last()].toQPointF(), points[hull.first()].toQPointF());
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

    // Точки. Вершины оболочки подсвечиваются зелёным.
    for (int i = 0; i < points.size(); ++i) {
        const Point<double> pt = points[i].toDoublePoint();

        const bool isHullPoint = (onHull[i] != 0);
        const bool isActive = (i == draggingIndex || i == hoveredIndex);

        QColor fillColor = isHullPoint ? QColor(60, 170, 90) : QColor(220, 80, 80);

        p.setBrush(fillColor);
        p.setPen(isActive ? QPen(QColor(255, 180, 0), 3.0 * invScale)
                          : QPen(Qt::black, 2.0 * invScale));

        p.drawEllipse(pt.toQPointF(),
                      m_pointRadius * invScale,
                      m_pointRadius * invScale);
    }

    p.restore();

    // Подписи индексов точек в экранных координатах.
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

    // Информационный текст.
    p.save();
    p.resetTransform();

    p.setPen(Qt::black);
    p.setFont(QFont("Arial", 14, QFont::Bold));
    p.drawText(20, rect.height() - 60,
               QStringLiteral("Выпуклая оболочка: точек = %1, вершин оболочки = %2")
                   .arg(points.size())
                   .arg(hull.size()));

    p.setFont(QFont("Arial", 10));
    p.setPen(Qt::darkBlue);

    if (useBigNum()) {
        p.drawText(20, rect.height() - 35,
                   QStringLiteral("Режим: точная арифметика (Quotient_Number)"));
    } else {
        p.drawText(20, rect.height() - 35,
                   QStringLiteral("Режим: double + эпсилон"));
    }

    p.restore();
}
