#include "SegmentIntersectionTask.h"
#include "Geometry.h"
#include <QPainter>
#include <QMouseEvent>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <cmath>

int SegmentIntersectionTask::getPointIndexAt(const QPointF& pos, const QVector<Point>& points) const {
    for (int i = 0; i < points.size(); ++i) {
        double dx = points[i].x - pos.x();
        double dy = points[i].y - pos.y();
        if (dx * dx + dy * dy <= m_pointRadius * m_pointRadius) return i;
    }
    return -1;
}

void SegmentIntersectionTask::draw(QPainter& p, const QRect& rect, const QVector<Point>& points, 
                                   int draggingIndex, int hoveredIndex, double offsetX, double offsetY, double scale) {
    // СЕТКА (в экранных координатах)
    p.save();
    p.resetTransform();
    
    int screenWidth = p.device()->width();
    int screenHeight = p.device()->height();
    
    double targetScreenStep = 80.0;
    double worldStep = targetScreenStep / scale;
    
    double magnitude = std::pow(10.0, std::floor(std::log10(worldStep)));
    double normalized = worldStep / magnitude;
    
    double gridStep;
    if (normalized < 1.5) gridStep = 1.0 * magnitude;
    else if (normalized < 3.5) gridStep = 2.0 * magnitude;
    else if (normalized < 7.5) gridStep = 5.0 * magnitude;
    else gridStep = 10.0 * magnitude;
    
    p.setPen(QPen(QColor(220, 220, 220), 1.0));
    double startX = std::floor(rect.left() / gridStep) * gridStep;
    for (double worldX = startX; worldX <= rect.right(); worldX += gridStep) {
        double screenX = worldX * scale + offsetX;
        p.drawLine(QPointF(screenX, 0), QPointF(screenX, screenHeight));
    }
    
    double startY = std::floor(rect.top() / gridStep) * gridStep;
    for (double worldY = startY; worldY <= rect.bottom(); worldY += gridStep) {
        double screenY = worldY * scale + offsetY;
        p.drawLine(QPointF(0, screenY), QPointF(screenWidth, screenY));
    }
    
    p.setPen(QPen(QColor(100, 100, 100), 2.0));
    
    if (rect.top() <= 0 && rect.bottom() >= 0) {
        double screenY = 0.0 * scale + offsetY;
        p.drawLine(QPointF(0, screenY), QPointF(screenWidth, screenY));
        
        double arrowSize = 10.0;
        QPolygonF arrowX;
        arrowX << QPointF(screenWidth - arrowSize, screenY - arrowSize/2) 
               << QPointF(screenWidth, screenY) 
               << QPointF(screenWidth - arrowSize, screenY + arrowSize/2);
        p.setBrush(QColor(100, 100, 100));
        p.drawPolygon(arrowX);
        
        p.setPen(Qt::black);
        p.setFont(QFont("Arial", 12, QFont::Bold));
        p.drawText(screenWidth - 20, screenY - 10, "X");
    }
    
    if (rect.left() <= 0 && rect.right() >= 0) {
        double screenX = 0.0 * scale + offsetX;
        p.setPen(QPen(QColor(100, 100, 100), 2.0));
        p.drawLine(QPointF(screenX, 0), QPointF(screenX, screenHeight));
        
        double arrowSize = 10.0;
        QPolygonF arrowY;
        arrowY << QPointF(screenX - arrowSize/2, arrowSize) 
               << QPointF(screenX, 0) 
               << QPointF(screenX + arrowSize/2, arrowSize);
        p.setBrush(QColor(100, 100, 100));
        p.drawPolygon(arrowY);
        
        p.setPen(Qt::black);
        p.setFont(QFont("Arial", 12, QFont::Bold));
        p.drawText(screenX + 10, 15, "Y");
    }
    
    p.setFont(QFont("Arial", 9));
    p.setPen(Qt::darkGray);
    
    if (rect.top() <= 0 && rect.bottom() >= 0) {
        double screenY = 0.0 * scale + offsetY;
        for (double worldX = startX; worldX <= rect.right(); worldX += gridStep) {
            if (std::abs(worldX) < gridStep * 0.1) continue;
            double screenX = worldX * scale + offsetX;
            p.drawText(QPointF(screenX - 15, screenY + 15), QString::number(worldX, 'f', 0));
        }
    }
    
    if (rect.left() <= 0 && rect.right() >= 0) {
        double screenX = 0.0 * scale + offsetX;
        for (double worldY = startY; worldY <= rect.bottom(); worldY += gridStep) {
            if (std::abs(worldY) < gridStep * 0.1) continue;
            double screenY = worldY * scale + offsetY;
            p.drawText(QPointF(screenX + 5, screenY + 4), QString::number(-worldY, 'f', 0));
        }
    }
    
    p.restore();
    
    // ГЕОМЕТРИЯ (с трансформацией)
    p.save();
    p.translate(offsetX, offsetY);
    p.scale(scale, scale);
    
    p.fillRect(rect, QColor(250, 250, 250));
    
    if (points.isEmpty()) {
        p.setPen(Qt::darkGray);
        p.setFont(QFont("Arial", 12.0 / scale));
        p.drawText(20.0/scale, 40.0/scale, "Кликните для добавления точек (нужно 4 точки)");
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
    
    for (int i = 0; i < points.size(); ++i) {
        const Point& pt = points[i];
        QColor fillColor = (i < 2) ? QColor(220, 80, 80) : QColor(80, 80, 220);
        
        p.setBrush(fillColor);
        bool isActive = (i == draggingIndex || i == hoveredIndex);
        p.setPen(isActive ? QPen(QColor(255, 180, 0), 3.0 / scale) : QPen(Qt::black, 2.0 / scale));
        p.drawEllipse(pt.toQPointF(), m_pointRadius / scale, m_pointRadius / scale);
        
        p.setPen(Qt::black);
        p.setFont(QFont("Arial", 12.0 / scale, QFont::Bold));
        p.drawText(pt.toQPointF() + QPointF(20.0/scale, -15.0/scale), labels[i]);
    }
    
    p.restore();
    
    // ИНФОРМАЦИОННЫЙ ТЕКСТ (в экранных координатах)
    if (points.size() >= 4) {
        int result = segment_intersection::orientation_segment(points[0], points[1], points[2], points[3], m_epsilon);
        
        QString statusText;
        QColor statusColor;
        
        if (result == 0) {
            statusText = "Отрезки пересекаются";
            statusColor = QColor(0, 150, 0);
            
            try {
                Point intersection = segment_intersection::intersection_line(points[0], points[1], points[2], points[3], m_epsilon);
                
                p.save();
                p.translate(offsetX, offsetY);
                p.scale(scale, scale);
                
                p.setBrush(QColor(255, 200, 0));
                p.setPen(QPen(Qt::black, 2.0 / scale));
                p.drawEllipse(intersection.toQPointF(), 8.0 / scale, 8.0 / scale);
                
                p.setPen(Qt::black);
                p.setFont(QFont("Arial", 10.0 / scale));
                p.drawText(intersection.toQPointF() + QPointF(15.0/scale, -15.0/scale),
                          QString("(%1, %2)").arg(intersection.x, 0, 'f', 1).arg(intersection.y, 0, 'f', 1));
                
                p.restore();
            } catch (...) {
                statusText += " (прямые параллельны)";
            }
        } else if (result == 1) {
            statusText = "Отрезки коллинеарны и пересекаются";
            statusColor = QColor(200, 100, 0);
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
                      .arg(points[0].x, 0, 'f', 1).arg(points[0].y, 0, 'f', 1)
                      .arg(points[1].x, 0, 'f', 1).arg(points[1].y, 0, 'f', 1)
                      .arg(points[2].x, 0, 'f', 1).arg(points[2].y, 0, 'f', 1)
                      .arg(points[3].x, 0, 'f', 1).arg(points[3].y, 0, 'f', 1));
    }
}

void SegmentIntersectionTask::handleMousePress(QMouseEvent* event, QVector<Point>& points, int& draggingIndex, int& hoveredIndex) {
    int idx = getPointIndexAt(event->pos(), points);
    
    if (event->button() == Qt::LeftButton) {
        if (idx != -1) {
            draggingIndex = idx;
        } else if (points.size() < maxPoints()) {
            points.push_back(Point(event->pos()));
        }
    } else if (event->button() == Qt::RightButton) {
        if (idx != -1) {
            points.removeAt(idx);
            draggingIndex = -1;
            hoveredIndex = -1;
        }
    }
}

void SegmentIntersectionTask::handleMouseMove(QMouseEvent* event, QVector<Point>& points, int& draggingIndex, int& hoveredIndex, QWidget* widget) {
    int idx = getPointIndexAt(event->pos(), points);
    
    if (draggingIndex != -1) {
        points[draggingIndex] = Point(event->pos());
    } else if (hoveredIndex != idx) {
        hoveredIndex = idx;
        widget->setCursor(idx != -1 ? Qt::PointingHandCursor : Qt::ArrowCursor);
    }
}

void SegmentIntersectionTask::handleMouseRelease(QMouseEvent* event, QVector<Point>& points, int& draggingIndex) {
    if (event->button() == Qt::LeftButton) {
        draggingIndex = -1;
    }
}

void SegmentIntersectionTask::handleLeave(QVector<Point>& points, int& hoveredIndex, QWidget* widget) {
    hoveredIndex = -1;
    widget->setCursor(Qt::ArrowCursor);
}
