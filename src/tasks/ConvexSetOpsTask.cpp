#include "ConvexSetOpsTask.h"
#include "Geometry.h"
#include <QPainter>
#include <QMouseEvent>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QPolygonF>
#include <QColor>
#include <cmath>
#include <vector>
#include <algorithm>

ConvexSetOpsTask::ConvexSetOpsTask() {

}

QStringList ConvexSetOpsTask::operationOptions() const {
    return QStringList{
        QStringLiteral("Пересечение (A ∩ B)"),
        QStringLiteral("Объединение (A ∪ B)"),
        QStringLiteral("Разность (A \\ B)"),
        QStringLiteral("Симметрическая разность (A XOR B)")
    };
}

void ConvexSetOpsTask::setOperation(int index) {
    if (index >= 0 && index <= 3) {
        m_operationMode = index;
    }
}

void ConvexSetOpsTask::resetState() {
    m_phase = Phase::InputA;
    m_countA = 0;
}

bool ConvexSetOpsTask::isPointInConvexPoly(const Point<double>& p, const QVector<Point<double>>& poly, double eps) const {
    if (poly.size() < 3) return false;
    int res = predicate::orientationConvexHull(poly, p, eps);
    return res >= 0;
}

QVector<int> ConvexSetOpsTask::hullIndices(const QVector<Point<Quotient_Number>>& pts,
                                           bool exact, double epsilon) {
    if (pts.size() < 3) {
        QVector<int> res;
        for (int i = 0; i < pts.size(); ++i) res.push_back(i);
        return res;
    }
    if (exact) return convex_hull::algorithm_Graham(pts, Quotient_Number(0));
    return convex_hull::algorithm_Graham(toDoubleVector(pts), epsilon);
}

void ConvexSetOpsTask::finalizePolygonA(QVector<Point<Quotient_Number>>& points) {
    if (points.size() < 3) return;
    m_countA = points.size();
    m_phase = Phase::InputB;
}

void ConvexSetOpsTask::finalizePolygonB(QVector<Point<Quotient_Number>>& points) {
    Q_UNUSED(points);
    m_phase = Phase::Done;
}

void ConvexSetOpsTask::handleMousePress(QMouseEvent* event, QVector<Point<Quotient_Number>>& points, int& draggingIndex, int& hoveredIndex) {
    if (event->button() == Qt::LeftButton) {
        if (event->type() == QEvent::MouseButtonDblClick) {
            if (m_phase == Phase::InputA) {
                finalizePolygonA(points);
                return;
            } else if (m_phase == Phase::InputB) {
                finalizePolygonB(points);
                return;
            } else if (m_phase == Phase::Done) {
                resetState();
                points.clear();
                return;
            }
        }
        
        if (m_phase == Phase::Done) {
            const int idx = pointIndexAt(event->pos(), points);
            if (idx == -1) return;
        }
        
        StandardPointTask::handleMousePress(event, points, draggingIndex, hoveredIndex);
    } else if (event->button() == Qt::RightButton) {
        const int idx = pointIndexAt(event->pos(), points);
        if (idx != -1) {
            points.removeAt(idx);
            if (idx < m_countA) m_countA--;
            draggingIndex = -1;
            hoveredIndex = -1;
        }
    }
}

void ConvexSetOpsTask::handleMouseRelease(QMouseEvent* event, QVector<Point<Quotient_Number>>& points, int& draggingIndex) {
    StandardPointTask::handleMouseRelease(event, points, draggingIndex);
}

void ConvexSetOpsTask::draw(QPainter& p, const QRect& rect, const QVector<Point<Quotient_Number>>& points,
                            int draggingIndex, int hoveredIndex, double offsetX, double offsetY, double scale) 
{
    drawCoordinateGrid(p, rect, offsetX, offsetY, scale);
    const double invScale = (scale > 1e-12) ? (1.0 / scale) : 1.0;
    const int countA = (m_phase == Phase::InputA) ? points.size() : m_countA;
    const int posA = (countA < 0) ? 0 : ((countA > points.size()) ? points.size() : countA);
    const QVector<Point<Quotient_Number>> srcA = points.mid(0, posA);
    const QVector<Point<Quotient_Number>> srcB = points.mid(posA);
    QVector<Point<Quotient_Number>> hullA;
    QVector<Point<Quotient_Number>> hullB;
    {
        const QVector<int> hi = hullIndices(srcA, useBigNum(), m_epsilon);
        for (int idx : hi) hullA.push_back(srcA[idx]);
        if (m_phase != Phase::InputA) {
            const QVector<int> hj = hullIndices(srcB, useBigNum(), m_epsilon);
            for (int idx : hj) hullB.push_back(srcB[idx]);
        }
    }
    
    p.save();
    p.translate(offsetX, offsetY);
    p.scale(scale, -scale);

    if (m_phase == Phase::InputA) {
        p.setPen(QPen(QColor(100, 150, 255), 2.0 * invScale, Qt::DashLine));
        for (int i = 1; i < points.size(); ++i) {
            p.drawLine(points[i-1].toQPointF(), points[i].toQPointF());
        }
    } else if (m_phase == Phase::InputB) {
        if (hullA.size() >= 3) {
            QPolygonF polyA;
            for (const auto& pt : hullA) polyA << pt.toQPointF();
            p.setPen(Qt::NoPen);
            p.setBrush(QColor(100, 150, 255, 50));
            p.drawPolygon(polyA);
            p.setPen(QPen(QColor(50, 100, 200), 2.0 * invScale));
            p.setBrush(Qt::NoBrush);
            p.drawPolygon(polyA);
        }
        p.setPen(QPen(QColor(255, 150, 100), 2.0 * invScale, Qt::DashLine));
        for (int i = m_countA + 1; i < points.size(); ++i) {
            p.drawLine(points[i-1].toQPointF(), points[i].toQPointF());
        }
    }

    if (m_phase == Phase::Done && hullA.size() >= 3 && hullB.size() >= 3) {
        std::vector<Point<double>> allPts;
        for (const auto& pt : hullA) allPts.push_back(pt.toDoublePoint());
        int countA = hullA.size();
        for (const auto& pt : hullB) allPts.push_back(pt.toDoublePoint());
        int countB = hullB.size();
        int startB = countA;


        std::vector<std::pair<int, int>> edgesA, edgesB;
        for(int i=0; i<countA; ++i) edgesA.push_back({i, (i+1)%countA});
        for(int i=0; i<countB; ++i) edgesB.push_back({startB+i, startB+(i+1)%countB});


        
        struct NewPoint {
            int edgeIdxA;
            int edgeIdxB;
            double t;
            int ptIdx;
        };
        
        struct Intersection {
            int originalEdgeIdx; // индекс в исходном списке edgesA или edgesB
            bool isA;
            double t;
            int ptIdx;
        };
        std::vector<Intersection> intersections;

        for (size_t i = 0; i < edgesA.size(); ++i) {
            int ia1 = edgesA[i].first;
            int ia2 = edgesA[i].second;
            Point<double> a1 = allPts[ia1];
            Point<double> a2 = allPts[ia2];

            for (size_t j = 0; j < edgesB.size(); ++j) {
                int ib1 = edgesB[j].first;
                int ib2 = edgesB[j].second;
                Point<double> b1 = allPts[ib1];
                Point<double> b2 = allPts[ib2];


                int o1 = predicate::orientation(a1, a2, b1, 1e-9);
                int o2 = predicate::orientation(a1, a2, b2, 1e-9);
                int o3 = predicate::orientation(b1, b2, a1, 1e-9);
                int o4 = predicate::orientation(b1, b2, a2, 1e-9);

                bool intersects = false;
                if (o1 != o2 && o3 != o4) intersects = true;
                
                if (intersects) {
                    try {
                        Point<double> ip = segment_intersection::intersection_line(a1, a2, b1, b2, 1e-9);
                        
                        int existingIdx = -1;
                        for(size_t k=0; k<allPts.size(); ++k) {
                            double dx = allPts[k].x - ip.x;
                            double dy = allPts[k].y - ip.y;
                            if (dx*dx + dy*dy < 1e-12) {
                                existingIdx = (int)k;
                                break;
                            }
                        }
                        
                        if (existingIdx == -1) {
                            existingIdx = (int)allPts.size();
                            allPts.push_back(ip);
                        }
                        
                        double dx = a2.x - a1.x;
                        double dy = a2.y - a1.y;
                        double len2 = dx*dx + dy*dy;
                        double tA = (len2 > 1e-12) ? ((ip.x - a1.x)*dx + (ip.y - a1.y)*dy) / len2 : 0.0;
                        
                        double dx2 = b2.x - b1.x;
                        double dy2 = b2.y - b1.y;
                        double len2_2 = dx2*dx2 + dy2*dy2;
                        double tB = (len2_2 > 1e-12) ? ((ip.x - b1.x)*dx2 + (ip.y - b1.y)*dy2) / len2_2 : 0.0;

                        intersections.push_back({(int)i, true, tA, existingIdx});
                        intersections.push_back({(int)j, false, tB, existingIdx});
                        
                    } catch (...) {}
                }
            }
        }


        std::vector<std::vector<int>> chainA(countA);
        for(int i=0; i<countA; ++i) {
            chainA[i].push_back(edgesA[i].first);
        }
        
        std::vector<std::vector<int>> chainB(countB);
        for(int i=0; i<countB; ++i) {
            chainB[i].push_back(edgesB[i].first);
        }

        for (const auto& intr : intersections) {
            if (intr.isA) {
                chainA[intr.originalEdgeIdx].push_back(intr.ptIdx);
            } else {
                chainB[intr.originalEdgeIdx].push_back(intr.ptIdx);
            }
        }

        auto sortChain = [&](std::vector<std::vector<int>>& chains, const std::vector<std::pair<int,int>>& originalEdges) {
            for(size_t i=0; i<chains.size(); ++i) {
                if (chains[i].size() <= 2) continue;
                int u = originalEdges[i].first;
                int v = originalEdges[i].second;
                Point<double> p1 = allPts[u];
                Point<double> p2 = allPts[v];
                double dx = p2.x - p1.x;
                double dy = p2.y - p1.y;
                
                std::sort(chains[i].begin(), chains[i].end(), [&](int idx1, int idx2) {
                    if (idx1 == u) return true;
                    if (idx2 == u) return false;
                    double t1 = (allPts[idx1].x - p1.x)*dx + (allPts[idx1].y - p1.y)*dy;
                    double t2 = (allPts[idx2].x - p1.x)*dx + (allPts[idx2].y - p1.y)*dy;
                    return t1 < t2;
                });
            }
        };
        
        sortChain(chainA, edgesA);
        sortChain(chainB, edgesB);

        QVector<std::pair<int, int>> constraints;
        for (const auto& chain : chainA) {
            for (size_t i = 0; i + 1 < chain.size(); ++i) {
                constraints.push_back({chain[i], chain[i+1]});
            }
        }
        for (const auto& chain : chainB) {
            for (size_t i = 0; i + 1 < chain.size(); ++i) {
                constraints.push_back({chain[i], chain[i+1]});
            }
        }

        // 3. Triangulate
        QVector<Point<double>> qAllPts;
        for(const auto& pt : allPts) qAllPts.push_back(pt);
        
        QVector<delaunay::Triangle> tris = delaunay::triangulateConstrained(qAllPts, constraints, 1e-9);

        // 4. Classification & Drawing
        QVector<Point<double>> hullA_d, hullB_d;
        for(const auto& p : hullA) hullA_d.push_back(p.toDoublePoint());
        for(const auto& p : hullB) hullB_d.push_back(p.toDoublePoint());

        for (const auto& t : tris) {
            if (t.a < 0 || t.a >= (int)allPts.size() || t.b < 0 || t.b >= (int)allPts.size() || t.c < 0 || t.c >= (int)allPts.size()) continue;
            
            Point<double> pa = allPts[t.a];
            Point<double> pb = allPts[t.b];
            Point<double> pc = allPts[t.c];
            
            Point<double> center((pa.x + pb.x + pc.x) / 3.0, (pa.y + pb.y + pc.y) / 3.0);
            
            bool inA = isPointInConvexPoly(center, hullA_d, 1e-9);
            bool inB = isPointInConvexPoly(center, hullB_d, 1e-9);
            
            bool draw = false;
            QColor color(200, 50, 50, 180);
            
            if (m_operationMode == 0) {
                if (inA && inB) draw = true;
            } else if (m_operationMode == 1) {
                if (inA || inB) draw = true;
            } else if (m_operationMode == 2) {
                if (inA && !inB) draw = true;
            } else if (m_operationMode == 3) {
                if (inA != inB) draw = true;
            }
            
            if (draw) {
                QPolygonF poly;
                poly << pa.toQPointF() << pb.toQPointF() << pc.toQPointF();
                p.setPen(Qt::NoPen);
                p.setBrush(color);
                p.drawPolygon(poly);
            }
        }
        
        p.setPen(QPen(QColor(50, 100, 200), 2.0 * invScale));
        p.setBrush(Qt::NoBrush);
        QPolygonF polyA;
        for (const auto& pt : hullA) polyA << pt.toQPointF();
        p.drawPolygon(polyA);
        
        p.setPen(QPen(QColor(200, 100, 50), 2.0 * invScale));
        QPolygonF polyB;
        for (const auto& pt : hullB) polyB << pt.toQPointF();
        p.drawPolygon(polyB);
    }

    for (int i = 0; i < points.size(); ++i) {
        const Point<double> pt = points[i].toDoublePoint();
        bool isActive = (i == draggingIndex || i == hoveredIndex);
        QColor c = (i < m_countA || m_phase == Phase::InputA) ? QColor(100, 150, 255) : QColor(255, 150, 100);
        
        p.setBrush(c);
        p.setPen(isActive ? QPen(QColor(255, 200, 0), 3.0 * invScale) : QPen(Qt::black, 1.5 * invScale));
        p.drawEllipse(pt.toQPointF(), m_pointRadius * invScale, m_pointRadius * invScale);
    }

    p.restore();


    p.save();
    p.resetTransform();
    p.setFont(QFont("Arial", 12, QFont::Bold));
    p.setPen(Qt::black);
    
    QString status;
    if (m_phase == Phase::InputA) status = "Ввод A: ЛКМ - точка, перетаскивание, двойной ЛКМ - готово";
    else if (m_phase == Phase::InputB) status = "Ввод B: ЛКМ - точка, перетаскивание, двойной ЛКМ - результат";
    else status = "Результат: точки можно перетаскивать, двойной ЛКМ - сброс";
    

    QString opName;
    if (m_operationMode == 0) opName = "A ∩ B";
    else if (m_operationMode == 1) opName = "A ∪ B";
    else if (m_operationMode == 2) opName = "A \\ B";
    else opName = "A XOR B";
    
    p.drawText(20, 30, status + " | Операция: " + opName);
    p.restore();
}
