#include "MainWindow.h"
#include "ControlPanel.h"
#include "CanvasWidget.h"
#include <QSplitter>
#include <QKeyEvent>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle(QStringLiteral("Геометрические задачи"));
    resize(1000, 700);
    
    m_splitter = new QSplitter(Qt::Horizontal, this);
    setCentralWidget(m_splitter);
    
    m_controlPanel = new ControlPanel(this);
    m_splitter->addWidget(m_controlPanel);
    
    m_canvas = new CanvasWidget(this);
    m_splitter->addWidget(m_canvas);
    
    m_splitter->setSizes({250, 750});
    m_splitter->setStretchFactor(0, 0);
    m_splitter->setStretchFactor(1, 1);
    
    QStringList taskNames;
    for (const auto& task : m_taskManager.tasks()) taskNames << task->name();
    m_controlPanel->setTaskNames(taskNames);
    
    if (!m_taskManager.tasks().isEmpty()) m_canvas->setCurrentTask(m_taskManager.getTask(0));
    
    connect(m_controlPanel, &ControlPanel::taskChanged, this, &MainWindow::onTaskChanged);
    connect(m_controlPanel, &ControlPanel::pointRadiusChanged, m_canvas, &CanvasWidget::setPointRadius);
    connect(m_controlPanel, &ControlPanel::epsilonChanged, m_canvas, &CanvasWidget::setEpsilon);
    connect(m_controlPanel, &ControlPanel::showEpsilonToggled, m_canvas, &CanvasWidget::setShowEpsilonNeighborhood);
    connect(m_controlPanel, &ControlPanel::clearRequested, m_canvas, &CanvasWidget::clearPoints);
    connect(m_controlPanel, &ControlPanel::resetCameraRequested, m_canvas, &CanvasWidget::resetCamera);
    
    setFocusPolicy(Qt::StrongFocus);
}

void MainWindow::onTaskChanged(int index) {
    auto task = m_taskManager.getTask(index);
    if (task) { m_canvas->setCurrentTask(task); m_canvas->clearPoints(); }
}

void MainWindow::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_C || event->key() == Qt::Key_Escape) m_canvas->clearPoints();
    else if (event->key() == Qt::Key_Home) m_canvas->resetCamera();
    else if (event->key() == Qt::Key_Tab) {
        m_panelVisible = !m_panelVisible;
        m_controlPanel->setVisible(m_panelVisible);
    }
    QMainWindow::keyPressEvent(event);
}
