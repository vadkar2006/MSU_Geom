#pragma once
#include <QWidget>
#include <QString>
#include <QVector>
#include <QPoint>
#include <QPointF>
#include <memory>
#include "Point.h"
#include "Task.h"
#include "Quotient_Number.h"

class CanvasWidget : public QWidget {
    Q_OBJECT
public:
    explicit CanvasWidget(QWidget* parent = nullptr);
    void setCurrentTask(std::shared_ptr<GeomTask> task);

public slots:
    void setPointRadius(double radius);
    void setEpsilon(double epsilon);
void setUseBigNum(bool use);
    void setShowEpsilonNeighborhood(bool show);
    void clearPoints();
    void resetCamera();
    
void addPointFromInput(const QString& xText, const QString& yText);
    QVector<Point<Quotient_Number>> points() const { return m_points; }
    void setPoints(const QVector<Point<Quotient_Number>>& pts);

    bool applyPointsFromDialog(const QVector<Point<Quotient_Number>>& pts, QString* message = nullptr);
signals:
    void coordinateInputApplied(const QString& xText, const QString& yText);
    void pointsChanged();


protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    QVector<Point<Quotient_Number>> m_points;
bool m_useBigNum = true;
    int m_draggingIndex = -1;
    int m_hoveredIndex = -1;
    std::shared_ptr<GeomTask> m_currentTask;
    
    double m_offsetX = 0.0;
    double m_offsetY = 0.0;
    double m_scale = 1.0;
    QPoint m_lastMousePos;
    
    QPointF mapToScene(const QPoint& screenPos) const;
};
