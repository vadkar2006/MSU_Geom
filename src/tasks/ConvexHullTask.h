#pragma once
// ConvexHullTask-v1
#include "Task.h"
#include "Quotient_Number.h"

class ConvexHullTask : public StandardPointTask {
public:
    QString name() const override { return QStringLiteral("Выпуклая оболочка"); }
    QString description() const override { return QStringLiteral("Построение выпуклой оболочки для набора точек"); }

    void draw(QPainter& p, const QRect& rect, const QVector<Point<Quotient_Number>>& points,
              int draggingIndex, int hoveredIndex, double offsetX, double offsetY, double scale) override;
    int maxPoints() const override { return -1; }
    int minPoints() const override { return 0; }
    void handleMouseRelease(QMouseEvent* event, QVector<Point<Quotient_Number>>& points, int& draggingIndex) override;

private:
    QVector<Point<Quotient_Number>> m_ghostHull;
    void updateGhostHull(const QVector<Point<Quotient_Number>>& points);
    static QVector<int> hullIndices(const QVector<Point<Quotient_Number>>& points, bool exact, double epsilon);
};
