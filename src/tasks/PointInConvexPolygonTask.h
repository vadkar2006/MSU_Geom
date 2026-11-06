#pragma once
// PointInConvexPolygonTask-v1
#include "Task.h"
#include "Quotient_Number.h"


class PointInConvexPolygonTask : public StandardPointTask {
public:
    QString name() const override {
        return QStringLiteral("Точка и выпуклый многоугольник");
    }
    QString description() const override {
        return QStringLiteral("Клики - вершины, двойной щелчок строит выпуклую оболочку, следующий клик ставит точку P");
    }

    void draw(QPainter& p, const QRect& rect, const QVector<Point<Quotient_Number>>& points,
              int draggingIndex, int hoveredIndex, double offsetX, double offsetY, double scale) override;

    void handleMousePress(QMouseEvent* event, QVector<Point<Quotient_Number>>& points,
                          int& draggingIndex, int& hoveredIndex) override;
    int maxPoints() const override { return -1; }
    int minPoints() const override { return 0; }

    bool canAddPointFromInput(const QVector<Point<Quotient_Number>>& points) const override {
        if (m_polygonCount >= 3) return !hasQueryPoint(points);
        return StandardPointTask::canAddPoint(points);
    }

    bool addPointFromInput(const Point<Quotient_Number>& pt, QVector<Point<Quotient_Number>>& points) override {
        if (!canAddPointFromInput(points)) return false;
        points.push_back(pt);
        m_lastPressAddedPoint = false;
        return true;
    }


private:
    void sanitizeState(const QVector<Point<Quotient_Number>>& points);


    void buildConvexHull(QVector<Point<Quotient_Number>>& points);

    // 1 - внутри, -1 - снаружи, 0 - на границе (или точка P ещё не поставлена).
    int pointStatus(const QVector<Point<Quotient_Number>>& points) const;

    bool hasQueryPoint(const QVector<Point<Quotient_Number>>& points) const {
        return m_polygonCount >= 3 && points.size() == m_polygonCount + 1;
    }

    int m_polygonCount = -1;
    bool m_lastPressAddedPoint = false;
    QVector<Point<Quotient_Number>> m_savedPolygon;
    void updateSavedPolygon(const QVector<Point<Quotient_Number>>& points);

    bool setPointsFromExternal(const QVector<Point<Quotient_Number>>& pts,
                               QVector<Point<Quotient_Number>>& points) override {
        const int oldCount = m_polygonCount;
        const bool oldHadP = hasQueryPoint(points);

        points = pts;
        m_lastPressAddedPoint = false;

        if (points.size() < 3) {
            m_polygonCount = -1;
            m_savedPolygon.clear();
            return true;
        }

        if (oldCount >= 3) {
            if (points.size() == oldCount ||
                (oldHadP && points.size() == oldCount + 1)) {
                m_polygonCount = oldCount;
                updateSavedPolygon(points);
                return true;
            }
        }

        m_polygonCount = -1;
        m_savedPolygon.clear();
        return true;
    }

};
