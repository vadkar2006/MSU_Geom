#include "OrientationTask.h"
#include "Geometry.h"
#include <QPainter>
#include <QMouseEvent>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QPolygonF>
#include <cmath>


void OrientationTask::draw(QPainter& p, const QRect& rect, const QVector<Point<Quotient_Number>>& points, 
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
    p.drawText(20, 40, "Кликните для добавления точек");
    p.restore();
    return;
}
    
    const QStringList labels = {"A", "B", "C"};
    
    // Полуплоскости
    if (points.size() >= 2) {
        const Point<double> a = points[0].toDoublePoint(), b = points[1].toDoublePoint();
        double dx = b.x - a.x, dy = b.y - a.y;
        double len = std::hypot(dx, dy);
        if (len < 1e-9) len = 1e-9;
        double nx = -dy / len, ny = dx / len;
        double viewWorld = (rect.width() + rect.height()) / ((scale > 1e-12) ? scale : 1.0);
double big = viewWorld * 2.0 + len + 1.0;
if (!std::isfinite(big) || big < 1.0) big = 1.0;
        
        QPolygonF halfPlane1;
        halfPlane1 << QPointF(a.x - dx * big, a.y - dy * big) 
                   << QPointF(b.x + dx * big, b.y + dy * big)
                   << QPointF(b.x + dx * big - nx * big, b.y + dy * big - ny * big) 
                   << QPointF(a.x - dx * big - nx * big, a.y - dy * big - ny * big);
        
        QPolygonF halfPlane2;
        halfPlane2 << QPointF(a.x - dx * big, a.y - dy * big) 
                   << QPointF(b.x + dx * big, b.y + dy * big)
                   << QPointF(b.x + dx * big + nx * big, b.y + dy * big + ny * big) 
                   << QPointF(a.x - dx * big + nx * big, a.y - dy * big + ny * big);
        
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(111, 173, 241, 80));
        p.drawPolygon(halfPlane1);
        p.setBrush(QColor(144, 238, 144, 80));
        p.drawPolygon(halfPlane2);
    }
    
    // Линия через A и B
    if (points.size() >= 2) {
        const Point<double> a = points[0].toDoublePoint(), b = points[1].toDoublePoint();
        double dx = b.x - a.x, dy = b.y - a.y;
        double len = std::hypot(dx, dy);
        if (len < 1e-9) len = 1e-9;
        dx /= len; dy /= len;
        double viewWorldLine = (rect.width() + rect.height()) / ((scale > 1e-12) ? scale : 1.0);
double t = viewWorldLine * 2.0 + len + 1.0;
if (!std::isfinite(t) || t < 1.0) t = 1.0;
        p.setPen(QPen(QColor(80, 80, 80), 3.0 / scale, Qt::SolidLine));
        p.drawLine(QPointF(a.x - dx * t, a.y - dy * t), QPointF(a.x + dx * t, a.y + dy * t));
    }
    
    // Отрезок BC
    if (points.size() >= 3) {
        p.setPen(QPen(QColor(200, 50, 50), 3.0 / scale));
        p.drawLine(points[1].toQPointF(), points[2].toQPointF());
    }
    
    // Epsilon окрестность
    if (m_showEpsilonNeighborhood && points.size() >= 2) {
        const Point<double> a = points[0].toDoublePoint(), b = points[1].toDoublePoint();
        double ab_len = std::hypot(b.x - a.x, b.y - a.y);
        double radius_ab = (m_epsilon > 0.0 ? std::sqrt(m_epsilon) : 0.0);
        QPen epsPen(QColor(255, 100, 0), 2.0 / scale, Qt::DashLine);
        p.setPen(epsPen);
        p.setBrush(Qt::NoBrush);
        if (radius_ab > 0.0) {
            p.drawEllipse(QPointF(a.x, a.y), radius_ab, radius_ab);
            p.drawEllipse(QPointF(b.x, b.y), radius_ab, radius_ab);
        }
        if (points.size() >= 3 && ab_len >= 1e-9) {
            const Point<double> c = points[2].toDoublePoint();
            double epsilon_dist = m_epsilon / ab_len;
            if (epsilon_dist > 0.0) {
                p.drawEllipse(QPointF(c.x, c.y), epsilon_dist, epsilon_dist);
            }
        }
    }

    // Точки (фиксированный размер на экране)
    for (int i = 0; i < points.size(); ++i) {
        const Point<double> pt = points[i].toDoublePoint();
        QColor fillColor;
        
        if (i == 2 && points.size() >= 3) {
            int o = 0;
            if (useBigNum())
                o = predicate::orientation(points[0], points[1], points[2], Quotient_Number(0));
            else {
                QVector<Point<double>> d = toDoubleVector(points);
                o = predicate::orientation(d[0], d[1], d[2], m_epsilon);
            }
            if (o == 1) fillColor = QColor(144, 238, 144);
            else if (o == -1) fillColor = QColor(16, 77, 147);
            else fillColor = QColor(200, 200, 200);
        } else {
            fillColor = QColor(220, 80, 80);
        }
        
        p.setBrush(fillColor);
        bool isActive = (i == draggingIndex || i == hoveredIndex);
        p.setPen(isActive ? QPen(QColor(255, 180, 0), 3.0 / scale) : QPen(Qt::black, 2.0 / scale));
        p.drawEllipse(pt.toQPointF(), m_pointRadius / scale, m_pointRadius / scale);
    }
    
    p.restore();


p.save();
p.resetTransform();
p.setPen(QColor(50, 50, 50));
p.setFont(QFont("Arial", 13, QFont::Bold));
for (int i = 0; i < points.size(); ++i) {
    const Point<double> pt = points[i].toDoublePoint();
    QPointF screen(pt.x * scale + offsetX, offsetY - pt.y * scale);
    if (i < labels.size()) p.drawText(screen + QPointF(20.0, -15.0), labels[i]);
}
p.restore();
    

    if (points.size() >= 3) {
        int o = 0;
        if (useBigNum())
            o = predicate::orientation(points[0], points[1], points[2], Quotient_Number(0));
        else {
            QVector<Point<double>> d = toDoubleVector(points);
            o = predicate::orientation(d[0], d[1], d[2], m_epsilon);
        }
        
        p.setPen(Qt::black);
        p.setFont(QFont("Arial", 14, QFont::Bold));
        p.drawText(20, rect.height() - 60, 
                  QString("predicate::orientation(A, B, C) = %1").arg(predicate::orientationNameRu(o)));
        
        p.setFont(QFont("Arial", 10));
        p.setPen(Qt::darkBlue);
        p.drawText(20, rect.height() - 35, 
                  QString("A=(%1, %2)  B=(%3, %4)  C=(%5, %6)")
                      .arg(geom_detail::toDouble(points[0].x), 0, 'f', 1).arg(geom_detail::toDouble(points[0].y), 0, 'f', 1)
                      .arg(geom_detail::toDouble(points[1].x), 0, 'f', 1).arg(geom_detail::toDouble(points[1].y), 0, 'f', 1)
                      .arg(geom_detail::toDouble(points[2].x), 0, 'f', 1).arg(geom_detail::toDouble(points[2].y), 0, 'f', 1));
        
        p.setFont(QFont("Arial", 9));
        p.setPen(QColor(50, 120, 50));
        p.drawText(20, rect.height() - 100, "Зелёная область: против часовой стрелки");
        p.setPen(QColor(50, 50, 120));
        p.drawText(20, rect.height() - 85, "Синяя область: по часовой стрелке");
    }
}

