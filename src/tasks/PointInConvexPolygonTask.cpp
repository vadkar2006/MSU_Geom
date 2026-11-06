#include "PointInConvexPolygonTask.h"
#include "Geometry.h"
#include <QPainter>
#include <QMouseEvent>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QPolygonF>
#include <QColor>
#include <QString>
#include <cmath>

namespace {
template <typename T>
bool isConvexPolygon(const QVector<Point<T>>& poly, const T& epsilon)
{
    if (poly.size() < 3) {
        return false;
    }

    bool hasPositive = false;
    bool hasNegative = false;
    const int n = poly.size();

    for (int i = 0; i < n; ++i) {
        const int o = predicate::orientation(poly[i], poly[(i + 1) % n], poly[(i + 2) % n], epsilon);
        if (o > 0) {
            hasPositive = true;
        } else if (o < 0) {
            hasNegative = true;
        }

        if (hasPositive && hasNegative) {
            return false;
        }
    }

    return hasPositive || hasNegative;
}


QString statusTextRu(int status)
{
    switch (status) {
    case 1:  return QStringLiteral("внутри");
    case -1: return QStringLiteral("снаружи");
    default: return QStringLiteral("на границе");
    }
}

QColor statusColor(int status)
{
    switch (status) {
    case 1:  return QColor(60, 170, 90);
    case -1: return QColor(200, 60, 60);
    default: return QColor(230, 180, 60);
    }
}

} // namespace

void PointInConvexPolygonTask::sanitizeState(const QVector<Point<Quotient_Number>>& points)
{
    if (points.isEmpty()) {
        m_polygonCount = -1;
        m_lastPressAddedPoint = false;
    m_savedPolygon.clear();
        return;
    }

    if (m_polygonCount < 0) {
        return;
    }

    if (!m_savedPolygon.isEmpty() && m_savedPolygon.size() != m_polygonCount) {
        m_savedPolygon.clear();
    }


    if (points.size() < 3 ||
        m_polygonCount > points.size() ||
        points.size() > m_polygonCount + 1) {
        m_polygonCount = -1;
        m_savedPolygon.clear();
    }
}

void PointInConvexPolygonTask::buildConvexHull(QVector<Point<Quotient_Number>>& points)
{
    if (m_lastPressAddedPoint && !points.isEmpty()) {
        points.removeLast();
        m_lastPressAddedPoint = false;
    }

    if (points.size() < 3) {
        return;
    }

    QVector<int> hull;
    if (useBigNum()) {
        hull = convex_hull::algorithm_Graham(points, Quotient_Number(0));
    } else {
        hull = convex_hull::algorithm_Graham(toDoubleVector(points), m_epsilon);
    }

    if (hull.size() < 3) {
        return;
    }

    QVector<Point<Quotient_Number>> hullPoints;
    hullPoints.reserve(hull.size());
    for (int idx : hull) {
        if (idx >= 0 && idx < points.size()) {
            hullPoints.push_back(points[idx]);
        }
    }

    if (hullPoints.size() < 3) {
        return;
    }

    points = hullPoints;
    m_polygonCount = static_cast<int>(points.size());
    m_savedPolygon = points;
}

void PointInConvexPolygonTask::updateSavedPolygon(const QVector<Point<Quotient_Number>>& points)
{
    if (m_polygonCount < 3 || points.size() < m_polygonCount) {
        return;
    }

    const QVector<Point<Quotient_Number>> poly = points.mid(0, m_polygonCount);
    bool convex = false;

    if (useBigNum()) {
        convex = isConvexPolygon(poly, Quotient_Number(0));
    } else {
        convex = isConvexPolygon(toDoubleVector(poly), m_epsilon);
    }

    if (convex) {
        m_savedPolygon = poly;
    }
}

int PointInConvexPolygonTask::pointStatus(const QVector<Point<Quotient_Number>>& points) const
{
    if (!hasQueryPoint(points)) {
        return 0;
    }

    QVector<Point<Quotient_Number>> poly = points.mid(0, m_polygonCount);
    const Point<Quotient_Number> query = points.last();


    bool useSaved = false;
    if (!m_savedPolygon.isEmpty() && m_savedPolygon.size() >= 3) {
        if (poly.size() != m_savedPolygon.size()) {
            useSaved = true;
        } else {
            if (useBigNum()) {
                if (!isConvexPolygon(poly, Quotient_Number(0))) {
                    useSaved = true;
                }
            } else {
                const QVector<Point<double>> polyD = toDoubleVector(poly);
                if (!isConvexPolygon(polyD, m_epsilon)) {
                    useSaved = true;
                }
            }
        }
    }

    if (useSaved) {
        poly = m_savedPolygon;
    }

    if (useBigNum()) {
        return predicate::orientationConvexHull(poly, query, Quotient_Number(0));
    }

    const QVector<Point<double>> polyD = toDoubleVector(poly);
    return predicate::orientationConvexHull(polyD, query.toDoublePoint(), m_epsilon);
}

void PointInConvexPolygonTask::handleMousePress(QMouseEvent* event,
                                                QVector<Point<Quotient_Number>>& points,
                                                int& draggingIndex,
                                                int& hoveredIndex)
{
    sanitizeState(points);

    if (event->button() == Qt::LeftButton) {

        if (event->type() == QEvent::MouseButtonDblClick) {
            if (m_polygonCount < 0) {
                buildConvexHull(points);
            }
            return;
        }


        if (m_polygonCount < 0) {
            const int before = static_cast<int>(points.size());
            StandardPointTask::handleMousePress(event, points, draggingIndex, hoveredIndex);
            m_lastPressAddedPoint = (static_cast<int>(points.size()) > before);
            return;
        }


        const int idx = pointIndexAt(event->pos(), points);
        if (idx != -1) {
            draggingIndex = idx;
            return;
        }
        if (!hasQueryPoint(points) && canAddPointAt(event->pos(), points)) {
            points.push_back(makePointFromScene(event->pos()));
        }
        return;
    }

    if (event->button() == Qt::RightButton) {
        const int idx = pointIndexAt(event->pos(), points);
        if (idx == -1) {
            return;
        }

        if (m_polygonCount < 0) {
            StandardPointTask::handleMousePress(event, points, draggingIndex, hoveredIndex);
            return;
        }

        const bool hadP = hasQueryPoint(points);
        if (hadP && idx == points.size() - 1) {
            points.removeLast();
        } else if (idx < m_polygonCount) {
            points.removeAt(idx);
            m_polygonCount--;
            if (m_polygonCount < 3) {
                m_polygonCount = -1;
            }
        }
        draggingIndex = -1;
        hoveredIndex = -1;
        return;
    }

    StandardPointTask::handleMousePress(event, points, draggingIndex, hoveredIndex);
}


void PointInConvexPolygonTask::draw(QPainter& p, const QRect& rect,
                                    const QVector<Point<Quotient_Number>>& points,
                                    int draggingIndex, int hoveredIndex,
                                    double offsetX, double offsetY, double scale)
{
    sanitizeState(points);
    updateSavedPolygon(points);

    drawCoordinateGrid(p, rect, offsetX, offsetY, scale);

    const double invScale = (scale > 1e-12) ? (1.0 / scale) : 1.0;
    const bool polygonBuilt = (m_polygonCount >= 3 && m_polygonCount <= points.size());
    const bool hasP = polygonBuilt && hasQueryPoint(points);
    const int status = hasP ? pointStatus(points) : 0;


    p.save();
    p.translate(offsetX, offsetY);
    p.scale(scale, -scale); // Y направлен вверх


    if (!m_savedPolygon.isEmpty()) {
        QPolygonF savedPoly;
        for (const auto& sp : m_savedPolygon) {
            savedPoly << sp.toQPointF();
        }

        p.setPen(Qt::NoPen);
        p.setBrush(QColor(170, 180, 200, 55));
        p.drawPolygon(savedPoly);

        p.setPen(QPen(QColor(110, 125, 160, 110), 2.0 * invScale, Qt::DashLine));
        p.drawPolygon(savedPoly);
    }

    if (!points.isEmpty() && !polygonBuilt) {

        p.setPen(QPen(QColor(100, 130, 210), 2.0 * invScale, Qt::DashLine));
        for (int i = 1; i < points.size(); ++i) {
            p.drawLine(points[i - 1].toQPointF(), points[i].toQPointF());
        }
    }

    if (polygonBuilt) {
        QPolygonF poly;
        for (int i = 0; i < m_polygonCount; ++i) {
            poly << points[i].toQPointF();
        }

        // Полупрозрачная заливка многоугольника.
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(180, 186, 200, 130));
        p.drawPolygon(poly);

        // Контур многоугольника.
        p.setBrush(Qt::NoBrush);
        p.setPen(QPen(QColor(60, 80, 130), 2.5 * invScale));
        p.drawPolygon(poly);

        // Вершины многоугольника
        for (int i = 0; i < m_polygonCount; ++i) {
            const bool isActive = (i == draggingIndex || i == hoveredIndex);
            p.setBrush(QColor(250, 250, 250));
            p.setPen(isActive ? QPen(QColor(255, 180, 0), 3.0 * invScale)
                              : QPen(QColor(90, 120, 190), 2.0 * invScale));
            p.drawEllipse(points[i].toQPointF(),
                          m_pointRadius * 0.6 * invScale,
                          m_pointRadius * 0.6 * invScale);
        }

        // Точка P
        if (hasP) {
            const Point<double> pt = points.last().toDoublePoint();
            const int lastIdx = static_cast<int>(points.size()) - 1;
            const bool isActive = (lastIdx == draggingIndex || lastIdx == hoveredIndex);
            p.setBrush(statusColor(status));
            p.setPen(isActive ? QPen(QColor(255, 180, 0), 3.0 * invScale)
                              : QPen(Qt::black, 2.0 * invScale));
            p.drawEllipse(pt.toQPointF(), m_pointRadius * invScale, m_pointRadius * invScale);
        }
    } else {

        for (int i = 0; i < points.size(); ++i) {
            const Point<double> pt = points[i].toDoublePoint();
            const bool isActive = (i == draggingIndex || i == hoveredIndex);
            p.setBrush(QColor(220, 80, 80));
            p.setPen(isActive ? QPen(QColor(255, 180, 0), 3.0 * invScale)
                              : QPen(Qt::black, 2.0 * invScale));
            p.drawEllipse(pt.toQPointF(), m_pointRadius * invScale, m_pointRadius * invScale);
        }
    }

    // Epsilon-окрестность
    if (m_showEpsilonNeighborhood) {
        const double r = (m_epsilon > 0.0) ? std::sqrt(m_epsilon) : 0.0;
        if (r > 0.0) {
            p.setPen(QPen(QColor(255, 100, 0), 2.0 * invScale, Qt::DashLine));
            p.setBrush(Qt::NoBrush);
            for (const auto& pt : points) {
                p.drawEllipse(pt.toQPointF(), r, r);
            }
        }
    }

    p.restore();


    p.save();
    p.resetTransform();
    p.setFont(QFont("Arial", 9));
    p.setPen(QColor(90, 120, 190));
    const int labelCount = polygonBuilt ? m_polygonCount : static_cast<int>(points.size());
    for (int i = 0; i < labelCount; ++i) {
        const Point<double> pt = points[i].toDoublePoint();
        const QPointF screen(pt.x * scale + offsetX, offsetY - pt.y * scale);
        p.drawText(screen + QPointF(10.0, -10.0), QString::number(i));
    }
    if (hasP) {
        const Point<double> pt = points.last().toDoublePoint();
        const QPointF screen(pt.x * scale + offsetX, offsetY - pt.y * scale);
        p.setFont(QFont("Arial", 12, QFont::Bold));
        p.setPen(statusColor(status));
        p.drawText(screen + QPointF(16.0, -12.0),
                   QStringLiteral("P: %1").arg(statusTextRu(status)));
    }
    p.restore();


    p.save();
    p.resetTransform();
    const QRect legendRect(rect.right() - 160, rect.top() + 12, 150, 70);
    p.setPen(QPen(QColor(200, 200, 200), 1));
    p.setBrush(QColor(255, 255, 255, 210));
    p.drawRect(legendRect);
    p.setFont(QFont("Arial", 9));
    const QColor legendColors[3] = { statusColor(1), statusColor(-1), statusColor(0) };
    const QString legendTexts[3] = { statusTextRu(1), statusTextRu(-1), statusTextRu(0) };
    for (int i = 0; i < 3; ++i) {
        const int y = legendRect.top() + 16 + i * 18;
        p.setPen(Qt::NoPen);
        p.setBrush(legendColors[i]);
        p.drawRoundedRect(QRect(legendRect.left() + 10, y - 10, 12, 12), 3, 3);
        p.setPen(QColor(60, 60, 60));
        p.drawText(legendRect.left() + 30, y, legendTexts[i]);
    }
    p.restore();


    p.save();
    p.resetTransform();
    p.setFont(QFont("Arial", 14, QFont::Bold));
    QString info;
    QColor infoColor = Qt::black;
    if (!polygonBuilt) {
        info = QStringLiteral("Вершин: %1. Двойной щелчок ЛКМ - построить выпуклую оболочку.")
                   .arg(points.size());
    } else if (!hasP) {
        info = QStringLiteral("Оболочка построена: %1 вершин. Следующий клик поставит точку P.")
                   .arg(m_polygonCount);
    } else {
        info = QStringLiteral("P %1. Вершин многоугольника: %2")
                   .arg(statusTextRu(status)).arg(m_polygonCount);
        infoColor = statusColor(status);
    }
    p.setPen(infoColor);
    p.drawText(20, rect.height() - 60, info);
    p.setFont(QFont("Arial", 10));
    p.setPen(Qt::darkGray);
    p.drawText(20, rect.height() - 35,
               QStringLiteral("ПКМ - удалить точку; перетаскивание работает для всех точек"));
    p.restore();
}
