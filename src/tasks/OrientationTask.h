#pragma once
#include "Task.h"
#include "Quotient_Number.h"

class OrientationTask : public StandardPointTask {
public:
    QString name() const override { return "Предикат поворота"; }
    QString description() const override { return "Определение ориентации трёх точек"; }

    void draw(QPainter& p, const QRect& rect, const QVector<Point<Quotient_Number>>& points,
              int draggingIndex, int hoveredIndex, double offsetX, double offsetY, double scale) override;
    int maxPoints() const override { return 3; }
};

