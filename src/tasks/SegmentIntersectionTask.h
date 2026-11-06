#pragma once
#include "Task.h"
#include "Quotient_Number.h"

class SegmentIntersectionTask : public StandardPointTask {
public:
    QString name() const override { return "Пересечение отрезков"; }
    QString description() const override { return "Определение пересечения двух отрезков"; }

    void draw(QPainter& p, const QRect& rect, const QVector<Point<Quotient_Number>>& points,
              int draggingIndex, int hoveredIndex, double offsetX, double offsetY, double scale) override;
    int maxPoints() const override { return 4; }
};

