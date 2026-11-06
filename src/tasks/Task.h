#pragma once
#include <QPainter>
#include <QRect>
#include <QVector>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QWidget>
#include <QString>
#include <QStringList>
#include "Point.h"
#include "Quotient_Number.h"
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QPolygonF>
#include <QPointF>
#include <QColor>
#include <cmath>


inline void drawCoordinateGrid(QPainter& p,
                               const QRect& screenRect,
                               double offsetX,
                               double offsetY,
                               double scale)
{
    if (!std::isfinite(scale) || scale <= 0.0 || screenRect.isEmpty()) {
return;
}

    p.save();
    p.resetTransform();
    p.setRenderHint(QPainter::Antialiasing, true);

    // Фон рисуем до сетки, чтобы сетка была видна.
    p.fillRect(screenRect, QColor(250, 250, 250));

    const double worldLeft   = -offsetX / scale;
    const double worldRight  = worldLeft + screenRect.width() / scale;

    const double worldTop    = offsetY / scale;
    const double worldBottom = (offsetY - screenRect.height()) / scale;

if (!std::isfinite(worldLeft) || !std::isfinite(worldTop) || !std::isfinite(worldRight) || !std::isfinite(worldBottom)) {
        p.restore();
        return;
    }


    double targetScreenStep = 80.0;
double worldStep = targetScreenStep / scale;

if (!std::isfinite(worldStep) || worldStep <= 0.0) {
        p.restore();
        return;
    }

double logWorldStep = std::log10(worldStep);

if (!std::isfinite(logWorldStep)) {
        p.restore();
        return;
    }

logWorldStep = logWorldStep < -300.0 ? -300.0 : (logWorldStep > 300.0 ? 300.0 : logWorldStep);
double magnitude = std::pow(10.0, std::floor(logWorldStep));

if (!std::isfinite(magnitude) || magnitude <= 0.0) {
        p.restore();
        return;
    }

double normalized = worldStep / magnitude;

    double gridStep;
    if (normalized < 1.5) {
        gridStep = 1.0 * magnitude;
    } else if (normalized < 3.5) {
        gridStep = 2.0 * magnitude;
    } else if (normalized < 7.5) {
        gridStep = 5.0 * magnitude;
    } else {
        gridStep = 10.0 * magnitude;
    }

    int decimals = 0;
if (gridStep > 0.0 && gridStep < 1.0) {
    double logGridStep = std::log10(gridStep);
    if (std::isfinite(logGridStep)) {
        decimals = static_cast<int>(std::ceil(-logGridStep));
    }
    if (decimals < 1) decimals = 1;
    if (decimals > 12) decimals = 12;
}

    auto formatCoord = [decimals](double v) {
        if (std::abs(v) < 1e-12) {
            v = 0.0;
        }
        return QString::number(v, 'f', decimals);
    };

    auto clamp = [](double v, double lo, double hi) {
        if (hi < lo) {
            return lo;
        }
        if (v < lo) {
            return lo;
        }
        if (v > hi) {
            return hi;
        }
        return v;
    };

    // Основные линии сетки.
    p.setPen(QPen(QColor(220, 220, 220), 1));

    double startX = std::floor(worldLeft / gridStep) * gridStep;
if (!std::isfinite(startX)) startX = 0.0;
int safetyX = 0;
for (double wx = startX; wx <= worldRight + gridStep * 0.5 && safetyX < 10000; wx += gridStep, ++safetyX) {
        double sx = wx * scale + offsetX;
        p.drawLine(QPointF(sx, screenRect.top()),
                   QPointF(sx, screenRect.bottom()));
    }

    double startY = std::floor(worldBottom / gridStep) * gridStep;
    if (!std::isfinite(startY)) startY = 0.0;
    int safetyY = 0;
    for (double wy = startY; wy <= worldTop + gridStep * 0.5 && safetyY < 10000; wy += gridStep, ++safetyY) {
        double sy = offsetY - wy * scale;
        p.drawLine(QPointF(screenRect.left(), sy),
                   QPointF(screenRect.right(), sy));
    }

    // Оси координат.
    p.setPen(QPen(QColor(100, 100, 100), 2));
    p.setBrush(QColor(100, 100, 100));

    const double arrow = 10.0;
    const bool xAxisVisible = (worldBottom <= 0.0 && worldTop >= 0.0);
    const bool yAxisVisible = (worldLeft <= 0.0 && worldRight >= 0.0);

    if (xAxisVisible) {
        double sy = offsetY;
        p.drawLine(QPointF(screenRect.left(), sy),
                   QPointF(screenRect.right(), sy));

        QPolygonF arrowX;
        arrowX << QPointF(screenRect.right() - arrow, sy - arrow * 0.5)
               << QPointF(screenRect.right(), sy)
               << QPointF(screenRect.right() - arrow, sy + arrow * 0.5);
        p.drawPolygon(arrowX);

        p.setPen(Qt::black);
        p.setFont(QFont("Arial", 10, QFont::Bold));
        p.drawText(QPointF(screenRect.right() - 25, sy - 8), "X");

        p.setPen(QPen(QColor(100, 100, 100), 2));
    }

    if (yAxisVisible) {
        double sx = offsetX;
        p.drawLine(QPointF(sx, screenRect.top()),
                   QPointF(sx, screenRect.bottom()));

        QPolygonF arrowY;
        arrowY << QPointF(sx - arrow * 0.5, screenRect.top() + arrow)
               << QPointF(sx, screenRect.top())
               << QPointF(sx + arrow * 0.5, screenRect.top() + arrow);
        p.drawPolygon(arrowY);

        p.setPen(Qt::black);
        p.setFont(QFont("Arial", 10, QFont::Bold));
        p.drawText(QPointF(sx + 8, screenRect.top() + 18), "Y");

        p.setPen(QPen(QColor(100, 100, 100), 2));
    }

    // Подписи координат.
    p.setPen(Qt::darkGray);
    p.setFont(QFont("Arial", 9));

    double xLabelScreenY = clamp(offsetY + 16.0,
                                 screenRect.top() + 14.0,
                                 screenRect.bottom() - 4.0);

    for (double wx = startX; wx <= worldRight + gridStep * 0.5; wx += gridStep) {
        if (std::abs(wx) < gridStep * 0.25) {
            continue;
        }

        double sx = wx * scale + offsetX;
        if (sx < screenRect.left() + 5 || sx > screenRect.right() - 5) {
            continue;
        }

        p.drawText(QPointF(sx - 18, xLabelScreenY), formatCoord(wx));
    }

    double yLabelScreenX = clamp(offsetX + 6.0,
                                 screenRect.left() + 4.0,
                                 screenRect.right() - 60.0);

    for (double wy = startY; wy <= worldTop + gridStep * 0.5; wy += gridStep) {
        if (std::abs(wy) < gridStep * 0.25) {
            continue;
        }
        double sy = offsetY - wy * scale;
        if (sy < screenRect.top() + 5 || sy > screenRect.bottom() - 5) {
            continue;
        }
        p.drawText(QPointF(yLabelScreenX, sy + 4), formatCoord(wy));
    }

    if (xAxisVisible && yAxisVisible) {
        p.drawText(QPointF(offsetX + 6, offsetY - 14), "0");
    }

    p.restore();
}

class GeomTask {
public:
    virtual ~GeomTask() = default;
    virtual QString name() const = 0;
    virtual QString description() const = 0;
    
    virtual void draw(QPainter& p, const QRect& rect, const QVector<Point<Quotient_Number>>& points, 
                     int draggingIndex, int hoveredIndex, double offsetX, double offsetY, double scale) = 0;
    
    virtual void handleMousePress(QMouseEvent* event, QVector<Point<Quotient_Number>>& points, int& draggingIndex, int& hoveredIndex) = 0;
    virtual void handleMouseMove(QMouseEvent* event, QVector<Point<Quotient_Number>>& points, int& draggingIndex, int& hoveredIndex, QWidget* widget) = 0;
    virtual void handleMouseRelease(QMouseEvent* event, QVector<Point<Quotient_Number>>& points, int& draggingIndex) = 0;
    virtual void handleLeave(QVector<Point<Quotient_Number>>& points, int& hoveredIndex, QWidget* widget) = 0;
    virtual void handleKeyPress(QKeyEvent* event, QVector<Point<Quotient_Number>>& points, int& draggingIndex, int& hoveredIndex) {
        Q_UNUSED(event); Q_UNUSED(points); Q_UNUSED(draggingIndex); Q_UNUSED(hoveredIndex);
        event->ignore();
    }
    
    virtual int maxPoints() const { return -1; } // отрицательное значение = без ограничения
    virtual QStringList operationOptions() const { return QStringList(); }
    virtual int currentOperation() const { return -1; }
    virtual void setOperation(int index) { Q_UNUSED(index); }
    virtual void setPointRadius(double radius) = 0;
    virtual void setEpsilon(double epsilon) { m_epsilon = epsilon; }
    virtual void setShowEpsilonNeighborhood(bool show) = 0;
    void setViewScale(double scale) {
        if (scale > 1e-12) m_viewScale = scale;
    }


    void   setUseBigNum(bool use) { m_useBigNum = use; }
    bool   useBigNum()      const { return m_useBigNum; }
virtual int minPoints() const { return 0; }

virtual bool canAddPoint(const QVector<Point<Quotient_Number>>& points) const {
    const int limit = maxPoints();
    return limit < 0 || static_cast<int>(points.size()) < limit;
}

virtual bool canRemovePoint(const QVector<Point<Quotient_Number>>& points) const {
    return static_cast<int>(points.size()) > minPoints();
}

virtual bool canAddPointAt(const QPointF& scenePos, const QVector<Point<Quotient_Number>>& points) const {
    Q_UNUSED(scenePos);
    return canAddPoint(points);
}

    virtual bool canAddPointFromInput(const QVector<Point<Quotient_Number>>& points) const {
        return canAddPoint(points);
    }

    virtual bool addPointFromInput(const Point<Quotient_Number>& pt, QVector<Point<Quotient_Number>>& points) {
        if (!canAddPointFromInput(points)) return false;
        points.push_back(pt);
        return true;
    }

    virtual bool setPointsFromExternal(const QVector<Point<Quotient_Number>>& pts,
                                       QVector<Point<Quotient_Number>>& points) {
        const int limit = maxPoints();
        points = pts;
        if (limit >= 0 && points.size() > limit) {
            points.resize(limit);
        }
        return true;
    }



virtual Point<Quotient_Number> makePointFromScene(const QPointF& scenePos) const {
    return Point<Quotient_Number>(Quotient_Number(scenePos.x()), Quotient_Number(scenePos.y()));
}


protected:
        bool m_useBigNum = true;
        double m_epsilon = 100.0;
    double m_viewScale = 1.0;
};

class StandardPointTask : public GeomTask {
public:
    void setPointRadius(double radius) override {
        m_pointRadius = radius;
    }

    void setShowEpsilonNeighborhood(bool show) override {
        m_showEpsilonNeighborhood = show;
    }

    void handleMousePress(QMouseEvent* event,
                          QVector<Point<Quotient_Number>>& points,
                          int& draggingIndex,
                          int& hoveredIndex) override {
        const int idx = pointIndexAt(event->pos(), points);

        if (event->button() == Qt::LeftButton) {
            if (idx != -1) {
                draggingIndex = idx;
            } else if (canAddPointAt(event->pos(), points)) {
                points.push_back(makePointFromScene(event->pos()));
            }
        } else if (event->button() == Qt::RightButton) {
            if (idx != -1 && canRemovePoint(points)) {
                points.removeAt(idx);
                draggingIndex = -1;
                hoveredIndex = -1;
            }
        }
    }

    void handleMouseMove(QMouseEvent* event,
                         QVector<Point<Quotient_Number>>& points,
                         int& draggingIndex,
                         int& hoveredIndex,
                         QWidget* widget) override {
        const int idx = pointIndexAt(event->pos(), points);

        if (draggingIndex != -1 && draggingIndex < static_cast<int>(points.size())) {
            points[draggingIndex] = makePointFromScene(event->pos());
        } else if (hoveredIndex != idx) {
            hoveredIndex = idx;
            if (widget) {
                widget->setCursor(idx != -1 ? Qt::PointingHandCursor : Qt::ArrowCursor);
            }
        }
    }

    void handleMouseRelease(QMouseEvent* event,
                            QVector<Point<Quotient_Number>>& points,
                            int& draggingIndex) override {
        Q_UNUSED(points);
        if (event->button() == Qt::LeftButton) {
            draggingIndex = -1;
        }
    }

    void handleLeave(QVector<Point<Quotient_Number>>& points,
                     int& hoveredIndex,
                     QWidget* widget) override {
        Q_UNUSED(points);
        hoveredIndex = -1;
        if (widget) {
            widget->setCursor(Qt::ArrowCursor);
        }
    }

protected:
    int pointIndexAt(const QPointF& pos, const QVector<Point<Quotient_Number>>& points) const {
        for (int i = 0; i < static_cast<int>(points.size()); ++i) {
            const double dx = geom_detail::toDouble(points[i].x) - pos.x();
            const double dy = geom_detail::toDouble(points[i].y) - pos.y();
            const double viewScale = (m_viewScale > 1e-12) ? m_viewScale : 1.0;
            const double hitRadius = (m_pointRadius + 2.0) / viewScale;

            if (dx * dx + dy * dy <= hitRadius * hitRadius) {
                return i;
            }
        }

        return -1;
    }

    double m_pointRadius = 12.0;
    bool m_showEpsilonNeighborhood = false;
};
