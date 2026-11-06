#include "ControlPanel.h"
#include <QLabel>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QDialog>
#include <QVBoxLayout>
#include <QFont>
#include <QLineEdit>
#include <QSpinBox>

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
    m_operationLabel = new QLabel(QStringLiteral("Операция над множествами:"), this);
    m_operationLabel->setFont(QFont("Arial", 10, QFont::Bold));
    m_operationLabel->setVisible(false);
    layout->addWidget(m_operationLabel);
    m_operationCombo = new QComboBox(this);
    m_operationCombo->setFont(QFont("Arial", 10));
    m_operationCombo->setStyleSheet("QComboBox { padding: 5px; }");
    m_operationCombo->setVisible(false);
    layout->addWidget(m_operationCombo);
    layout->addSpacing(10);

QLabel* coordinateLabel = new QLabel("Точные координаты (Quotient_Number):", this);
coordinateLabel->setFont(QFont("Arial", 10, QFont::Bold));
layout->addWidget(coordinateLabel);

QLabel* xLabel = new QLabel("X:", this);
xLabel->setFont(QFont("Arial", 10));
layout->addWidget(xLabel);

m_xInput = new QLineEdit(this);
m_xInput->setText("0");
m_xInput->setFont(QFont("Arial", 10));
layout->addWidget(m_xInput);

QLabel* yLabel = new QLabel("Y:", this);
yLabel->setFont(QFont("Arial", 10));
layout->addWidget(yLabel);

m_yInput = new QLineEdit(this);
m_yInput->setText("0");
m_yInput->setFont(QFont("Arial", 10));
layout->addWidget(m_yInput);


m_addPointButton = new QPushButton("Ввести координаты точки...", this);
m_addPointButton->setFont(QFont("Arial", 10));
layout->addWidget(m_addPointButton);

m_pointsButton = new QPushButton("Окно точек...", this);
m_pointsButton->setFont(QFont("Arial", 10));
layout->addWidget(m_pointsButton);

layout->addSpacing(10);


    
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
m_bigNumCheck = new QCheckBox("\u0422\u043e\u0447\u043d\u0430\u044f \u0430\u0440\u0438\u0444\u043c\u0435\u0442\u0438\u043a\u0430 (Quotient_Number)", this);
    m_bigNumCheck->setFont(QFont("Arial", 10));
    m_bigNumCheck->setChecked(true);
    layout->addWidget(m_bigNumCheck);
    connect(m_bigNumCheck, &QCheckBox::toggled, this, &ControlPanel::onUseBigNumToggled);
    
    m_showEpsilonCheckBox = new QCheckBox("Показать ε-окрестность", this);
    m_showEpsilonCheckBox->setFont(QFont("Arial", 10));
    layout->addWidget(m_showEpsilonCheckBox); layout->addSpacing(20);
    
    
    // Управление количеством цифр после точки для вывода Quotient_Number.
    m_precisionLabel = new QLabel(QStringLiteral("Цифр после точки: 63"), this);
    m_precisionLabel->setFont(QFont("Arial", 10));
    layout->addWidget(m_precisionLabel);

    m_precisionSpin = new QSpinBox(this);
    m_precisionSpin->setRange(1, 500);
    m_precisionSpin->setValue(63);
    m_precisionSpin->setToolTip(QStringLiteral("Глобальная точность вывода дробной части: Quotient_Number::Set_Precision(...)"));
    layout->addWidget(m_precisionSpin);
    layout->addSpacing(10);

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
    connect(m_operationCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
        emit operationChanged(index);
    });
    connect(m_radiusSlider, &QSlider::valueChanged, this, &ControlPanel::onRadiusSliderChanged);
    connect(m_epsilonSlider, &QSlider::valueChanged, this, &ControlPanel::onEpsilonSliderChanged);
    connect(m_showEpsilonCheckBox, &QCheckBox::toggled, this, &ControlPanel::showEpsilonToggled);
    connect(m_clearButton, &QPushButton::clicked, this, &ControlPanel::clearRequested);
    connect(m_resetCameraButton, &QPushButton::clicked, this, &ControlPanel::resetCameraRequested);

connect(m_addPointButton, &QPushButton::clicked, this, &ControlPanel::onAddPointClicked);
connect(m_pointsButton, &QPushButton::clicked, this, &ControlPanel::showPointsRequested);
    connect(m_precisionSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &ControlPanel::onPrecisionSpinChanged);


    // Прямой ввод с панели: Enter в X или Y добавляет точку текущими текстами.
    connect(m_xInput, &QLineEdit::returnPressed, this, [this]() {
        if (m_xInput && m_yInput) {
            emit pointInputRequested(m_xInput->text(), m_yInput->text());
        }
    });

    connect(m_yInput, &QLineEdit::returnPressed, this, [this]() {
        if (m_xInput && m_yInput) {
            emit pointInputRequested(m_xInput->text(), m_yInput->text());
        }
    });


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

void ControlPanel::onUseBigNumToggled(bool on) { emit useBigNumChanged(on); }

void ControlPanel::setCoordinateInputs(const QString& xText, const QString& yText)
{
    if (!m_xInput || !m_yInput) return;

    m_xInput->blockSignals(true);
    m_yInput->blockSignals(true);

    m_xInput->setText(xText);
    m_yInput->setText(yText);

    m_xInput->blockSignals(false);
    m_yInput->blockSignals(false);
}

void ControlPanel::onAddPointClicked()
{
    QDialog dlg(this);
    dlg.setWindowTitle(QStringLiteral("Ввод координат точки"));
    dlg.setMinimumWidth(340);

    QVBoxLayout* v = new QVBoxLayout(&dlg);

    QLabel* info = new QLabel(QStringLiteral("Введите координаты. Проверка корректности отключена."), &dlg);
    info->setWordWrap(true);

    QFormLayout* form = new QFormLayout;

    QLineEdit* xEdit = new QLineEdit(m_xInput ? m_xInput->text() : QStringLiteral("0"), &dlg);
    QLineEdit* yEdit = new QLineEdit(m_yInput ? m_yInput->text() : QStringLiteral("0"), &dlg);

    xEdit->setPlaceholderText(QStringLiteral("X"));
    yEdit->setPlaceholderText(QStringLiteral("Y"));

    form->addRow(QStringLiteral("X:"), xEdit);
    form->addRow(QStringLiteral("Y:"), yEdit);

    QDialogButtonBox* box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);

    QObject::connect(box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    v->addWidget(info);
    v->addLayout(form);
    v->addWidget(box);

    if (dlg.exec() != QDialog::Accepted) return;

    QString xText = xEdit->text();
    QString yText = yEdit->text();

    if (m_xInput) {
        m_xInput->blockSignals(true);
        m_xInput->setText(xText);
        m_xInput->blockSignals(false);
    }

    if (m_yInput) {
        m_yInput->blockSignals(true);
        m_yInput->setText(yText);
        m_yInput->blockSignals(false);
    }

    emit pointInputRequested(xText, yText);
}






void ControlPanel::setOperationOptions(const QStringList& options, int current)
{
    const bool visible = !options.isEmpty();
    if (m_operationLabel) {
        m_operationLabel->setVisible(visible);
    }
    if (!m_operationCombo) {
        return;
    }
    m_operationCombo->blockSignals(true);
    m_operationCombo->clear();
    if (visible) {
        m_operationCombo->addItems(options);
        const int idx = (current >= 0 && current < options.size()) ? current : 0;
        m_operationCombo->setCurrentIndex(idx);
    }
    m_operationCombo->setVisible(visible);
    m_operationCombo->blockSignals(false);
}


void ControlPanel::onPrecisionSpinChanged(int value)
{
    if (m_precisionLabel) {
        m_precisionLabel->setText(QStringLiteral("Цифр после точки: %1").arg(value));
    }

    emit outputPrecisionChanged(value);
}
