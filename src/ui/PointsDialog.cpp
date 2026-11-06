#include "PointsDialog.h"

#include <QTableWidget>
#include <QVariant>
#include <QColor>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QAbstractItemView>
#include <QApplication>
#include <QLineEdit>
#include <QMessageBox>
#include <QModelIndex>
#include <QList>

#include <algorithm>
#include <stdexcept>
#include <sstream>






namespace {
bool parseQuotientDirect(const QString& text, Quotient_Number& out, QString& error)
{
    const QString trimmed = text.trimmed();
    if (trimmed.isEmpty()) {
        error = QStringLiteral("Пустой текст");
        return false;
    }

    std::istringstream in(trimmed.toStdString());

    try {
        in >> out;
    } catch (...) {
        error = QStringLiteral("Оператор ввода не смог прочитать число");
        return false;
    }

    if (in.fail()) {
        error = QStringLiteral("Оператор ввода не принял данные");
        return false;
    }

    return true;
}

QString quotientToString(const Quotient_Number& v)
{
    std::ostringstream out;
    out << v;
    return QString::fromStdString(out.str()).trimmed();
}
} // namespace

PointsDialog::PointsDialog(QWidget* parent)
    : QDialog(parent)
{
    buildUi();
}

void PointsDialog::buildUi()
{
    setWindowTitle(QStringLiteral("Точки"));
    setMinimumSize(480, 380);

    QVBoxLayout* root = new QVBoxLayout(this);

    QLabel* hint = new QLabel(
        QStringLiteral("Правьте координаты прямо в ячейках. "
                       "Проверка выполняется только при нажатии «Применить»."),
        this);
    hint->setWordWrap(true);

    m_table = new QTableWidget(0, 2, this);
    m_table->setHorizontalHeaderLabels({QStringLiteral("X"), QStringLiteral("Y")});
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_table->setAlternatingRowColors(true);

connect(m_table, &QTableWidget::itemChanged, this, &PointsDialog::onTableItemChanged);
    // Прямое редактирование ячеек.
    m_table->setEditTriggers(QAbstractItemView::DoubleClicked |
                             QAbstractItemView::SelectedClicked |
                             QAbstractItemView::EditKeyPressed |
                             QAbstractItemView::AnyKeyPressed);

    m_addButton = new QPushButton(QStringLiteral("Добавить"), this);
    m_deleteButton = new QPushButton(QStringLiteral("Удалить"), this);
    m_refreshButton = new QPushButton(QStringLiteral("Обновить"), this);

m_checkButton = new QPushButton(QStringLiteral("Проверить"), this);
    QHBoxLayout* tableButtons = new QHBoxLayout;
    tableButtons->addWidget(m_addButton);
    tableButtons->addWidget(m_deleteButton);
    tableButtons->addWidget(m_refreshButton);
tableButtons->addWidget(m_checkButton);
    tableButtons->addStretch();

    m_applyButton = new QPushButton(QStringLiteral("Применить"), this);
    m_autoRefresh = new QCheckBox(QStringLiteral("Автообновление"), this);
    m_autoRefresh->setChecked(false);

    m_status = new QLabel(QStringLiteral("Точек: 0"), this);
    m_status->setStyleSheet("color: #555555;");

    QHBoxLayout* bottom = new QHBoxLayout;
    bottom->addWidget(m_autoRefresh);
    bottom->addStretch();
    bottom->addWidget(m_status);
    bottom->addWidget(m_applyButton);

    root->addWidget(hint);
    root->addWidget(m_table, 1);
    root->addLayout(tableButtons);
    root->addLayout(bottom);

    connect(m_addButton, &QPushButton::clicked, this, &PointsDialog::addRow);
    connect(m_deleteButton, &QPushButton::clicked, this, &PointsDialog::deleteSelectedRows);
    connect(m_refreshButton, &QPushButton::clicked, this, &PointsDialog::refreshLast);
connect(m_checkButton, &QPushButton::clicked, this, &PointsDialog::validateClicked);
    connect(m_applyButton, &QPushButton::clicked, this, &PointsDialog::applyClicked);
}




void PointsDialog::refreshPoints(const QVector<Point<Quotient_Number>>& pts)
{
    m_lastPoints = pts;
    m_updating = true;

    m_table->setRowCount(0);

    for (const auto& pt : pts) {
        const int row = m_table->rowCount();
        m_table->insertRow(row);

        QTableWidgetItem* xItem = new QTableWidgetItem(quotientToString(pt.x));
        QTableWidgetItem* yItem = new QTableWidgetItem(quotientToString(pt.y));

        xItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
        yItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);

        m_table->setItem(row, 0, xItem);
        m_table->setItem(row, 1, yItem);
    }

    m_updating = false;
m_dirty = false;
m_status->setText(QStringLiteral("Точек: %1").arg(pts.size()));
}




void PointsDialog::notifyCanvasPointsChanged(const QVector<Point<Quotient_Number>>& pts)
{
    m_lastPoints = pts;

    if (m_updating || !m_autoRefresh || !m_autoRefresh->isChecked()) {
        return;
    }

    if (!m_table) {
        return;
    }


    if (m_dirty) {
        return;
    }

    refreshPoints(pts);
}





void PointsDialog::refreshLast()
{
    if (m_dirty) {
        const QMessageBox::StandardButton answer = QMessageBox::question(
            this,
            QStringLiteral("Окно точек"),
            QStringLiteral("Есть неприменённые изменения. Обновить таблицу и потерять их?"),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No);

        if (answer != QMessageBox::Yes) {
            return;
        }
    }

    refreshPoints(m_lastPoints);
}



void PointsDialog::addRow()
{
    const int row = m_table->rowCount();
    m_table->insertRow(row);

    QTableWidgetItem* xItem = new QTableWidgetItem();
    QTableWidgetItem* yItem = new QTableWidgetItem();

    xItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
    yItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);

    m_table->setItem(row, 0, xItem);
    m_table->setItem(row, 1, yItem);

    m_table->setCurrentCell(row, 0);
    m_table->editItem(xItem);

    markDirty();
m_status->setText(QStringLiteral("Добавлена пустая строка %1. Заполните обе координаты.").arg(row + 1));
}




void PointsDialog::deleteSelectedRows()
{
    QList<int> rows;

    if (m_table->selectionModel()) {
        const auto selected = m_table->selectionModel()->selectedRows();
        for (const QModelIndex& index : selected) {
            rows.push_back(index.row());
        }
    }

    if (rows.isEmpty()) {
        const int current = m_table->currentRow();
        if (current >= 0) {
            rows.push_back(current);
        }
    }

    std::sort(rows.begin(), rows.end(), [](int a, int b) {
        return a > b;
    });

    for (int row : rows) {
        m_table->removeRow(row);
    }

    if (!rows.isEmpty()) {
markDirty();
}
m_status->setText(QStringLiteral("Удалено строк: %1").arg(rows.size()));
}

QVector<Point<Quotient_Number>> PointsDialog::readTablePoints(bool& ok)
{
    QVector<Point<Quotient_Number>> result;

    for (int row = 0; row < m_table->rowCount(); ++row) {
        QString xText;
        QString yText;

        if (QTableWidgetItem* item = m_table->item(row, 0)) {
            xText = item->text().trimmed();
        }

        if (QTableWidgetItem* item = m_table->item(row, 1)) {
            yText = item->text().trimmed();
        }

        if (xText.isEmpty() && yText.isEmpty()) {
            continue;
        }

        if (xText.isEmpty() || yText.isEmpty()) {
            ok = false;
            m_status->setText(
                QStringLiteral("В строке %1 заполните обе координаты или удалите строку")
                    .arg(row + 1));
            return result;
        }

        Quotient_Number x(0);
        Quotient_Number y(0);
        QString error;

        if (!parseQuotientDirect(xText, x, error)) {
            ok = false;
            m_status->setText(QStringLiteral("Ошибка X в строке %1: %2").arg(row + 1).arg(error));
            return result;
        }

        if (!parseQuotientDirect(yText, y, error)) {
            ok = false;
            m_status->setText(QStringLiteral("Ошибка Y в строке %1: %2").arg(row + 1).arg(error));
            return result;
        }

        result.push_back(Point<Quotient_Number>(x, y));
    }

    ok = true;
    return result;
}




void PointsDialog::applyClicked()
{
    bool ok = false;
    const QVector<Point<Quotient_Number>> pts = readTablePoints(ok);

    if (!ok) {
        QMessageBox::warning(
            this,
            QStringLiteral("Проблема ввода"),
            m_status ? m_status->text() : QStringLiteral("Не удалось обработать таблицу."));
        return;
    }

    if (m_table) {
        m_updating = true;

        for (int row = 0; row < m_table->rowCount(); ++row) {
            if (QTableWidgetItem* item = m_table->item(row, 0)) {
                item->setData(Qt::BackgroundRole, QVariant());
            }

            if (QTableWidgetItem* item = m_table->item(row, 1)) {
                item->setData(Qt::BackgroundRole, QVariant());
            }
        }

        m_updating = false;
    }

    emit applyPointsRequested(pts);

    if (m_status) {
        m_status->setText(QStringLiteral("Запрос на применение точек: %1").arg(pts.size()));
    }
}





void PointsDialog::markDirty()
{
    if (m_updating) {
        return;
    }

    m_dirty = true;

    if (m_status) {
        m_status->setText(QStringLiteral("Есть неприменённые изменения таблицы."));
    }
}

void PointsDialog::onTableItemChanged(QTableWidgetItem* item)
{
    if (m_updating || !item) {
        return;
    }

    m_updating = true;
    item->setData(Qt::BackgroundRole, QVariant());
    m_updating = false;

    markDirty();
}

void PointsDialog::validateClicked()
{
    if (!m_table) {
        return;
    }

    const bool oldUpdating = m_updating;
    m_updating = true;

    bool ok = true;
    QString firstError;

    for (int row = 0; row < m_table->rowCount(); ++row) {
        QTableWidgetItem* xItem = m_table->item(row, 0);
        QTableWidgetItem* yItem = m_table->item(row, 1);

        const QString xText = xItem ? xItem->text().trimmed() : QString();
        const QString yText = yItem ? yItem->text().trimmed() : QString();

        if (xItem) {
            xItem->setData(Qt::BackgroundRole, QVariant());
        }
        if (yItem) {
            yItem->setData(Qt::BackgroundRole, QVariant());
        }

        if (xText.isEmpty() && yText.isEmpty()) {
            continue;
        }

        if (xText.isEmpty() || yText.isEmpty()) {
            ok = false;

            if (firstError.isEmpty()) {
                firstError = QStringLiteral(
                ).arg(row + 1);
            }

            if (xText.isEmpty() && xItem) {
                xItem->setData(Qt::BackgroundRole, QColor(255, 205, 205));
            }
            if (yText.isEmpty() && yItem) {
                yItem->setData(Qt::BackgroundRole, QColor(255, 205, 205));
            }

            continue;
        }

        Quotient_Number x(0);
        Quotient_Number y(0);
        QString error;

        if (!parseQuotientDirect(xText, x, error)) {
            ok = false;

            if (firstError.isEmpty()) {
                firstError = QStringLiteral("Ошибка X в строке %1: %2")
                                 .arg(row + 1)
                                 .arg(error);
            }

            if (xItem) {
                xItem->setData(Qt::BackgroundRole, QColor(255, 205, 205));
            }
        }

        if (!parseQuotientDirect(yText, y, error)) {
            ok = false;

            if (firstError.isEmpty()) {
                firstError = QStringLiteral("Ошибка Y в строке %1: %2")
                                 .arg(row + 1)
                                 .arg(error);
            }

            if (yItem) {
                yItem->setData(Qt::BackgroundRole, QColor(255, 205, 205));
            }
        }
    }

    m_updating = oldUpdating;

    if (ok) {
        if (m_status) {
            m_status->setText(QStringLiteral("Таблица корректна. Можно применять."));
        }
    } else {
        if (m_status) {
            m_status->setText(firstError);
        }
    }
}


void PointsDialog::reportApplyResult(bool ok, const QString& message)
{
    if (ok) {
        m_dirty = false;
    }

    if (m_status) {
        if (!message.isEmpty()) {
            m_status->setText(message);
        } else {
            m_status->setText(ok ? QStringLiteral("Точки применены.")
                                 : QStringLiteral("Не удалось применить точки."));
        }
    }
}
