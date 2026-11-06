#pragma once
#include <QMainWindow>
#include <QSplitter>
#include <memory>
#include "TaskManager.h"


class ControlPanel;
class CanvasWidget;
class PointsDialog;


class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

private:
    ControlPanel* m_controlPanel;
    CanvasWidget* m_canvas;
    QSplitter* m_splitter;
    TaskManager m_taskManager;
    
bool m_panelVisible = true;
    int m_currentTaskIndex = 0;
    PointsDialog* m_pointsDialog = nullptr;

private slots:
    void onTaskChanged(int index);
    void showPointsDialog();


    void onPointsDialogApplyRequested(const QVector<Point<Quotient_Number>>& pts);
protected:
    void keyPressEvent(QKeyEvent* event) override;
};
