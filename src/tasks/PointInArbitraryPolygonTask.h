#pragma once

#include "Task.h"
#include "Quotient_Number.h"


class PointInArbitraryPolygonTask : public StandardPointTask {
public:
    QString name() const override {
        return QStringLiteral("Точка и произвольный многоугольник");
    }
    QString description() const override {
        return QStringLiteral("Клик - вершина контура по порядку; двойной клик - замкнуть контур (дырку); "
                              "Enter или кнопка - завершить многоугольник; затем клик ставит точку P. "
                              "Backspace - отменить вершину; ПКМ - удалить вершину или P.");
    }
    void draw(QPainter& p, const QRect& rect, const QVector<Point<Quotient_Number>>& points,
              int draggingIndex, int hoveredIndex, double offsetX, double offsetY, double scale) override;
    void handleMousePress(QMouseEvent* event, QVector<Point<Quotient_Number>>& points,
                          int& draggingIndex, int& hoveredIndex) override;
    void handleMouseMove(QMouseEvent* event, QVector<Point<Quotient_Number>>& points,
                         int& draggingIndex, int& hoveredIndex, QWidget* widget) override;
    void handleKeyPress(QKeyEvent* event, QVector<Point<Quotient_Number>>& points,
                        int& draggingIndex, int& hoveredIndex) override;
    int maxPoints() const override { return -1; }
    int minPoints() const override { return 0; }
private:
    enum class Phase { Input, Done };
    Phase m_phase = Phase::Input;
    QVector<int> m_closed;
    int m_open = 0;
    bool m_hasP = false;
    bool m_lastPressAddedPoint = false;




    bool setPointsFromExternal(const QVector<Point<Quotient_Number>>& pts,
                               QVector<Point<Quotient_Number>>& points) override {
        const int oldPoly = totalPoly();
        const bool oldHasP = m_hasP;
        const int expected = oldPoly + (oldHasP ? 1 : 0);


        if (expected == pts.size()) {
            points = pts;
            m_lastPressAddedPoint = false;
            return true;
        }

        if (m_phase == Phase::Done) {

            if (!oldHasP && pts.size() == oldPoly + 1) {
                points = pts;
                m_hasP = true;
                m_lastPressAddedPoint = false;
                return true;
            }


            if (oldHasP && pts.size() == oldPoly) {
                points = pts;
                m_hasP = false;
                m_lastPressAddedPoint = false;
                return true;
            }

            if (m_closed.size() == 1) {
                points = pts;
                m_open = 0;

                if (oldHasP && points.size() >= 4) {
                    m_closed[0] = points.size() - 1;
                    m_hasP = true;
                } else if (points.size() >= 3) {
                    m_closed[0] = points.size();
                    m_hasP = false;
                } else {
                    m_closed.clear();
                    m_open = points.size();
                    m_phase = Phase::Input;
                    m_hasP = false;
                }

                m_lastPressAddedPoint = false;
                return true;
            }

            return false;
        }

        const bool simpleInput =
            !oldHasP &&
            (m_closed.isEmpty() || (m_closed.size() == 1 && m_open == 0));

        if (simpleInput) {
            points = pts;
            m_closed.clear();
            m_open = 0;

            if (points.size() >= 3) {
                m_closed.push_back(points.size());
            } else {
                m_open = points.size();
            }

            m_phase = Phase::Input;
            m_hasP = false;
            m_lastPressAddedPoint = false;
            return true;
        }

        return false;
    }
    bool canAddPointFromInput(const QVector<Point<Quotient_Number>>& points) const override {
        if (m_phase == Phase::Done) return !m_hasP && polygonValid();
        return StandardPointTask::canAddPoint(points);
    }

    bool addPointFromInput(const Point<Quotient_Number>& pt, QVector<Point<Quotient_Number>>& points) override {
        if (!canAddPointFromInput(points)) return false;
        points.push_back(pt);
        if (m_phase == Phase::Done) m_hasP = true;
        else ++m_open;
        m_lastPressAddedPoint = false;
        return true;
    }
    Point<double> m_cursor;
    bool m_hasCursor = false;
    double m_lastOffsetX = 0.0;
    double m_lastOffsetY = 0.0;
    double m_lastScale = 1.0;
    struct ScreenButton { QRect rect; int action; };
    QVector<ScreenButton> m_buttons;
    QPointF sceneToScreen(const QPointF& scene) const;
    void applyAction(int action, QVector<Point<Quotient_Number>>& points);
    int totalPoly() const;
    bool polygonValid() const;
    void finishPolygon();
    void closeContour(QVector<Point<Quotient_Number>>& points);
    void resetState();
    void sanitizeState(const QVector<Point<Quotient_Number>>& points);
    bool deletePointAt(QVector<Point<Quotient_Number>>& points, int idx);
    QVector<QVector<Point<Quotient_Number>>> collectContours(const QVector<Point<Quotient_Number>>& points) const;
    // 1 - внутри, -1 - снаружи, 0 - на границе, 2 - точки P нет.
    int pointStatus(const QVector<Point<Quotient_Number>>& points) const;
};
