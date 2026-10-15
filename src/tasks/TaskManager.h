#pragma once
#include <QVector>
#include <memory>
#include "Task.h"
#include "OrientationTask.h"
#include "SegmentIntersectionTask.h"

class TaskManager {
public:
    TaskManager() {
        m_tasks.push_back(std::make_shared<OrientationTask>());
        m_tasks.push_back(std::make_shared<SegmentIntersectionTask>());
    }
    QVector<std::shared_ptr<Task>>& tasks() { return m_tasks; }
    const QVector<std::shared_ptr<Task>>& tasks() const { return m_tasks; }
    std::shared_ptr<Task> getTask(int index) const {
        if (index >= 0 && index < m_tasks.size()) return m_tasks[index];
        return nullptr;
    }
private:
    QVector<std::shared_ptr<Task>> m_tasks;
};
