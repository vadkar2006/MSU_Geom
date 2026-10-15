#pragma once
#include <QWidget>
#include <QVector>
#include <QPoint>
#include <QPointF>
#include <memory>
#include "Point.h"
#include "Task.h"

class CanvasWidget : public QWidget {
    Q_OBJECT
public:
    explicit CanvasWidget(QWidget* parent = nullptr);
    void setCurrentTask(std::shared_ptr<Task> task);

public slots:
    void setPointRadius(double radius);
    void setEpsilon(double epsilon);
    void setShowEpsilonNeighborhood(bool show);
    void clearPoints();
    void resetCamera();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    QVector<Point> m_points;
    int m_draggingIndex = -1;
    int m_hoveredIndex = -1;
    std::shared_ptr<Task> m_currentTask;
    
    double m_offsetX = 0.0;
    double m_offsetY = 0.0;
    double m_scale = 1.0;
    QPoint m_lastMousePos;
    
    QPointF mapToScene(const QPoint& screenPos) const;
};
