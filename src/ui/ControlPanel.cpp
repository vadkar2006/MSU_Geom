#include "ControlPanel.h"
#include <QVBoxLayout>
#include <QFont>

ControlPanel::ControlPanel(QWidget* parent) : QWidget(parent) {
    setFixedWidth(250);
    setStyleSheet("background-color: #f0f0f0; border-right: 1px solid #cccccc;");
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setSpacing(15); layout->setContentsMargins(15, 15, 15, 15);
    
    QLabel* titleLabel = new QLabel("Настройки", this);
    titleLabel->setFont(QFont("Arial", 14, QFont::Bold));
    titleLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(titleLabel); layout->addSpacing(10);
    
    QLabel* taskLabel = new QLabel("Выберите задачу:", this);
    taskLabel->setFont(QFont("Arial", 10, QFont::Bold));
    layout->addWidget(taskLabel);
    m_taskCombo = new QComboBox(this);
    m_taskCombo->setFont(QFont("Arial", 10));
    m_taskCombo->setStyleSheet("QComboBox { padding: 5px; }");
    layout->addWidget(m_taskCombo); layout->addSpacing(15);
    
    m_radiusLabel = new QLabel("Размер точки: 12 пикс.", this);
    m_radiusLabel->setFont(QFont("Arial", 10));
    layout->addWidget(m_radiusLabel);
    m_radiusSlider = new QSlider(Qt::Horizontal, this);
    m_radiusSlider->setRange(5, 30); m_radiusSlider->setValue(12);
    m_radiusSlider->setTickPosition(QSlider::TicksBelow); m_radiusSlider->setTickInterval(5);
    layout->addWidget(m_radiusSlider); layout->addSpacing(10);
    
    m_epsilonLabel = new QLabel("Эпсилон: 100", this);
    m_epsilonLabel->setFont(QFont("Arial", 10));
    layout->addWidget(m_epsilonLabel);
    m_epsilonSlider = new QSlider(Qt::Horizontal, this);
    m_epsilonSlider->setRange(10, 1000); m_epsilonSlider->setValue(100);
    m_epsilonSlider->setTickPosition(QSlider::TicksBelow); m_epsilonSlider->setTickInterval(100);
    layout->addWidget(m_epsilonSlider); layout->addSpacing(10);
    
    m_showEpsilonCheckBox = new QCheckBox("Показать ε-окрестность", this);
    m_showEpsilonCheckBox->setFont(QFont("Arial", 10));
    layout->addWidget(m_showEpsilonCheckBox); layout->addSpacing(20);
    
    m_clearButton = new QPushButton("Очистить всё (C)", this);
    m_clearButton->setFont(QFont("Arial", 10));
    layout->addWidget(m_clearButton);
    
    m_resetCameraButton = new QPushButton("Сбросить камеру (Home)", this);
    m_resetCameraButton->setFont(QFont("Arial", 10));
    layout->addWidget(m_resetCameraButton);
    
    layout->addStretch();
    
    QLabel* hintLabel = new QLabel("ЛКМ: добавить/двигать\nПКМ: удалить точку", this);
    hintLabel->setFont(QFont("Arial", 9)); hintLabel->setStyleSheet("color: #666666;");
    hintLabel->setWordWrap(true); layout->addWidget(hintLabel);
    
    connect(m_taskCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ControlPanel::onTaskComboChanged);
    connect(m_radiusSlider, &QSlider::valueChanged, this, &ControlPanel::onRadiusSliderChanged);
    connect(m_epsilonSlider, &QSlider::valueChanged, this, &ControlPanel::onEpsilonSliderChanged);
    connect(m_showEpsilonCheckBox, &QCheckBox::toggled, this, &ControlPanel::showEpsilonToggled);
    connect(m_clearButton, &QPushButton::clicked, this, &ControlPanel::clearRequested);
    connect(m_resetCameraButton, &QPushButton::clicked, this, &ControlPanel::resetCameraRequested);
}

void ControlPanel::setTaskNames(const QStringList& names) {
    m_taskCombo->clear(); m_taskCombo->addItems(names);
}
void ControlPanel::onTaskComboChanged(int index) { emit taskChanged(index); }
void ControlPanel::onRadiusSliderChanged(int value) {
    m_radiusLabel->setText(QString("Размер точки: %1 пикс.").arg(value));
    emit pointRadiusChanged(value);
}
void ControlPanel::onEpsilonSliderChanged(int value) {
    m_epsilonLabel->setText(QString("Эпсилон: %1").arg(value));
    emit epsilonChanged(value);
}
