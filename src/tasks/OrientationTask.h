#pragma once
#include "Task.h"

class OrientationTask : public Task {
public:
    QString name() const override { return "Предикат поворота"; }
    QString description() const override { return "Определение ориентации трёх точек"; }
    
    void draw(QPainter& p, const QRect& rect, const QVector<Point>& points, 
             int draggingIndex, int hoveredIndex, double offsetX, double offsetY, double scale) override;
    
    void handleMousePress(QMouseEvent* event, QVector<Point>& points, int& draggingIndex, int& hoveredIndex) override;
    void handleMouseMove(QMouseEvent* event, QVector<Point>& points, int& draggingIndex, int& hoveredIndex, QWidget* widget) override;
    void handleMouseRelease(QMouseEvent* event, QVector<Point>& points, int& draggingIndex) override;
    void handleLeave(QVector<Point>& points, int& hoveredIndex, QWidget* widget) override;
    
    int maxPoints() const override { return 3; }
    void setPointRadius(double radius) override { m_pointRadius = radius; }
    void setEpsilon(double epsilon) override { m_epsilon = epsilon; }
    void setShowEpsilonNeighborhood(bool show) override { m_showEpsilonNeighborhood = show; }

private:
    double m_pointRadius = 12.0;
    double m_epsilon = 100.0;
    bool m_showEpsilonNeighborhood = false;
    
    int getPointIndexAt(const QPointF& pos, const QVector<Point>& points) const;
};
