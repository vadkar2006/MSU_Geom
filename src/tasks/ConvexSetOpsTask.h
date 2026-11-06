#pragma once
#include "Task.h"
#include "Quotient_Number.h"

class ConvexSetOpsTask : public StandardPointTask {
public:
    ConvexSetOpsTask();
    QStringList operationOptions() const override;
    int  currentOperation() const override { return m_operationMode; }
    void setOperation(int index) override;
    QString name() const override { return QStringLiteral("Операции над выпуклыми множествами"); }
    QString description() const override { 
        return QStringLiteral("ЛКМ - точки A, двойной ЛКМ - завершить A; ЛКМ - точки B, двойной ЛКМ - результат. Точки можно перетаскивать, ПКМ - удалить."); 
    }
    
    void draw(QPainter& p, const QRect& rect, const QVector<Point<Quotient_Number>>& points,
              int draggingIndex, int hoveredIndex, double offsetX, double offsetY, double scale) override;
              
    void handleMousePress(QMouseEvent* event, QVector<Point<Quotient_Number>>& points, int& draggingIndex, int& hoveredIndex) override;
    void handleMouseRelease(QMouseEvent* event, QVector<Point<Quotient_Number>>& points, int& draggingIndex) override;
    
    int maxPoints() const override { return -1; }

private:
    enum class Phase { InputA, InputB, Done };
    Phase m_phase = Phase::InputA;
    
    int m_countA = 0;
    static QVector<int> hullIndices(const QVector<Point<Quotient_Number>>& pts,
                                    bool exact, double epsilon);
    
    int m_operationMode = 0;


    bool setPointsFromExternal(const QVector<Point<Quotient_Number>>& pts,
                               QVector<Point<Quotient_Number>>& points) override {
        points = pts;

        if (m_phase == Phase::InputA) {
            m_countA = 0;
            return true;
        }

        if (m_countA < 0) {
            m_countA = 0;
        }

        if (m_countA > points.size()) {
            m_phase = Phase::InputA;
            m_countA = 0;
            return true;
        }

        if (m_phase == Phase::Done && points.size() < m_countA + 3) {
            m_phase = Phase::InputB;
        }

        return true;
    }


    bool canAddPointFromInput(const QVector<Point<Quotient_Number>>& points) const override {
        if (m_phase == Phase::Done) return false;
        return StandardPointTask::canAddPoint(points);
    }
    
    void finalizePolygonA(QVector<Point<Quotient_Number>>& points);
    void finalizePolygonB(QVector<Point<Quotient_Number>>& points);
    void resetState();
    
    bool isPointInConvexPoly(const Point<double>& p, const QVector<Point<double>>& poly, double eps) const;
};
