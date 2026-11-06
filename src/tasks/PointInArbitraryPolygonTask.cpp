#include "PointInArbitraryPolygonTask.h"
#include "Geometry.h"
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QColor>
#include <cmath>

namespace {
QString statusTextRu(int status)
{
    switch (status) {
    case 1:  return QStringLiteral("внутри");
    case -1: return QStringLiteral("снаружи");
    case 0:  return QStringLiteral("на границе");
    }
    return QStringLiteral("не определено");
}
QColor statusColor(int status)
{
    switch (status) {
    case 1:  return QColor(60, 170, 90);
    case -1: return QColor(200, 60, 60);
    case 0:  return QColor(230, 180, 60);
    }
    return QColor(150, 150, 150);
}
} // namespace

int PointInArbitraryPolygonTask::totalPoly() const
{
    int s = m_open;
    for (int v : m_closed) s += v;
    return s;
}

bool PointInArbitraryPolygonTask::polygonValid() const
{
    return !m_closed.isEmpty() && totalPoly() >= 3;
}

QPointF PointInArbitraryPolygonTask::sceneToScreen(const QPointF& scene) const
{
    return QPointF(scene.x() * m_lastScale + m_lastOffsetX,
                   m_lastOffsetY - scene.y() * m_lastScale);
}

void PointInArbitraryPolygonTask::resetState()
{
    m_phase = Phase::Input;
    m_closed.clear();
    m_open = 0;
    m_hasP = false;
    m_lastPressAddedPoint = false;
}

void PointInArbitraryPolygonTask::finishPolygon()
{
    if (m_open >= 3) {
        m_closed.push_back(m_open);
        m_open = 0;
    }
    if (m_closed.isEmpty()) return;
    m_phase = Phase::Done;
    m_hasP = false;
    m_lastPressAddedPoint = false;
}

void PointInArbitraryPolygonTask::closeContour(QVector<Point<Quotient_Number>>& points)
{
    if (m_open >= 3) {
        m_closed.push_back(m_open);
        m_open = 0;
        return;
    }
    while (m_open > 0 && !points.isEmpty()) {
        points.removeLast();
        m_open--;
    }
    if (!m_closed.isEmpty()) {
        m_phase = Phase::Done;
        m_hasP = false;
    }
}

void PointInArbitraryPolygonTask::applyAction(int action, QVector<Point<Quotient_Number>>& points)
{
    if (action == 0) {
        if (m_phase == Phase::Input && m_open >= 3) {
            m_closed.push_back(m_open);
            m_open = 0;
            m_lastPressAddedPoint = false;
        }
        return;
    }
    if (m_phase == Phase::Input) {
        finishPolygon();
    } else {
        resetState();
        points.clear();
    }
}

void PointInArbitraryPolygonTask::sanitizeState(const QVector<Point<Quotient_Number>>& points)
{
    const int expected = totalPoly() + (m_hasP ? 1 : 0);
    if (expected != points.size()) {
        m_closed.clear();
        m_open = 0;
        m_hasP = false;
        m_phase = Phase::Input;
        if (points.size() >= 3) m_closed.push_back(points.size());
        else m_open = points.size();
        m_lastPressAddedPoint = false;
    }
}

bool PointInArbitraryPolygonTask::deletePointAt(QVector<Point<Quotient_Number>>& points, int idx)
{
    if (idx < 0 || idx >= points.size()) return false;
    int pos = 0;
    for (int k = 0; k < m_closed.size(); ++k) {
        if (idx < pos + m_closed[k]) {
            m_closed[k]--;
            if (m_closed[k] <= 0) m_closed.removeAt(k);
            points.removeAt(idx);
            return true;
        }
        pos += m_closed[k];
    }
    if (idx < pos + m_open) {
        m_open--;
        points.removeAt(idx);
        return true;
    }
    return false;
}

QVector<QVector<Point<Quotient_Number>>>
PointInArbitraryPolygonTask::collectContours(const QVector<Point<Quotient_Number>>& points) const
{
    QVector<QVector<Point<Quotient_Number>>> out;
    int pos = 0;
    for (int sz : m_closed) {
        if (sz >= 3 && pos + sz <= points.size()) {
            QVector<Point<Quotient_Number>> c;
            c.reserve(sz);
            for (int i = 0; i < sz; ++i) c.push_back(points[pos + i]);
            out.push_back(c);
        }
        pos += sz;
    }
    return out;
}

int PointInArbitraryPolygonTask::pointStatus(const QVector<Point<Quotient_Number>>& points) const
{
    if (!m_hasP || points.isEmpty()) return 2;
    const QVector<QVector<Point<Quotient_Number>>> cont = collectContours(points);
    if (cont.isEmpty()) return 2;
    const Point<Quotient_Number> q = points.last();
    if (useBigNum()) {
        if (point_in_polygon::onBoundary(q, cont, Quotient_Number(0))) return 0;
        return point_in_polygon::parityIn(q, cont) ? 1 : -1;
    }
    QVector<QVector<Point<double>>> contD;
    contD.reserve(cont.size());
    for (const auto& c : cont) {
        QVector<Point<double>> cd;
        cd.reserve(c.size());
        for (const auto& pt : c) cd.push_back(pt.toDoublePoint());
        contD.push_back(cd);
    }
    const Point<double> qd = q.toDoublePoint();
    if (point_in_polygon::onBoundary(qd, contD, m_epsilon)) return 0;
    return point_in_polygon::parityIn(qd, contD) ? 1 : -1;
}

void PointInArbitraryPolygonTask::handleMousePress(QMouseEvent* event,
                                                   QVector<Point<Quotient_Number>>& points,
                                                   int& draggingIndex,
                                                   int& hoveredIndex)
{
    sanitizeState(points);
    {
        const QPointF scr = sceneToScreen(event->pos());
        for (const auto& btn : m_buttons) {
            if (btn.rect.contains(scr.toPoint())) {
                applyAction(btn.action, points);
                draggingIndex = -1;
                hoveredIndex = -1;
                return;
            }
        }
    }
    if (event->button() == Qt::LeftButton) {
        if (event->type() == QEvent::MouseButtonDblClick) {
            if (m_phase == Phase::Done) {
                resetState();
                points.clear();
                return;
            }
            if (m_lastPressAddedPoint && !points.isEmpty()) {
                points.removeLast();
                if (m_open > 0) m_open--;
                m_lastPressAddedPoint = false;
            }
            closeContour(points);
            return;
        }
        const int idx = pointIndexAt(event->pos(), points);
        if (idx != -1) {
            draggingIndex = idx;
            m_lastPressAddedPoint = false;
            return;
        }
        if (m_phase == Phase::Done) {
            if (!m_hasP && polygonValid() && canAddPointAt(event->pos(), points)) {
                points.push_back(makePointFromScene(event->pos()));
                m_hasP = true;
            }
            return;
        }
        points.push_back(makePointFromScene(event->pos()));
        m_open++;
        m_lastPressAddedPoint = true;
        return;
    }
    if (event->button() == Qt::RightButton) {
        const int idx = pointIndexAt(event->pos(), points);
        if (idx == -1) return;
        if (m_hasP && idx == points.size() - 1) {
            points.removeLast();
            m_hasP = false;
            draggingIndex = -1;
            hoveredIndex = -1;
            return;
        }
        deletePointAt(points, idx);
        if (totalPoly() < 3) {
            m_phase = Phase::Input;
            if (m_hasP && !points.isEmpty()) {
                points.removeLast();
                m_hasP = false;
            }
            m_closed.clear();
            m_open = points.size();
        }
        draggingIndex = -1;
        hoveredIndex = -1;
        return;
    }
    StandardPointTask::handleMousePress(event, points, draggingIndex, hoveredIndex);
}

void PointInArbitraryPolygonTask::handleMouseMove(QMouseEvent* event,
                                                  QVector<Point<Quotient_Number>>& points,
                                                  int& draggingIndex,
                                                  int& hoveredIndex,
                                                  QWidget* widget)
{
    m_cursor = Point<double>(event->pos().x(), event->pos().y());
    m_hasCursor = true;
    StandardPointTask::handleMouseMove(event, points, draggingIndex, hoveredIndex, widget);
}

void PointInArbitraryPolygonTask::handleKeyPress(QKeyEvent* event,
                                                 QVector<Point<Quotient_Number>>& points,
                                                 int& draggingIndex,
                                                 int& hoveredIndex)
{
    Q_UNUSED(draggingIndex);
    Q_UNUSED(hoveredIndex);
    const int key = event->key();
    if (key == Qt::Key_Return || key == Qt::Key_Enter || key == Qt::Key_Space) {
        applyAction(1, points);
        event->accept();
        return;
    }
    if (key == Qt::Key_Backspace && m_phase == Phase::Input) {
        if (m_open > 0 && !points.isEmpty()) {
            points.removeLast();
            m_open--;
            m_lastPressAddedPoint = false;
            event->accept();
            return;
        }
    }
    event->ignore();
}

void PointInArbitraryPolygonTask::draw(QPainter& p, const QRect& rect,
                                       const QVector<Point<Quotient_Number>>& points,
                                       int draggingIndex, int hoveredIndex,
                                       double offsetX, double offsetY, double scale)
{
    sanitizeState(points);
    drawCoordinateGrid(p, rect, offsetX, offsetY, scale);
    const double invScale = (scale > 1e-12) ? (1.0 / scale) : 1.0;
    m_lastOffsetX = offsetX;
    m_lastOffsetY = offsetY;
    m_lastScale = scale;
    const QVector<QVector<Point<Quotient_Number>>> cont = collectContours(points);
    const int tot = totalPoly();
    int status = 2;
    if (m_phase == Phase::Done && m_hasP && !points.isEmpty()) {
        status = pointStatus(points);
    }
    auto buildPath = [](const QVector<QVector<Point<Quotient_Number>>>& cs) {
        QPainterPath path;
        for (const auto& c : cs) {
            if (c.size() < 3) continue;
            path.moveTo(c[0].toQPointF());
            for (int i = 1; i < c.size(); ++i) path.lineTo(c[i].toQPointF());
            path.closeSubpath();
        }
        return path;
    };
    p.save();
    p.translate(offsetX, offsetY);
    p.scale(scale, -scale); // Y направлен вверх
    const QPainterPath pathPoly = buildPath(cont);
    if (!pathPoly.isEmpty()) {
        p.fillPath(pathPoly, QBrush(QColor(180, 186, 200, 130)));
        p.setPen(QPen(QColor(60, 80, 130), 2.0 * invScale));
        p.setBrush(Qt::NoBrush);
        p.drawPath(pathPoly);
    }
    if (m_phase == Phase::Input && m_open >= 2) {
        const int base = tot - m_open;
        p.setPen(QPen(QColor(100, 130, 210), 2.0 * invScale, Qt::DashLine));
        for (int i = base + 1; i < base + m_open; ++i) {
            p.drawLine(points[i - 1].toQPointF(), points[i].toQPointF());
        }
        if (m_hasCursor) {
            p.setPen(QPen(QColor(90, 90, 90), 1.5 * invScale, Qt::DashLine));
            p.drawLine(points[base + m_open - 1].toQPointF(), m_cursor.toQPointF());
            p.drawLine(m_cursor.toQPointF(), points[base].toQPointF());
        }
    }
    // Вершины многоугольника.
    for (int i = 0; i < tot && i < points.size(); ++i) {
        const bool isActive = (i == draggingIndex || i == hoveredIndex);
        p.setBrush(QColor(250, 250, 250));
        p.setPen(isActive ? QPen(QColor(255, 180, 0), 3.0 * invScale)
                          : QPen(QColor(90, 120, 190), 2.0 * invScale));
        p.drawEllipse(points[i].toQPointF(),
                      m_pointRadius * 0.6 * invScale,
                      m_pointRadius * 0.6 * invScale);
    }
    // Точка P.
    if (m_hasP && !points.isEmpty()) {
        const int lastIdx = points.size() - 1;
        const bool isActive = (lastIdx == draggingIndex || lastIdx == hoveredIndex);
        p.setBrush(statusColor(status));
        p.setPen(isActive ? QPen(QColor(255, 180, 0), 3.0 * invScale)
                          : QPen(Qt::black, 2.0 * invScale));
        p.drawEllipse(points[lastIdx].toQPointF(),
                      m_pointRadius * invScale, m_pointRadius * invScale);
    }
    // Epsilon-окрестность.
    if (m_showEpsilonNeighborhood) {
        const double r = (m_epsilon > 0.0) ? std::sqrt(m_epsilon) : 0.0;
        if (r > 0.0) {
            p.setPen(QPen(QColor(255, 100, 0), 2.0 * invScale, Qt::DashLine));
            p.setBrush(Qt::NoBrush);
            for (const auto& pt : points) {
                p.drawEllipse(pt.toQPointF(), r, r);
            }
        }
    }
    p.restore();
    // Подписи
    p.save();
    p.resetTransform();
    if (points.size() <= 60) {
        p.setFont(QFont("Arial", 9));
        for (int i = 0; i < points.size(); ++i) {
            const Point<double> pt = points[i].toDoublePoint();
            const QPointF screen(pt.x * scale + offsetX, offsetY - pt.y * scale);
            if (i < tot) {
                p.setPen(QColor(60, 60, 60));
                p.drawText(screen + QPointF(10.0, -10.0), QString::number(i));
            } else {
                p.setFont(QFont("Arial", 12, QFont::Bold));
                p.setPen(statusColor(status));
                p.drawText(screen + QPointF(16.0, -12.0),
                           QStringLiteral("P: %1").arg(statusTextRu(status)));
                p.setFont(QFont("Arial", 9));
            }
        }
    }
    // Экранные кнопки задачи.
    m_buttons.clear();
    p.setFont(QFont("Arial", 10, QFont::Bold));
    if (m_phase == Phase::Input) {
        const QRect btn0(rect.right() - 300, rect.top() + 12, 288, 30);
        const bool canClose = (m_open >= 3);
        p.setBrush(canClose ? QColor(235, 245, 235) : QColor(242, 242, 242));
        p.setPen(QPen(canClose ? QColor(60, 140, 60) : QColor(160, 160, 160), 2));
        p.drawRoundedRect(btn0, 6, 6);
        p.setPen(canClose ? QColor(20, 90, 20) : QColor(140, 140, 140));
        p.drawText(btn0, Qt::AlignCenter,
                   QStringLiteral("+ контур (дырка): замкнуть [двойной клик]"));
        m_buttons.push_back(ScreenButton{btn0, 0});
        const QRect btn1(rect.right() - 300, rect.top() + 48, 288, 30);
        p.setBrush(QColor(235, 245, 235));
        p.setPen(QPen(QColor(60, 140, 60), 2));
        p.drawRoundedRect(btn1, 6, 6);
        p.setPen(QColor(20, 90, 20));
        p.drawText(btn1, Qt::AlignCenter,
                   QStringLiteral("Завершить многоугольник [Enter]"));
        m_buttons.push_back(ScreenButton{btn1, 1});
    } else {
        const QRect btn1(rect.right() - 300, rect.top() + 12, 288, 30);
        p.setBrush(QColor(245, 235, 235));
        p.setPen(QPen(QColor(160, 60, 60), 2));
        p.drawRoundedRect(btn1, 6, 6);
        p.setPen(QColor(120, 20, 20));
        p.drawText(btn1, Qt::AlignCenter,
                   QStringLiteral("Начать заново [Enter или двойной клик]"));
        m_buttons.push_back(ScreenButton{btn1, 1});
    }
    // Легенда статусов точки P.
    const QRect legendRect(rect.right() - 160, rect.bottom() - 92, 150, 80);
    p.setPen(QPen(QColor(200, 200, 200), 1));
    p.setBrush(QColor(255, 255, 255, 210));
    p.drawRect(legendRect);
    p.setFont(QFont("Arial", 9));
    const QColor legendColors[3] = { statusColor(1), statusColor(-1), statusColor(0) };
    const QString legendTexts[3] = { statusTextRu(1), statusTextRu(-1), statusTextRu(0) };
    for (int i = 0; i < 3; ++i) {
        const int y = legendRect.top() + 18 + i * 20;
        p.setPen(Qt::NoPen);
        p.setBrush(legendColors[i]);
        p.drawRoundedRect(QRect(legendRect.left() + 10, y - 10, 12, 12), 3, 3);
        p.setPen(QColor(60, 60, 60));
        p.drawText(legendRect.left() + 30, y, legendTexts[i]);
    }
    // Строки состояния.
    p.setFont(QFont("Arial", 12, QFont::Bold));
    QString statusStr;
    QColor textColor = Qt::black;
    if (m_phase == Phase::Input) {
        statusStr = QStringLiteral("Многоугольник: контуров %1, в текущем контуре вершин %2; клик - вершина, двойной клик - замкнуть")
                     .arg(m_closed.size()).arg(m_open);
    } else if (!m_hasP) {
        statusStr = QStringLiteral("Многоугольник готов: контуров %1, вершин %2. Клик - поставить точку P.")
                     .arg(m_closed.size()).arg(tot);
    } else {
        statusStr = QStringLiteral("P %1. Kontуров: %2, вершин: %3")
                     .arg(statusTextRu(status)).arg(m_closed.size()).arg(tot);
        textColor = statusColor(status);
    }
    p.setPen(textColor);
    p.drawText(20, 30, statusStr);
    p.setFont(QFont("Arial", 9));
    p.setPen(Qt::darkGray);
    p.drawText(20, 48, QStringLiteral("Правило заполнения: чёт/нечет (дырки и самопересечения учитываются); Backspace - отменить вершину"));
    p.drawText(20, 64, QStringLiteral("Дырка: новый контур внутри многоугольника; ПКМ - удалить вершину или точку P"));
    p.restore();
}
