#include "OrientationTask.h"
#include "Geometry.h"
#include <QPainter>
#include <QMouseEvent>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QPolygonF>
#include <cmath>

int OrientationTask::getPointIndexAt(const QPointF& pos, const QVector<Point>& points) const {
    for (int i = 0; i < points.size(); ++i) {
        double dx = points[i].x - pos.x();
        double dy = points[i].y - pos.y();
        if (dx * dx + dy * dy <= m_pointRadius * m_pointRadius) return i;
    }
    return -1;
}

void OrientationTask::draw(QPainter& p, const QRect& rect, const QVector<Point>& points, 
                           int draggingIndex, int hoveredIndex, double offsetX, double offsetY, double scale) {
    // rect - видимая область в мировых координатах
    
    // === СЕТКА (рисуется в экранных координатах) ===
    p.save();
    p.resetTransform();
    
    int screenWidth = p.device()->width();
    int screenHeight = p.device()->height();
    
    // Адаптивный шаг сетки
    double targetScreenStep = 80.0;
    double worldStep = targetScreenStep / scale;
    
    double magnitude = std::pow(10.0, std::floor(std::log10(worldStep)));
    double normalized = worldStep / magnitude;
    
    double gridStep;
    if (normalized < 1.5) gridStep = 1.0 * magnitude;
    else if (normalized < 3.5) gridStep = 2.0 * magnitude;
    else if (normalized < 7.5) gridStep = 5.0 * magnitude;
    else gridStep = 10.0 * magnitude;
    
    // Вертикальные линии
    p.setPen(QPen(QColor(220, 220, 220), 1.0));
    double startX = std::floor(rect.left() / gridStep) * gridStep;
    for (double worldX = startX; worldX <= rect.right(); worldX += gridStep) {
        double screenX = worldX * scale + offsetX;
        p.drawLine(QPointF(screenX, 0), QPointF(screenX, screenHeight));
    }
    
    // Горизонтальные линии
    double startY = std::floor(rect.top() / gridStep) * gridStep;
    for (double worldY = startY; worldY <= rect.bottom(); worldY += gridStep) {
        double screenY = worldY * scale + offsetY;
        p.drawLine(QPointF(0, screenY), QPointF(screenWidth, screenY));
    }
    
    // Оси координат
    p.setPen(QPen(QColor(100, 100, 100), 2.0));
    
    // Ось X (y=0)
    if (rect.top() <= 0 && rect.bottom() >= 0) {
        double screenY = 0.0 * scale + offsetY;
        p.drawLine(QPointF(0, screenY), QPointF(screenWidth, screenY));
        
        // Стрелка
        double arrowSize = 10.0;
        QPolygonF arrowX;
        arrowX << QPointF(screenWidth - arrowSize, screenY - arrowSize/2) 
               << QPointF(screenWidth, screenY) 
               << QPointF(screenWidth - arrowSize, screenY + arrowSize/2);
        p.setBrush(QColor(100, 100, 100));
        p.drawPolygon(arrowX);
        
        // Подпись
        p.setPen(Qt::black);
        p.setFont(QFont("Arial", 12, QFont::Bold));
        p.drawText(screenWidth - 20, screenY - 10, "X");
    }
    
    // Ось Y (x=0)
    if (rect.left() <= 0 && rect.right() >= 0) {
        double screenX = 0.0 * scale + offsetX;
        p.setPen(QPen(QColor(100, 100, 100), 2.0));
        p.drawLine(QPointF(screenX, 0), QPointF(screenX, screenHeight));
        
        // Стрелка
        double arrowSize = 10.0;
        QPolygonF arrowY;
        arrowY << QPointF(screenX - arrowSize/2, arrowSize) 
               << QPointF(screenX, 0) 
               << QPointF(screenX + arrowSize/2, arrowSize);
        p.setBrush(QColor(100, 100, 100));
        p.drawPolygon(arrowY);
        
        // Подпись
        p.setPen(Qt::black);
        p.setFont(QFont("Arial", 12, QFont::Bold));
        p.drawText(screenX + 10, 15, "Y");
    }
    
    // Числовые метки
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
    
    // === ГЕОМЕТРИЯ (рисуется с трансформацией) ===
    p.save();
    p.translate(offsetX, offsetY);
    p.scale(scale, scale);
    
    p.fillRect(rect, QColor(250, 250, 250));
    
    if (points.isEmpty()) {
        p.setPen(Qt::darkGray);
        p.setFont(QFont("Arial", 12.0 / scale));
        p.drawText(20.0/scale, 40.0/scale, "Кликните для добавления точек");
        p.restore();
        return;
    }
    
    const QStringList labels = {"A", "B", "C"};
    
    // Полуплоскости
    if (points.size() >= 2) {
        const Point& a = points[0], &b = points[1];
        double dx = b.x - a.x, dy = b.y - a.y;
        double len = std::hypot(dx, dy);
        if (len < 1e-9) len = 1e-9;
        double nx = -dy / len, ny = dx / len;
        const double big = 1e5;
        
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
        p.setBrush(QColor(144, 238, 144, 80));
        p.drawPolygon(halfPlane1);
        p.setBrush(QColor(111, 173, 241, 80));
        p.drawPolygon(halfPlane2);
    }
    
    // Линия через A и B
    if (points.size() >= 2) {
        const Point& a = points[0], &b = points[1];
        double dx = b.x - a.x, dy = b.y - a.y;
        double len = std::hypot(dx, dy);
        if (len < 1e-9) len = 1e-9;
        dx /= len; dy /= len;
        const double t = 1e5;
        p.setPen(QPen(QColor(80, 80, 80), 3.0 / scale, Qt::SolidLine));
        p.drawLine(QPointF(a.x - dx * t, a.y - dy * t), QPointF(a.x + dx * t, a.y + dy * t));
    }
    
    // Отрезок BC
    if (points.size() >= 3) {
        p.setPen(QPen(QColor(200, 50, 50), 3.0 / scale));
        p.drawLine(points[1].toQPointF(), points[2].toQPointF());
    }
    
    // Epsilon окрестность
    if (m_showEpsilonNeighborhood && points.size() >= 3) {
        const Point& a = points[0], &b = points[1], &c = points[2];
        double ab_len = std::hypot(b.x - a.x, b.y - a.y);
        if (ab_len >= 1e-9) {
            double epsilon_dist = m_epsilon / ab_len;
            double radius_ab = std::sqrt(m_epsilon);
            
            QPen epsPen(QColor(255, 100, 0), 2.0 / scale, Qt::DashLine);
            p.setPen(epsPen);
            p.setBrush(Qt::NoBrush);
            p.drawEllipse(QPointF(c.x, c.y), epsilon_dist, epsilon_dist);
            p.drawEllipse(QPointF(a.x, a.y), radius_ab, radius_ab);
            p.drawEllipse(QPointF(b.x, b.y), radius_ab, radius_ab);
        }
    }
    
    // Точки (фиксированный размер на экране)
    for (int i = 0; i < points.size(); ++i) {
        const Point& pt = points[i];
        QColor fillColor;
        
        if (i == 2 && points.size() >= 3) {
            int o = predicate::orientation(points[0], points[1], points[2], m_epsilon);
            if (o == 1) fillColor = QColor(16, 77, 147);
            else if (o == -1) fillColor = QColor(144, 238, 144);
            else fillColor = QColor(200, 200, 200);
        } else {
            fillColor = QColor(220, 80, 80);
        }
        
        p.setBrush(fillColor);
        bool isActive = (i == draggingIndex || i == hoveredIndex);
        p.setPen(isActive ? QPen(QColor(255, 180, 0), 3.0 / scale) : QPen(Qt::black, 2.0 / scale));
        p.drawEllipse(pt.toQPointF(), m_pointRadius / scale, m_pointRadius / scale);
    }
    
    // Подписи точек
    for (int i = 0; i < points.size(); ++i) {
        const Point& pt = points[i];
        p.setPen(Qt::black);
        p.setFont(QFont("Arial", 12.0 / scale, QFont::Bold));
        p.drawText(pt.toQPointF() + QPointF(-6.0/scale, 5.0/scale), labels[i]);
        
        p.setPen(QColor(50, 50, 50));
        p.setFont(QFont("Arial", 13.0 / scale, QFont::Bold));
        p.drawText(pt.toQPointF() + QPointF(20.0/scale, -15.0/scale), labels[i]);
    }
    
    p.restore();
    
    // === ИНФОРМАЦИОННЫЙ ТЕКСТ (в экранных координатах) ===
    if (points.size() >= 3) {
        int o = predicate::orientation(points[0], points[1], points[2], m_epsilon);
        
        p.setPen(Qt::black);
        p.setFont(QFont("Arial", 14, QFont::Bold));
        p.drawText(20, rect.height() - 60, 
                  QString("predicate::orientation(A, B, C) = %1").arg(predicate::orientationNameRu(o)));
        
        p.setFont(QFont("Arial", 10));
        p.setPen(Qt::darkBlue);
        p.drawText(20, rect.height() - 35, 
                  QString("A=(%1, %2)  B=(%3, %4)  C=(%5, %6)")
                      .arg(points[0].x, 0, 'f', 1).arg(points[0].y, 0, 'f', 1)
                      .arg(points[1].x, 0, 'f', 1).arg(points[1].y, 0, 'f', 1)
                      .arg(points[2].x, 0, 'f', 1).arg(points[2].y, 0, 'f', 1));
        
        p.setFont(QFont("Arial", 9));
        p.setPen(QColor(50, 120, 50));
        p.drawText(20, rect.height() - 100, "Зелёная область: против часовой стрелки");
        p.setPen(QColor(50, 50, 120));
        p.drawText(20, rect.height() - 85, "Синяя область: по часовой стрелке");
    }
}

void OrientationTask::handleMousePress(QMouseEvent* event, QVector<Point>& points, int& draggingIndex, int& hoveredIndex) {
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

void OrientationTask::handleMouseMove(QMouseEvent* event, QVector<Point>& points, int& draggingIndex, int& hoveredIndex, QWidget* widget) {
    int idx = getPointIndexAt(event->pos(), points);
    
    if (draggingIndex != -1) {
        points[draggingIndex] = Point(event->pos());
    } else if (hoveredIndex != idx) {
        hoveredIndex = idx;
        widget->setCursor(idx != -1 ? Qt::PointingHandCursor : Qt::ArrowCursor);
    }
}

void OrientationTask::handleMouseRelease(QMouseEvent* event, QVector<Point>& points, int& draggingIndex) {
    if (event->button() == Qt::LeftButton) {
        draggingIndex = -1;
    }
}

void OrientationTask::handleLeave(QVector<Point>& points, int& hoveredIndex, QWidget* widget) {
    hoveredIndex = -1;
    widget->setCursor(Qt::ArrowCursor);
}
