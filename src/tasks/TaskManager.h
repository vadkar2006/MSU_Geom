#pragma once
#include <QVector>
#include <memory>
#include "Task.h"
#include "OrientationTask.h"
#include "SegmentIntersectionTask.h"
#include "ConvexHullTask.h"
#include "PointInConvexPolygonTask.h"
#include "DelaunayTask.h"
#include "ConvexSetOpsTask.h"
#include "ArbitraryPolygonOpsTask.h"
#include "PointInArbitraryPolygonTask.h"

class TaskManager {
public:
    TaskManager() {
        m_tasks.push_back(std::make_shared<OrientationTask>());
        m_tasks.push_back(std::make_shared<SegmentIntersectionTask>());
        m_tasks.push_back(std::make_shared<ConvexHullTask>());
        m_tasks.push_back(std::make_shared<PointInConvexPolygonTask>());
        m_tasks.push_back(std::make_shared<DelaunayTask>());
        m_tasks.push_back(std::make_shared<ConvexSetOpsTask>());
        m_tasks.push_back(std::make_shared<ArbitraryPolygonOpsTask>());
        m_tasks.push_back(std::make_shared<PointInArbitraryPolygonTask>());
    }
    QVector<std::shared_ptr<GeomTask>>& tasks() { return m_tasks; }
    const QVector<std::shared_ptr<GeomTask>>& tasks() const { return m_tasks; }
    std::shared_ptr<GeomTask> getTask(int index) const {
        if (index >= 0 && index < m_tasks.size()) return m_tasks[index];
        return nullptr;
    }
private:
    QVector<std::shared_ptr<GeomTask>> m_tasks;
};
