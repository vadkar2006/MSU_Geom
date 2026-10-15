#pragma once
#include <QPainter>
#include <QRect>
#include <QVector>
#include <QMouseEvent>
#include <QWidget>
#include <QString>
#include "Point.h"

class Task {
public:
    virtual ~Task() = default;
    virtual QString name() const = 0;
    virtual QString description() const = 0;
    
    virtual void draw(QPainter& p, const QRect& rect, const QVector<Point>& points, 
                     int draggingIndex, int hoveredIndex, double offsetX, double offsetY, double scale) = 0;
    
    virtual void handleMousePress(QMouseEvent* event, QVector<Point>& points, int& draggingIndex, int& hoveredIndex) = 0;
    virtual void handleMouseMove(QMouseEvent* event, QVector<Point>& points, int& draggingIndex, int& hoveredIndex, QWidget* widget) = 0;
    virtual void handleMouseRelease(QMouseEvent* event, QVector<Point>& points, int& draggingIndex) = 0;
    virtual void handleLeave(QVector<Point>& points, int& hoveredIndex, QWidget* widget) = 0;
    
    virtual int maxPoints() const = 0;
    virtual void setPointRadius(double radius) = 0;
    virtual void setEpsilon(double epsilon) = 0;
    virtual void setShowEpsilonNeighborhood(bool show) = 0;
};
