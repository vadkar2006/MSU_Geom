#pragma once

#include "Task.h"
#include "Quotient_Number.h"


class ArbitraryPolygonOpsTask : public StandardPointTask {
public:
    QStringList operationOptions() const override;
    int  currentOperation() const override { return m_operationMode; }
    void setOperation(int index) override;
    QString name() const override { return QStringLiteral("Операции над произвольными многоугольниками"); }
    QString description() const override {
        return QStringLiteral("ЛКМ - вершина контура по порядку; двойной ЛКМ - замкнуть контур (дырку); "
                              "Enter или кнопка на холсте - завершить многоугольник (A -> B -> результат); Backspace - отменить последнюю вершину. "
                              "Дырка: новый контур внутри многоугольника. Перетаскивание работает, ПКМ - удалить вершину.");
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
    enum class Phase { InputA, InputB, Done };
    Phase m_phase = Phase::InputA;
    QVector<int> m_closedA;
    QVector<int> m_closedB;
    int m_openA = 0;
    int m_openB = 0;
    int m_operationMode = 0;


    bool setPointsFromExternal(const QVector<Point<Quotient_Number>>& pts,
                               QVector<Point<Quotient_Number>>& points) override {
        if (totalA() + totalB() == pts.size()) {
            points = pts;
            m_lastPressAddedPoint = false;
            return true;
        }

        if (m_phase == Phase::InputA) {
            const bool simpleA =
                m_closedA.isEmpty() ||
                (m_closedA.size() == 1 && m_openA == 0);

            if (!simpleA) {
                return false;
            }

            points = pts;
            resetState();

            if (points.size() >= 3) {
                m_closedA.push_back(points.size());
            } else {
                m_openA = points.size();
            }

            return true;
        }

        const int keepA = totalA();

        if (pts.size() >= keepA && m_closedB.isEmpty()) {
            points = pts;
            m_openB = points.size() - keepA;

            if (m_phase == Phase::Done) {
                m_phase = Phase::InputB;
            }

            m_lastPressAddedPoint = false;
            return true;
        }

        return false;
    }


    bool canAddPointFromInput(const QVector<Point<Quotient_Number>>& points) const override {
        return (m_phase != Phase::Done) && StandardPointTask::canAddPoint(points);
    }

    bool addPointFromInput(const Point<Quotient_Number>& pt, QVector<Point<Quotient_Number>>& points) override {
        if (!canAddPointFromInput(points)) return false;
        points.push_back(pt);
        ++currentOpen();
        m_lastPressAddedPoint = false;
        return true;
    }
    bool m_lastPressAddedPoint = false;
    Point<double> m_cursor;
    bool m_hasCursor = false;
    double m_lastOffsetX = 0.0;
    double m_lastOffsetY = 0.0;
    double m_lastScale = 1.0;
    struct ScreenButton { QRect rect; int action; };
    QVector<ScreenButton> m_buttons;
    QPointF sceneToScreen(const QPointF& scene) const;
    void applyAction(int action, QVector<Point<Quotient_Number>>& points);

    int totalA() const;
    int totalB() const;
    int& currentOpen();
    QVector<int>& currentClosed();
    void finishPolygon();
    void resetState();
    void sanitizeState(const QVector<Point<Quotient_Number>>& points);
    bool deletePointAt(QVector<Point<Quotient_Number>>& points, int idx);
    void collectContours(const QVector<Point<Quotient_Number>>& points,
                         QVector<QVector<Point<double>>>& outA,
                         QVector<QVector<Point<double>>>& outB) const;
};
