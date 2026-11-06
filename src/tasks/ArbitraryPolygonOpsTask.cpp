#include "ArbitraryPolygonOpsTask.h"
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
#include <vector>
#include <map>
#include <algorithm>

namespace {
using ContourD = QVector<Point<double>>;


bool parityIn(const Point<double>& p, const QVector<ContourD>& cs)
{
    int cnt = 0;
    for (const auto& c : cs) {
        const int n = c.size();
        for (int i = 0; i < n; ++i) {
            const Point<double>& a = c[i];
            const Point<double>& b = c[(i + 1) % n];
            if ((a.y > p.y) != (b.y > p.y)) {
                const double xint = a.x + (p.y - a.y) * (b.x - a.x) / (b.y - a.y);
                if (xint > p.x) ++cnt;
            }
        }
    }
    return (cnt & 1) == 1;
}

double orientD(const Point<double>& a, const Point<double>& b, const Point<double>& c, double eps)
{
    const double cr = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
    if (cr > eps) return 1.0;
    if (cr < -eps) return -1.0;
    return 0.0;
}

bool inBoxD(const Point<double>& p, const Point<double>& s0, const Point<double>& s1, double q)
{
    const double mnx = std::min(s0.x, s1.x) - q, mxx = std::max(s0.x, s1.x) + q;
    const double mny = std::min(s0.y, s1.y) - q, mxy = std::max(s0.y, s1.y) + q;
    return p.x >= mnx && p.x <= mxx && p.y >= mny && p.y <= mxy;
}

QPainterPath computeResultPath(const QVector<ContourD>& A, const QVector<ContourD>& B, int opMode)
{
    std::vector<Point<double>> V;
    std::vector<std::pair<int, int>> E;
    auto addConturs = [&](const QVector<ContourD>& cs) {
        for (const auto& c : cs) {
            if (c.size() < 3) continue;
            const int base = static_cast<int>(V.size());
            for (const auto& pt : c) V.push_back(pt);
            const int n = c.size();
            for (int i = 0; i < n; ++i) E.push_back({base + i, base + (i + 1) % n});
        }
    };
    addConturs(A);
    addConturs(B);
    QPainterPath empty;
    if (V.empty() || E.empty()) return empty;

    double minX = V[0].x, maxX = V[0].x, minY = V[0].y, maxY = V[0].y;
    for (const auto& v : V) {
        minX = std::min(minX, v.x); maxX = std::max(maxX, v.x);
        minY = std::min(minY, v.y); maxY = std::max(maxY, v.y);
    }
    const double span = std::max(1.0, std::max(maxX - minX, maxY - minY));
    const double q = 1e-6 * span;
    const double epsO = 1e-9 * span * span;
    const double off = q;

    auto fOp = [&](bool inA, bool inB) -> bool {
        switch (opMode) {
        case 0:  return inA && inB;
        case 1:  return inA || inB;
        case 2:  return inA && !inB;
        default: return inA != inB;
        }
    };

    std::vector<std::vector<double>> ts(E.size());
    for (size_t i = 0; i < E.size(); ++i) ts[i] = {0.0, 1.0};
    auto addT = [](std::vector<double>& v, double t, double tol) {
        t = std::min(1.0, std::max(0.0, t));
        for (double e : v) if (std::abs(e - t) <= tol) return;
        v.push_back(t);
    };
    for (size_t i = 0; i < E.size(); ++i) {
        for (size_t j = i + 1; j < E.size(); ++j) {
            const Point<double> a = V[E[i].first], b = V[E[i].second];
            const Point<double> c = V[E[j].first], d = V[E[j].second];
            const double abx = b.x - a.x, aby = b.y - a.y;
            const double cdx = d.x - c.x, cdy = d.y - c.y;
            const double len2ab = abx * abx + aby * aby;
            const double len2cd = cdx * cdx + cdy * cdy;
            if (len2ab <= q * q || len2cd <= q * q) continue;
            const double tolA = q / std::sqrt(len2ab);
            const double tolB = q / std::sqrt(len2cd);
            const double o1 = orientD(a, b, c, epsO), o2 = orientD(a, b, d, epsO);
            const double o3 = orientD(c, d, a, epsO), o4 = orientD(c, d, b, epsO);
            if (o1 != 0 && o2 != 0 && o3 != 0 && o4 != 0 && o1 != o2 && o3 != o4) {
                const double den = abx * cdy - aby * cdx;
                if (std::abs(den) > 1e-300) {
                    const double t = ((c.x - a.x) * cdy - (c.y - a.y) * cdx) / den;
                    const double s = ((c.x - a.x) * aby - (c.y - a.y) * abx) / den;
                    addT(ts[i], t, tolA);
                    addT(ts[j], s, tolB);
                }
            } else if (o1 == 0 && o2 == 0 && o3 == 0 && o4 == 0) {
                if (inBoxD(c, a, b, q)) addT(ts[i], ((c.x - a.x) * abx + (c.y - a.y) * aby) / len2ab, tolA);
                if (inBoxD(d, a, b, q)) addT(ts[i], ((d.x - a.x) * abx + (d.y - a.y) * aby) / len2ab, tolA);
                if (inBoxD(a, c, d, q)) addT(ts[j], ((a.x - c.x) * cdx + (a.y - c.y) * cdy) / len2cd, tolB);
                if (inBoxD(b, c, d, q)) addT(ts[j], ((b.x - c.x) * cdx + (b.y - c.y) * cdy) / len2cd, tolB);
            } else {
                if (o1 == 0 && inBoxD(c, a, b, q)) addT(ts[i], ((c.x - a.x) * abx + (c.y - a.y) * aby) / len2ab, tolA);
                if (o2 == 0 && inBoxD(d, a, b, q)) addT(ts[i], ((d.x - a.x) * abx + (d.y - a.y) * aby) / len2ab, tolA);
                if (o3 == 0 && inBoxD(a, c, d, q)) addT(ts[j], ((a.x - c.x) * cdx + (a.y - c.y) * cdy) / len2cd, tolB);
                if (o4 == 0 && inBoxD(b, c, d, q)) addT(ts[j], ((b.x - c.x) * cdx + (b.y - c.y) * cdy) / len2cd, tolB);
            }
        }
    }

    struct DEdge { int u; int v; };
    std::vector<DEdge> DE;
    std::map<std::pair<long long, long long>, int> vid;
    std::vector<Point<double>> P;
    auto idOf = [&](const Point<double>& pt) -> int {
        const auto key = std::make_pair(std::llround(pt.x / q), std::llround(pt.y / q));
        auto it = vid.find(key);
        if (it != vid.end()) return it->second;
        const int id = static_cast<int>(P.size());
        P.push_back(pt);
        vid.emplace(key, id);
        return id;
    };
    for (size_t i = 0; i < E.size(); ++i) {
        const Point<double> a = V[E[i].first], b = V[E[i].second];
        const double abx = b.x - a.x, aby = b.y - a.y;
        const double len = std::sqrt(abx * abx + aby * aby);
        if (len <= q) continue;
        auto& vv = ts[i];
        std::sort(vv.begin(), vv.end());
        for (size_t k = 0; k + 1 < vv.size(); ++k) {
            const double t0 = vv[k], t1 = vv[k + 1];
            if ((t1 - t0) * len <= q) continue;
            const double tm = 0.5 * (t0 + t1);
            const Point<double> m(a.x + abx * tm, a.y + aby * tm);
            const double nx = -aby / len, ny = abx / len;
            const Point<double> pl(m.x + nx * off, m.y + ny * off);
            const Point<double> pr(m.x - nx * off, m.y - ny * off);
            const bool fL = fOp(parityIn(pl, A), parityIn(pl, B));
            const bool fR = fOp(parityIn(pr, A), parityIn(pr, B));
            if (fL == fR) continue;
            const Point<double> pa(a.x + abx * t0, a.y + aby * t0);
            const Point<double> pb(a.x + abx * t1, a.y + aby * t1);
            if (fL) DE.push_back({idOf(pa), idOf(pb)});
            else    DE.push_back({idOf(pb), idOf(pa)});
        }
    }
    if (DE.empty()) return empty;

    std::map<int, std::vector<int>> out;
    for (size_t i = 0; i < DE.size(); ++i) out[DE[i].u].push_back(static_cast<int>(i));
    std::vector<char> used(DE.size(), 0);
    QVector<QVector<Point<double>>> contours;
    const double TWO_PI = 6.2831853071795864769;
    for (size_t s = 0; s < DE.size(); ++s) {
        if (used[s]) continue;
        if (out.find(DE[s].v) == out.end()) { used[s] = 1; continue; }
        QVector<Point<double>> ctr;
        ctr.push_back(P[DE[s].u]);
        int cur = static_cast<int>(s);
        used[cur] = 1;
        const int guard = static_cast<int>(DE.size()) + 2;
        for (int step = 0; step < guard; ++step) {
            const int v = DE[cur].v;
            ctr.push_back(P[v]);
            if (v == DE[s].u) break;
            const double rx = P[DE[cur].u].x - P[v].x;
            const double ry = P[DE[cur].u].y - P[v].y;
            int best = -1;
            double bestCw = 1e18;
            for (int ei : out[v]) {
                if (used[ei]) continue;
                const double cx = P[DE[ei].v].x - P[v].x;
                const double cy = P[DE[ei].v].y - P[v].y;
                const double th = std::atan2(rx * cy - ry * cx, rx * cx + ry * cy);
                const double cw = (th <= 0.0) ? -th : TWO_PI - th;
                if (cw < bestCw) { bestCw = cw; best = ei; }
            }
            if (best < 0) break;
            cur = best;
            used[cur] = 1;
        }
        if (ctr.size() >= 3) contours.push_back(ctr);
    }

    QPainterPath path;
    for (const auto& c : contours) {
        path.moveTo(c[0].toQPointF());
        for (int i = 1; i < c.size(); ++i) path.lineTo(c[i].toQPointF());
        path.closeSubpath();
    }
    return path;
}
} // namespace

QStringList ArbitraryPolygonOpsTask::operationOptions() const
{
    return QStringList{
        QStringLiteral("Пересечение (A ∩ B)"),
        QStringLiteral("Объединение (A ∪ B)"),
        QStringLiteral("Разность (A \\ B)"),
        QStringLiteral("Симметрическая разность (A XOR B)")
    };
}

void ArbitraryPolygonOpsTask::setOperation(int index)
{
    if (index >= 0 && index <= 3) m_operationMode = index;
}

int ArbitraryPolygonOpsTask::totalA() const
{
    int s = m_openA;
    for (int v : m_closedA) s += v;
    return s;
}

int ArbitraryPolygonOpsTask::totalB() const
{
    int s = m_openB;
    for (int v : m_closedB) s += v;
    return s;
}

int& ArbitraryPolygonOpsTask::currentOpen()
{
    return (m_phase == Phase::InputB) ? m_openB : m_openA;
}

QVector<int>& ArbitraryPolygonOpsTask::currentClosed()
{
    return (m_phase == Phase::InputB) ? m_closedB : m_closedA;
}

void ArbitraryPolygonOpsTask::finishPolygon()
{
    if (m_phase == Phase::InputA) {
        if (m_openA >= 3) { m_closedA.push_back(m_openA); m_openA = 0; }
        m_phase = Phase::InputB;
    } else if (m_phase == Phase::InputB) {
        if (m_openB >= 3) { m_closedB.push_back(m_openB); m_openB = 0; }
        m_phase = Phase::Done;
    }
}

void ArbitraryPolygonOpsTask::resetState()
{
    m_phase = Phase::InputA;
    m_closedA.clear();
    m_closedB.clear();
    m_openA = 0;
    m_openB = 0;
    m_lastPressAddedPoint = false;
}

void ArbitraryPolygonOpsTask::sanitizeState(const QVector<Point<Quotient_Number>>& points)
{
    if (totalA() + totalB() != points.size()) {
        m_closedA.clear();
        m_closedB.clear();
        m_openA = 0;
        m_openB = 0;
        m_phase = Phase::InputA;
        if (points.size() >= 3) m_closedA.push_back(points.size());
        else m_openA = points.size();
    }
    m_lastPressAddedPoint = false;
}

bool ArbitraryPolygonOpsTask::deletePointAt(QVector<Point<Quotient_Number>>& points, int idx)
{
    if (idx < 0 || idx >= points.size()) return false;
    auto dec = [&](QVector<int>& closed, int& open, int local) {
        int pos = 0;
        for (int k = 0; k < closed.size(); ++k) {
            if (local < pos + closed[k]) {
                closed[k]--;
                if (closed[k] <= 0) closed.removeAt(k);
                return;
            }
            pos += closed[k];
        }
        if (open > 0) open--;
    };
    const int totA = totalA();
    if (idx < totA) dec(m_closedA, m_openA, idx);
    else dec(m_closedB, m_openB, idx - totA);
    points.removeAt(idx);
    return true;
}

void ArbitraryPolygonOpsTask::collectContours(const QVector<Point<Quotient_Number>>& points,
                                              QVector<ContourD>& outA,
                                              QVector<ContourD>& outB) const
{
    outA.clear();
    outB.clear();
    int pos = 0;
    for (int sz : m_closedA) {
        if (sz >= 3 && pos + sz <= points.size()) {
            ContourD c;
            for (int i = 0; i < sz; ++i) c.push_back(points[pos + i].toDoublePoint());
            outA.push_back(c);
        }
        pos += sz;
    }
    pos += m_openA;
    for (int sz : m_closedB) {
        if (sz >= 3 && pos + sz <= points.size()) {
            ContourD c;
            for (int i = 0; i < sz; ++i) c.push_back(points[pos + i].toDoublePoint());
            outB.push_back(c);
        }
        pos += sz;
    }
}

void ArbitraryPolygonOpsTask::handleMousePress(QMouseEvent* event,
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
                int& op = currentOpen();
                if (op > 0) op--;
                m_lastPressAddedPoint = false;
            }
            int& op = currentOpen();
            QVector<int>& cl = currentClosed();
            if (op >= 3) {
                cl.push_back(op);
                op = 0;
            } else {
                op = 0;
                if (!cl.isEmpty()) finishPolygon();
            }
            return;
        }
        const int idx = pointIndexAt(event->pos(), points);
        if (idx != -1) {
            draggingIndex = idx;
            m_lastPressAddedPoint = false;
            return;
        }
        if (m_phase == Phase::Done) return;
        points.push_back(makePointFromScene(event->pos()));
        currentOpen()++;
        m_lastPressAddedPoint = true;
        return;
    }
    if (event->button() == Qt::RightButton) {
        const int idx = pointIndexAt(event->pos(), points);
        if (idx == -1) return;
        deletePointAt(points, idx);
        draggingIndex = -1;
        hoveredIndex = -1;
        return;
    }
    StandardPointTask::handleMousePress(event, points, draggingIndex, hoveredIndex);
}

QPointF ArbitraryPolygonOpsTask::sceneToScreen(const QPointF& scene) const
{
    return QPointF(scene.x() * m_lastScale + m_lastOffsetX,
                   m_lastOffsetY - scene.y() * m_lastScale);
}

void ArbitraryPolygonOpsTask::applyAction(int action, QVector<Point<Quotient_Number>>& points)
{
    Q_UNUSED(action);
    if (m_phase == Phase::Done) {
        resetState();
        points.clear();
    } else {
        finishPolygon();
    }
}

void ArbitraryPolygonOpsTask::handleMouseMove(QMouseEvent* event,
                                              QVector<Point<Quotient_Number>>& points,
                                              int& draggingIndex,
                                              int& hoveredIndex,
                                              QWidget* widget)
{
    m_cursor = Point<double>(event->pos().x(), event->pos().y());
    m_hasCursor = true;
    StandardPointTask::handleMouseMove(event, points, draggingIndex, hoveredIndex, widget);
}

void ArbitraryPolygonOpsTask::handleKeyPress(QKeyEvent* event,
                                             QVector<Point<Quotient_Number>>& points,
                                             int& draggingIndex,
                                             int& hoveredIndex)
{
    Q_UNUSED(draggingIndex);
    Q_UNUSED(hoveredIndex);
    const int key = event->key();
    if (key == Qt::Key_Return || key == Qt::Key_Enter || key == Qt::Key_Space) {
        applyAction(0, points);
        event->accept();
        return;
    }
    if (key == Qt::Key_Backspace && m_phase != Phase::Done) {
        int& op = currentOpen();
        if (op > 0 && !points.isEmpty()) {
            points.removeLast();
            op--;
            m_lastPressAddedPoint = false;
            event->accept();
            return;
        }
    }
    event->ignore();
}

void ArbitraryPolygonOpsTask::draw(QPainter& p, const QRect& rect,
                                   const QVector<Point<Quotient_Number>>& points,
                                   int draggingIndex, int hoveredIndex,
                                   double offsetX, double offsetY, double scale)
{
    sanitizeState(points);
    drawCoordinateGrid(p, rect, offsetX, offsetY, scale);
    const double invScale = (scale > 1e-12) ? (1.0 / scale) : 1.0;
    m_lastOffsetX = offsetX; m_lastOffsetY = offsetY; m_lastScale = scale;

    QVector<ContourD> contA, contB;
    collectContours(points, contA, contB);
    const int totA = totalA();

    auto buildPath = [](const QVector<ContourD>& cs) {
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
    p.scale(scale, -scale);

    const QPainterPath pathA = buildPath(contA);
    const QPainterPath pathB = buildPath(contB);

    if (!pathA.isEmpty()) {
        p.fillPath(pathA, QBrush(QColor(100, 150, 255, 45)));
        p.setPen(QPen(QColor(50, 100, 200), 2.0 * invScale));
        p.setBrush(Qt::NoBrush);
        p.drawPath(pathA);
    }
    if (!pathB.isEmpty()) {
        p.fillPath(pathB, QBrush(QColor(255, 150, 100, 45)));
        p.setPen(QPen(QColor(200, 100, 50), 2.0 * invScale));
        p.setBrush(Qt::NoBrush);
        p.drawPath(pathB);
    }
    auto drawOpen = [&](int offset, int count, const QColor& col) {
        if (count < 2) return;
        p.setPen(QPen(col, 2.0 * invScale, Qt::DashLine));
        for (int i = offset + 1; i < offset + count; ++i) {
            p.drawLine(points[i - 1].toQPointF(), points[i].toQPointF());
        }
    };
    if (m_phase == Phase::InputA) drawOpen(totA - m_openA, m_openA, QColor(100, 150, 255));
    if (m_phase == Phase::InputB) drawOpen(points.size() - m_openB, m_openB, QColor(255, 150, 100));
    if (m_hasCursor && m_phase != Phase::Done && !points.isEmpty()) {
        const int open = (m_phase == Phase::InputB) ? m_openB : m_openA;
        if (open >= 1) {
            const int base = points.size() - open;
            p.setPen(QPen(QColor(90, 90, 90), 1.5 * invScale, Qt::DashLine));
            p.drawLine(points[points.size() - 1].toQPointF(), m_cursor.toQPointF());
            if (open >= 2) {
                p.drawLine(m_cursor.toQPointF(), points[base].toQPointF());
            }
        }
    }

    if (m_phase == Phase::Done) {
        const QPainterPath res = computeResultPath(contA, contB, m_operationMode);
        if (!res.isEmpty()) {
            p.fillPath(res, QBrush(QColor(200, 60, 60, 150)));
            p.setPen(QPen(QColor(140, 30, 30), 2.0 * invScale));
            p.setBrush(Qt::NoBrush);
            p.drawPath(res);
        }
    }

    for (int i = 0; i < points.size(); ++i) {
        const Point<double> pt = points[i].toDoublePoint();
        const bool isActive = (i == draggingIndex || i == hoveredIndex);
        const QColor c = (i < totA || m_phase == Phase::InputA) ? QColor(100, 150, 255) : QColor(255, 150, 100);
        p.setBrush(c);
        p.setPen(isActive ? QPen(QColor(255, 200, 0), 3.0 * invScale) : QPen(Qt::black, 1.5 * invScale));
        p.drawEllipse(pt.toQPointF(), m_pointRadius * invScale, m_pointRadius * invScale);
    }
    p.restore();

    p.save();
    p.resetTransform();
    if (points.size() <= 60) {
        p.setFont(QFont("Arial", 9));
        p.setPen(QColor(60, 60, 60));
        for (int i = 0; i < points.size(); ++i) {
            const Point<double> pt = points[i].toDoublePoint();
            const QPointF screen(pt.x * scale + offsetX, offsetY - pt.y * scale);
            p.drawText(screen + QPointF(10.0, -10.0), QString::number(i));
        }
    }
    p.setFont(QFont("Arial", 12, QFont::Bold));
    p.setPen(Qt::black);
    m_buttons.clear();
    {
        QString label;
        if (m_phase == Phase::InputA) label = QStringLiteral("Завершить многоугольник A -> ввод B [Enter]");
        else if (m_phase == Phase::InputB) label = QStringLiteral("Завершить многоугольник B -> результат [Enter]");
        else label = QStringLiteral("Начать заново [Enter]");
        const QRect btnRect(rect.right() - 300, rect.top() + 12, 288, 36);
        p.setBrush(QColor(235, 245, 235));
        p.setPen(QPen(QColor(60, 140, 60), 2));
        p.drawRoundedRect(btnRect, 6, 6);
        p.setPen(QColor(20, 90, 20));
        p.setFont(QFont("Arial", 10, QFont::Bold));
        p.drawText(btnRect, Qt::AlignCenter, label);
        m_buttons.push_back(ScreenButton{btnRect, 0});
    }
    QString status;
    if (m_phase == Phase::InputA) {
        status = QStringLiteral("Многоугольник A: контуров %1, в текущем контуре вершин %2; клик - вершина, двойной клик - замкнуть контур, Enter/кнопка - завершить A")
            .arg(m_closedA.size()).arg(m_openA);
    } else if (m_phase == Phase::InputB) {
        status = QStringLiteral("Многоугольник B: контуров %1, в текущем контуре вершин %2; Enter/кнопка - результат")
            .arg(m_closedB.size()).arg(m_openB);
    } else {
        status = QStringLiteral("Результат готов: перетаскивание работает; Enter/кнопка - начать заново");
    }
    QString opName;
    if (m_operationMode == 0) opName = QStringLiteral("A ∩ B");
    else if (m_operationMode == 1) opName = QStringLiteral("A ∪ B");
    else if (m_operationMode == 2) opName = QStringLiteral("A \\ B");
    else opName = QStringLiteral("A XOR B");
    p.drawText(20, 30, status + QStringLiteral(" | Операция: ") + opName);
    p.setFont(QFont("Arial", 9));
    p.setPen(Qt::darkGray);
    p.drawText(20, 48, QStringLiteral("Правило заполнения: чёт/нечет (дырки и самопересечения учитываются)"));
    p.drawText(20, 64, QStringLiteral("Backspace - отменить последнюю вершину; дырка = новый контур внутри многоугольника;"));
    p.restore();
}
