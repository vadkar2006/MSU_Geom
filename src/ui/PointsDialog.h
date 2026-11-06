#pragma once

#include <QDialog>
#include <QVector>

#include "Point.h"
#include "Quotient_Number.h"

class QTableWidget;
class QTableWidgetItem;
class QPushButton;
class QCheckBox;
class QLabel;

class PointsDialog : public QDialog {
    Q_OBJECT

public:
    explicit PointsDialog(QWidget* parent = nullptr);

    void reportApplyResult(bool ok, const QString& message);
public slots:
    void refreshPoints(const QVector<Point<Quotient_Number>>& pts);
    void notifyCanvasPointsChanged(const QVector<Point<Quotient_Number>>& pts);

signals:
    void applyPoints(const QVector<Point<Quotient_Number>>& pts);

    void applyPointsRequested(const QVector<Point<Quotient_Number>>& pts);
private slots:
    void addRow();
    void deleteSelectedRows();
    void applyClicked();
    void refreshLast();

    void validateClicked();
    void onTableItemChanged(QTableWidgetItem* item);
    void markDirty();
private:
    void buildUi();
    QVector<Point<Quotient_Number>> readTablePoints(bool& ok);

    QTableWidget* m_table = nullptr;
    QPushButton* m_addButton = nullptr;
    QPushButton* m_deleteButton = nullptr;
    QPushButton* m_applyButton = nullptr;
    QPushButton* m_refreshButton = nullptr;
    QPushButton* m_checkButton = nullptr;
    QCheckBox* m_autoRefresh = nullptr;
    QLabel* m_status = nullptr;

    QVector<Point<Quotient_Number>> m_lastPoints;
    bool m_updating = false;
    bool m_dirty = false;
};
