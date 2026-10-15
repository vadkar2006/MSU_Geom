#include "CanvasWidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QCursor>
#include "Geometry.h"
#include <cmath>
#include <algorithm>

CanvasWidget::CanvasWidget(QWidget* parent) : QWidget(parent) {
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    m_points.push_back(Point(300, 300));
    m_points.push_back(Point(600, 300));
    m_points.push_back(Point(450, 150));
}

void CanvasWidget::setCurrentTask(std::shared_ptr<Task> task) {
    m_currentTask = task;
    update();
}

void CanvasWidget::setPointRadius(double radius) {
    if (m_currentTask) { m_currentTask->setPointRadius(radius); update(); }
}

void CanvasWidget::setEpsilon(double epsilon) {
    if (m_currentTask) { m_currentTask->setEpsilon(epsilon); update(); }
}

void CanvasWidget::setShowEpsilonNeighborhood(bool show) {
    if (m_currentTask) { m_currentTask->setShowEpsilonNeighborhood(show); update(); }
}

void CanvasWidget::clearPoints() {
    m_points.clear();
    m_draggingIndex = -1;
    m_hoveredIndex = -1;
    update();
}

void CanvasWidget::resetCamera() {
    m_offsetX = 0.0;
    m_offsetY = 0.0;
    m_scale = 1.0;
    update();
}

QPointF CanvasWidget::mapToScene(const QPoint& screenPos) const {
    return QPointF(
        (screenPos.x() - m_offsetX) / m_scale,
        (screenPos.y() - m_offsetY) / m_scale
    );
}

void CanvasWidget::paintEvent(QPaintEvent* /*event*/) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    
    // Видимая область в мировых координатах
    QRectF worldRect(
        -m_offsetX / m_scale,
        -m_offsetY / m_scale,
        width() / m_scale,
        height() / m_scale
    );
    
    if (m_currentTask) {
        m_currentTask->draw(p, worldRect.toRect(), m_points, m_draggingIndex, m_hoveredIndex, m_offsetX, m_offsetY, m_scale);
    }
    
    // HUD
    p.setPen(QColor(100, 100, 100));
    p.setFont(QFont("Arial", 9));
    p.drawText(10, height() - 10, 
        QString("Масштаб: %1x | WASD/ЦФЫВ — перемещение | Колесо — зум | Home — сброс")
            .arg(m_scale, 0, 'f', 2));
}

void CanvasWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::MiddleButton) {
        m_lastMousePos = event->pos();
        setCursor(Qt::ClosedHandCursor);
        return;
    }
    
    if (m_currentTask) {
        QPointF scenePos = mapToScene(event->pos());
        QMouseEvent mappedEvent(
            event->type(),
            scenePos,
            QPointF(event->globalPos()),
            event->button(),
            event->buttons(),
            event->modifiers()
        );
        m_currentTask->handleMousePress(&mappedEvent, m_points, m_draggingIndex, m_hoveredIndex);
        update();
    }
}

void CanvasWidget::mouseMoveEvent(QMouseEvent* event) {
    if (event->buttons() & Qt::MiddleButton) {
        QPoint delta = event->pos() - m_lastMousePos;
        m_offsetX += delta.x();
        m_offsetY += delta.y();
        m_lastMousePos = event->pos();
        update();
        return;
    }
    
    if (m_currentTask) {
        QPointF scenePos = mapToScene(event->pos());
        QMouseEvent mappedEvent(
            event->type(),
            scenePos,
            QPointF(event->globalPos()),
            event->button(),
            event->buttons(),
            event->modifiers()
        );
        m_currentTask->handleMouseMove(&mappedEvent, m_points, m_draggingIndex, m_hoveredIndex, this);
        update();
    }
}

void CanvasWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::MiddleButton) {
        setCursor(Qt::ArrowCursor);
        return;
    }
    
    if (m_currentTask) {
        QPointF scenePos = mapToScene(event->pos());
        QMouseEvent mappedEvent(
            event->type(),
            scenePos,
            QPointF(event->globalPos()),
            event->button(),
            event->buttons(),
            event->modifiers()
        );
        m_currentTask->handleMouseRelease(&mappedEvent, m_points, m_draggingIndex);
        update();
    }
}

void CanvasWidget::leaveEvent(QEvent* event) {
    if (m_currentTask) {
        m_currentTask->handleLeave(m_points, m_hoveredIndex, this);
    }
    setCursor(Qt::ArrowCursor);
    QWidget::leaveEvent(event);
}

void CanvasWidget::wheelEvent(QWheelEvent* event) {
    QPoint degrees = event->angleDelta();
    if (degrees.isNull()) return;
    
    double factor = (degrees.y() > 0) ? 1.15 : (1.0 / 1.15);
    double newScale = m_scale * factor;
    newScale = std::max(0.05, std::min(50.0, newScale));
    
    QPointF mousePos = event->position();
    double ratio = newScale / m_scale;
    m_offsetX = mousePos.x() - (mousePos.x() - m_offsetX) * ratio;
    m_offsetY = mousePos.y() - (mousePos.y() - m_offsetY) * ratio;
    m_scale = newScale;
    
    update();
    event->accept();
}

void CanvasWidget::keyPressEvent(QKeyEvent* event) {
    const double moveStep = 20.0;
    QString text = event->text().toLower();
    
    // WASD
    if (event->key() == Qt::Key_W) { m_offsetY += moveStep; update(); }
    else if (event->key() == Qt::Key_S) { m_offsetY -= moveStep; update(); }
    else if (event->key() == Qt::Key_A) { m_offsetX += moveStep; update(); }
    else if (event->key() == Qt::Key_D) { m_offsetX -= moveStep; update(); }
    // ЦФЫВ
    else if (text == "ц") { m_offsetY += moveStep; update(); }
    else if (text == "ы") { m_offsetY -= moveStep; update(); }
    else if (text == "ф") { m_offsetX += moveStep; update(); }
    else if (text == "в") { m_offsetX -= moveStep; update(); }
    else if (event->key() == Qt::Key_Home) { resetCamera(); }
    else { QWidget::keyPressEvent(event); }
}
