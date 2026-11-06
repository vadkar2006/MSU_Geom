
#include "MainWindow.h"
#include <QString>
#include "ControlPanel.h"
#include "CanvasWidget.h"
#include "PointsDialog.h"

#include <QSplitter>
#include <QKeyEvent>
#include <QApplication>
#include <QLineEdit>
#include <QTableWidget>
#include <QTextEdit>
#include <QComboBox>
#include "Quotient_Number.h"

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
connect(m_controlPanel, &ControlPanel::useBigNumChanged,
                m_canvas, &CanvasWidget::setUseBigNum);
    m_splitter->setStretchFactor(0, 0);
    m_splitter->setStretchFactor(1, 1);
    
    QStringList taskNames;
    for (const auto& task : m_taskManager.tasks()) taskNames << task->name();
    m_controlPanel->setTaskNames(taskNames);
    
    if (!m_taskManager.tasks().isEmpty()) {
        m_canvas->setCurrentTask(m_taskManager.getTask(0));
        m_canvas->setPointRadius(m_controlPanel->pointRadius());
        m_canvas->setEpsilon(m_controlPanel->epsilon());
        m_canvas->setShowEpsilonNeighborhood(m_controlPanel->showEpsilonNeighborhood());
    m_controlPanel->setOperationOptions(m_taskManager.getTask(0)->operationOptions(),
                                        m_taskManager.getTask(0)->currentOperation());
    }
    
    connect(m_controlPanel, &ControlPanel::taskChanged, this, &MainWindow::onTaskChanged);
    connect(m_controlPanel, &ControlPanel::operationChanged, this, [this](int index) {
        auto task = m_taskManager.getTask(m_currentTaskIndex);
        if (task) {
            task->setOperation(index);
            m_canvas->update();
        }
    });
    connect(m_controlPanel, &ControlPanel::pointRadiusChanged, m_canvas, &CanvasWidget::setPointRadius);
    connect(m_controlPanel, &ControlPanel::epsilonChanged, m_canvas, &CanvasWidget::setEpsilon);
    connect(m_controlPanel, &ControlPanel::showEpsilonToggled, m_canvas, &CanvasWidget::setShowEpsilonNeighborhood);
    connect(m_controlPanel, &ControlPanel::clearRequested, m_canvas, &CanvasWidget::clearPoints);
    connect(m_controlPanel, &ControlPanel::resetCameraRequested, m_canvas, &CanvasWidget::resetCamera);
connect(m_controlPanel, &ControlPanel::pointInputRequested, m_canvas, &CanvasWidget::addPointFromInput);
    connect(m_controlPanel, &ControlPanel::outputPrecisionChanged, this, [this](int digits) {
        Quotient_Number::Set_Precision(digits);

        if (m_canvas) {
            m_canvas->update();
        }

        if (m_pointsDialog && m_canvas) {
            m_pointsDialog->notifyCanvasPointsChanged(m_canvas->points());
        }
    });

connect(m_canvas, &CanvasWidget::coordinateInputApplied, m_controlPanel, &ControlPanel::setCoordinateInputs);
    
    
setFocusPolicy(Qt::StrongFocus);

    m_pointsDialog = new PointsDialog(this);
    m_pointsDialog->setWindowTitle(QStringLiteral("Точки"));

    connect(m_controlPanel, &ControlPanel::showPointsRequested,
            this, &MainWindow::showPointsDialog);

    connect(m_canvas, &CanvasWidget::pointsChanged, this, [this]() {
        if (m_pointsDialog) {
            m_pointsDialog->notifyCanvasPointsChanged(m_canvas->points());
        }
    });

    connect(m_pointsDialog, &PointsDialog::applyPointsRequested,
        this, &MainWindow::onPointsDialogApplyRequested);
}

void MainWindow::onTaskChanged(int index) {
    auto task = m_taskManager.getTask(index);
        m_currentTaskIndex = index;
    if (task) {
        m_canvas->setCurrentTask(task);
        m_canvas->setPointRadius(m_controlPanel->pointRadius());
        m_canvas->setEpsilon(m_controlPanel->epsilon());
        m_canvas->setShowEpsilonNeighborhood(m_controlPanel->showEpsilonNeighborhood());
        m_controlPanel->setOperationOptions(task->operationOptions(), task->currentOperation());
        m_canvas->clearPoints();
    }
}

void MainWindow::keyPressEvent(QKeyEvent* event) {
    QWidget* focused = QApplication::focusWidget();

    const bool editing =
        qobject_cast<QLineEdit*>(focused) ||
        qobject_cast<QTableWidget*>(focused) ||
        qobject_cast<QTextEdit*>(focused) ||
        qobject_cast<QComboBox*>(focused);

    if (!editing) {
        if (event->key() == Qt::Key_C || event->key() == Qt::Key_Escape) {
            m_canvas->clearPoints();
        } else if (event->key() == Qt::Key_Home) {
            m_canvas->resetCamera();
        } else if (event->key() == Qt::Key_Tab) {
            m_panelVisible = !m_panelVisible;
            m_controlPanel->setVisible(m_panelVisible);
        }
    }

    QMainWindow::keyPressEvent(event);
}

void MainWindow::showPointsDialog()
{
    if (!m_pointsDialog || !m_canvas) {
        return;
    }

    m_pointsDialog->refreshPoints(m_canvas->points());
    m_pointsDialog->show();
    m_pointsDialog->raise();
    m_pointsDialog->activateWindow();
}


void MainWindow::onPointsDialogApplyRequested(const QVector<Point<Quotient_Number>>& pts)
{
    if (!m_canvas || !m_pointsDialog) {
        return;
    }

    QString message;
    const bool ok = m_canvas->applyPointsFromDialog(pts, &message);

    if (ok) {
        m_pointsDialog->refreshPoints(m_canvas->points());
    }

    m_pointsDialog->reportApplyResult(ok, message);
}
