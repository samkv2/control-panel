
#ifndef PERFORMANCEGRAPH_H
#define PERFORMANCEGRAPH_H

#include <QWidget>
#include <QVector>
#include <QColor>
#include <QTimer>

class PerformanceGraph : public QWidget
{
    Q_OBJECT
public:
    explicit PerformanceGraph(QWidget *parent = nullptr);
    void addValue(float value); // 0.0 to 100.0
    void setLineColor(QColor color);
    void setFillColor(QColor color);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QVector<float> m_history;
    int m_maxPoints = 60; // 60 seconds history
    QColor m_lineColor = QColor(0, 120, 215); // Aero Blue
    QColor m_fillColor = QColor(0, 120, 215, 50);
};

#endif // PERFORMANCEGRAPH_H
