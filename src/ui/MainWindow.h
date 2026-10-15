#pragma once
#include <QMainWindow>
#include <QSplitter>
#include <memory>
#include "TaskManager.h"

class ControlPanel;
class CanvasWidget;

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

private slots:
    void onTaskChanged(int index);

protected:
    void keyPressEvent(QKeyEvent* event) override;
};
