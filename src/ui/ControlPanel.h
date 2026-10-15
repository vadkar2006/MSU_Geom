#pragma once
#include <QWidget>
#include <QSlider>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>

class ControlPanel : public QWidget {
    Q_OBJECT
public:
    explicit ControlPanel(QWidget* parent = nullptr);
    void setTaskNames(const QStringList& names);

signals:
    void pointRadiusChanged(int value);
    void epsilonChanged(int value);
    void showEpsilonToggled(bool checked);
    void clearRequested();
    void resetCameraRequested();
    void taskChanged(int index);

private slots:
    void onRadiusSliderChanged(int value);
    void onEpsilonSliderChanged(int value);
    void onTaskComboChanged(int index);

private:
    QComboBox* m_taskCombo;
    QSlider* m_radiusSlider;
    QSlider* m_epsilonSlider;
    QCheckBox* m_showEpsilonCheckBox;
    QLabel* m_radiusLabel;
    QLabel* m_epsilonLabel;
    QPushButton* m_clearButton;
    QPushButton* m_resetCameraButton;
};
