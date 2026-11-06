#pragma once
// DelaunayTask-v1

#include "Task.h"
#include "Quotient_Number.h"

class DelaunayTask : public StandardPointTask {
public:
    QString name() const override {
        return QStringLiteral("Триангуляция Делоне");
    }

    QString description() const override {
        return QStringLiteral("Построение триангуляции Делоне для набора точек");
    }

    void draw(QPainter& p, const QRect& rect, const QVector<Point<Quotient_Number>>& points,
              int draggingIndex, int hoveredIndex, double offsetX, double offsetY, double scale) override;

    int maxPoints() const override { return -1; }
    int minPoints() const override { return 0; }
};
