#pragma once
#include <QPointF>


struct Point {
    double x = 0.0;
    double y = 0.0;

    Point() = default;
    Point(double x_, double y_) : x(x_), y(y_) {}
    Point(const QPointF& p) : x(p.x()), y(p.y()) {}

    QPointF toQPointF() const { return QPointF(x, y); }
};
