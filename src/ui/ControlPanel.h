#pragma once
#include <QWidget>
#include <QSlider>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>

class ControlPanel : public QWidget {
    Q_OBJECT
public:
    explicit ControlPanel(QWidget* parent = nullptr);
    void setTaskNames(const QStringList& names);
    void setOperationOptions(const QStringList& options, int current = -1);

public slots:
    void setCoordinateInputs(const QString& xText, const QString& yText);

public:
    int pointRadius() const { return m_radiusSlider->value(); }
    int epsilon() const { return m_epsilonSlider->value(); }
    bool showEpsilonNeighborhood() const { return m_showEpsilonCheckBox->isChecked(); }
    int outputPrecision() const { return m_precisionSpin ? m_precisionSpin->value() : 63; }

signals:
    void pointRadiusChanged(int value);
    void epsilonChanged(int value);
void useBigNumChanged(bool use);
    void showEpsilonToggled(bool checked);
    void clearRequested();
    void resetCameraRequested();
    void taskChanged(int index);
    void operationChanged(int index);
    
void pointInputRequested(const QString& xText, const QString& yText);
    void showPointsRequested();
    void outputPrecisionChanged(int digits);


private slots:
    void onRadiusSliderChanged(int value);
    void onEpsilonSliderChanged(int value);
void onUseBigNumToggled(bool on);
    void onTaskComboChanged(int index);
    void onAddPointClicked();
    void onPrecisionSpinChanged(int value);

private:
    QComboBox* m_taskCombo;
    QComboBox* m_operationCombo = nullptr;
    QLabel* m_operationLabel = nullptr;
    QSlider* m_radiusSlider;
    QSlider* m_epsilonSlider;
QCheckBox* m_bigNumCheck = nullptr;
    QCheckBox* m_showEpsilonCheckBox;
    QLabel* m_radiusLabel;
    QLabel* m_epsilonLabel;
    QPushButton* m_clearButton;
    QPushButton* m_resetCameraButton;
    QLineEdit* m_xInput = nullptr;
    QLineEdit* m_yInput = nullptr;
    
QPushButton* m_addPointButton = nullptr;
    QPushButton* m_pointsButton = nullptr;
    QSpinBox* m_precisionSpin = nullptr;
    QLabel* m_precisionLabel = nullptr;

};
