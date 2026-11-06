#include "SegmentIntersectionTask.h"
#include "Geometry.h"
#include <QPainter>
#include <QMouseEvent>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QStringList>
#include <cmath>


void SegmentIntersectionTask::draw(QPainter& p, const QRect& rect, const QVector<Point<Quotient_Number>>& points,
int draggingIndex, int hoveredIndex, double offsetX, double offsetY, double scale) {

    drawCoordinateGrid(p, rect, offsetX, offsetY, scale);


    p.save();
    p.translate(offsetX, offsetY);
    p.scale(scale, -scale); // Y направлен вверх

    if (points.isEmpty()) {
        p.restore();
        p.save();
        p.resetTransform();
        p.setPen(Qt::darkGray);
        p.setFont(QFont("Arial", 12));
        p.drawText(20, 40, "Кликните для добавления точек (нужно 4 точки)");
        p.restore();
        return;
    }

    const QStringList labels = {"A", "B", "C", "D"};


    if (points.size() >= 2) {
        p.setPen(QPen(QColor(220, 80, 80), 3.0 / scale));
        p.drawLine(points[0].toQPointF(), points[1].toQPointF());
    }
    if (points.size() >= 4) {
        p.setPen(QPen(QColor(80, 80, 220), 3.0 / scale));
        p.drawLine(points[2].toQPointF(), points[3].toQPointF());
    }

    if (m_showEpsilonNeighborhood && !points.isEmpty()) {
        QPen epsPen(QColor(255, 100, 0), 2.0 / scale, Qt::DashLine);
        p.setPen(epsPen);
        p.setBrush(Qt::NoBrush);
        double r = (m_epsilon > 0.0 ? std::sqrt(m_epsilon) : 0.0);
        if (r > 0.0) {
            for (const auto& pt : points) {
                p.drawEllipse(pt.toQPointF(), r, r);
            }
        }
    }

    for (int i = 0; i < points.size(); ++i) {
        const Point<double> pt = points[i].toDoublePoint();
        QColor fillColor = (i < 2) ? QColor(220, 80, 80) : QColor(80, 80, 220);
        p.setBrush(fillColor);
        bool isActive = (i == draggingIndex || i == hoveredIndex);
        p.setPen(isActive ? QPen(QColor(255, 180, 0), 3.0 / scale) : QPen(Qt::black, 2.0 / scale));
        p.drawEllipse(pt.toQPointF(), m_pointRadius / scale, m_pointRadius / scale);
    }
    p.restore();


    p.save();
    p.resetTransform();
    p.setPen(Qt::black);
    p.setFont(QFont("Arial", 12, QFont::Bold));
    for (int i = 0; i < points.size(); ++i) {
        const Point<double> pt = points[i].toDoublePoint();
        QPointF screen(pt.x * scale + offsetX, offsetY - pt.y * scale);
        if (i < labels.size()) p.drawText(screen + QPointF(20.0, -15.0), labels[i]);
    }
    p.restore();


    if (points.size() >= 4) {
        int result = 0;
        if (useBigNum())
            result = segment_intersection::orientation_segment(points[0], points[1], points[2], points[3], Quotient_Number(0));
        else {
            QVector<Point<double>> d = toDoubleVector(points);
            result = segment_intersection::orientation_segment(d[0], d[1], d[2], d[3], m_epsilon);
        }

        QString statusText;
        QColor statusColor;

        auto drawMarker = [&](const Point<Quotient_Number>& ip) {
            const Point<double> ipt = ip.toDoublePoint();
            const QPointF screen(ipt.x * scale + offsetX, offsetY - ipt.y * scale);
            p.save();
            p.resetTransform();
            p.setBrush(QColor(255, 200, 0));
            p.setPen(QPen(Qt::black, 2));
            p.drawEllipse(screen, 8.0, 8.0);
            p.setPen(Qt::black);
            p.setFont(QFont("Arial", 10));
            p.drawText(screen + QPointF(15.0, -15.0),
                       QString("(%1, %2)")
                           .arg(geom_detail::toDouble(ip.x), 0, 'f', 1)
                           .arg(geom_detail::toDouble(ip.y), 0, 'f', 1));
            p.restore();
        };

        if (result == 0) {
            statusText = "Отрезки пересекаются";
            statusColor = QColor(0, 150, 0);
            bool markerDrawn = false;
            try {
                Point<Quotient_Number> intersection = segment_intersection::intersection_line(points[0], points[1], points[2], points[3], Quotient_Number(0));
                drawMarker(intersection);
                markerDrawn = true;
            } catch (...) {
                Point<Quotient_Number> ip(Quotient_Number(0), Quotient_Number(0));
                if (segment_intersection::on_segment(points[0], points[1], points[2], Quotient_Number(0))) {
                    ip = points[2];
                    markerDrawn = true;
                } else if (segment_intersection::on_segment(points[0], points[1], points[3], Quotient_Number(0))) {
                    ip = points[3];
                    markerDrawn = true;
                } else if (segment_intersection::on_segment(points[2], points[3], points[0], Quotient_Number(0))) {
                    ip = points[0];
                    markerDrawn = true;
                } else if (segment_intersection::on_segment(points[2], points[3], points[1], Quotient_Number(0))) {
                    ip = points[1];
                    markerDrawn = true;
                }

                if (markerDrawn) {
                    drawMarker(ip);
                } else {
                    statusText += " (без единственной точки пересечения)";
                }
            }
        } else if (result == 2) {
            statusText = "Отрезки коллинеарны и пересекаются (наложение)";
            statusColor = QColor(200, 100, 0);
        } else if (result == 1) {
            statusText = "Отрезки коллинеарны, но не пересекаются";
            statusColor = QColor(150, 0, 0);
        } else {
            statusText = "Отрезки не пересекаются";
            statusColor = QColor(150, 0, 0);
        }

        p.setPen(statusColor);
        p.setFont(QFont("Arial", 14, QFont::Bold));
        p.drawText(20, rect.height() - 60, statusText);

        p.setPen(Qt::darkBlue);
        p.setFont(QFont("Arial", 10));
        p.drawText(20, rect.height() - 35,
                   QString("A=(%1, %2)  B=(%3, %4)  C=(%5, %6)  D=(%7, %8)")
                       .arg(geom_detail::toDouble(points[0].x), 0, 'f', 1).arg(geom_detail::toDouble(points[0].y), 0, 'f', 1)
                       .arg(geom_detail::toDouble(points[1].x), 0, 'f', 1).arg(geom_detail::toDouble(points[1].y), 0, 'f', 1)
                       .arg(geom_detail::toDouble(points[2].x), 0, 'f', 1).arg(geom_detail::toDouble(points[2].y), 0, 'f', 1)
                       .arg(geom_detail::toDouble(points[3].x), 0, 'f', 1).arg(geom_detail::toDouble(points[3].y), 0, 'f', 1));
    }
}
