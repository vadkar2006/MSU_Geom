#include "CanvasWidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QCursor>
#include "Geometry.h"
#include <cmath>
#include <algorithm>
#include <sstream>
#include <QMessageBox>


namespace {
bool parseQuotientDirect(const QString& text, Quotient_Number& out, QString& error)
{
    const QString trimmed = text.trimmed();
    if (trimmed.isEmpty()) {
        error = QStringLiteral("Пустой текст");
        return false;
    }

    std::istringstream in(trimmed.toStdString());

    try {
        in >> out;
    } catch (...) {
        error = QStringLiteral("Оператор ввода не смог прочитать число");
        return false;
    }

    if (in.fail()) {
        error = QStringLiteral("Оператор ввода не принял данные");
        return false;
    }

    return true;
}
} // namespace

CanvasWidget::CanvasWidget(QWidget* parent) : QWidget(parent) {
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    resetCamera();
}

void CanvasWidget::setCurrentTask(std::shared_ptr<GeomTask> task) {
    if (task) task->setUseBigNum(m_useBigNum);
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
    emit pointsChanged();
}

void CanvasWidget::resetCamera() {
    m_offsetX = 50.0;
    m_offsetY = (height() > 0 ? static_cast<double>(height()) - 50.0 : 600.0);
    m_scale = 1.0;
    update();
}

QPointF CanvasWidget::mapToScene(const QPoint& screenPos) const {
    return QPointF(
        (screenPos.x() - m_offsetX) / m_scale,
        (m_offsetY - screenPos.y()) / m_scale
    );
}

void CanvasWidget::paintEvent(QPaintEvent* /*event*/) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    
    
    if (m_currentTask) {
        m_currentTask->draw(p, QRect(0, 0, width(), height()), m_points, m_draggingIndex, m_hoveredIndex, m_offsetX, m_offsetY, m_scale);
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
        m_currentTask->setViewScale(m_scale);
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
        emit pointsChanged();
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
        m_currentTask->setViewScale(m_scale);
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
        m_currentTask->setViewScale(m_scale);
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
        emit pointsChanged();
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
    newScale = std::max(1e-6, std::min(1e6, newScale));
    
    QPointF mousePos = event->position();
    double ratio = newScale / m_scale;
    m_offsetX = mousePos.x() - (mousePos.x() - m_offsetX) * ratio;
    m_offsetY = mousePos.y() - (mousePos.y() - m_offsetY) * ratio;
    m_scale = newScale;
    
    update();
    event->accept();
}

void CanvasWidget::keyPressEvent(QKeyEvent* event) {
    if (m_currentTask) {
        m_currentTask->handleKeyPress(event, m_points, m_draggingIndex, m_hoveredIndex);
        if (event->isAccepted()) {
            update();
            emit pointsChanged();
            return;
        }
    }
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

void CanvasWidget::setUseBigNum(bool use) {
    m_useBigNum = use;
    if (m_currentTask) m_currentTask->setUseBigNum(use);
    update();
}

void CanvasWidget::addPointFromInput(const QString& xText, const QString& yText)
{
    if (!m_currentTask) return;

    if (!m_currentTask->canAddPointFromInput(m_points)) {
        QMessageBox::information(this, QStringLiteral("Ограничение задачи"),
            QStringLiteral("Текущая задача не может принять ещё одну точку из ввода."));
        return;
    }

    const QString xTrimmed = xText.trimmed();
    const QString yTrimmed = yText.trimmed();

    if (xTrimmed.isEmpty() || yTrimmed.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Ввод координат"),
            QStringLiteral("Заполните обе координаты."));
        return;
    }

    Quotient_Number x(0);
    Quotient_Number y(0);
    QString error;

    if (!parseQuotientDirect(xTrimmed, x, error)) {
        QMessageBox::warning(this, QStringLiteral("Проблема ввода X"),
            QStringLiteral("Не удалось прочитать X: %1").arg(error));
        return;
    }

    if (!parseQuotientDirect(yTrimmed, y, error)) {
        QMessageBox::warning(this, QStringLiteral("Проблема ввода Y"),
            QStringLiteral("Не удалось прочитать Y: %1").arg(error));
        return;
    }

    if (!m_currentTask->addPointFromInput(Point<Quotient_Number>(x, y), m_points)) {
        QMessageBox::information(this, QStringLiteral("Ограничение задачи"),
                                 QStringLiteral("Текущая задача не может принять точку из ввода."));
        return;
    }
    m_draggingIndex = -1;
    m_hoveredIndex = -1;

    emit coordinateInputApplied(xTrimmed, yTrimmed);

    update();
    emit pointsChanged();
}



void CanvasWidget::setPoints(const QVector<Point<Quotient_Number>>& pts)
{
    QVector<Point<Quotient_Number>> incoming = pts;

    if (m_currentTask) {
        const int limit = m_currentTask->maxPoints();
        if (limit >= 0 && static_cast<int>(incoming.size()) > limit) {
            QMessageBox::information(this, QStringLiteral("Лимит точек"),
                                     QStringLiteral("Для текущей задачи можно использовать не более %1 точек. Лишние точки удалены.")
                                     .arg(limit));
            incoming.resize(limit);
        }

        if (!m_currentTask->setPointsFromExternal(incoming, m_points)) {
            QMessageBox::warning(this, QStringLiteral("Окно точек"),
                                 QStringLiteral("Текущая задача не может корректно применить такой набор точек без потери структуры."));
            return;
        }
    } else {
        m_points = incoming;
    }

    m_draggingIndex = -1;
    m_hoveredIndex = -1;
    update();
    emit pointsChanged();
}



bool CanvasWidget::applyPointsFromDialog(const QVector<Point<Quotient_Number>>& pts, QString* message)
{
    QVector<Point<Quotient_Number>> incoming = pts;

    if (m_currentTask) {
        const int limit = m_currentTask->maxPoints();

        if (limit >= 0 && static_cast<int>(incoming.size()) > limit) {
            if (message) {
                *message = QStringLiteral(
                    "Для текущей задачи можно использовать не более %1 точек. "
                    "Лишние точки удалены."
                ).arg(limit);
            }

            incoming.resize(limit);
        }

        if (!m_currentTask->setPointsFromExternal(incoming, m_points)) {
            if (message) {
                *message = QStringLiteral(
                    "Текущая задача не может корректно применить такой набор точек "
                    "без потери структуры."
                );
            }

            return false;
        }
    } else {
        m_points = incoming;
    }

    m_draggingIndex = -1;
    m_hoveredIndex = -1;

    update();
    emit pointsChanged();

    if (message && message->isEmpty()) {
        *message = QStringLiteral("Применено точек: %1").arg(m_points.size());
    }

    return true;
}
