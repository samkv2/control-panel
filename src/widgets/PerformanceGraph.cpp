
#include "PerformanceGraph.h"
#include <QPainter>
#include <QPainterPath>

PerformanceGraph::PerformanceGraph(QWidget *parent) : QWidget(parent)
{
    // Initialize with 0s
    m_history.resize(m_maxPoints);
    m_history.fill(0.0f);
    
    // Minimum Height
    setMinimumHeight(150);
    setStyleSheet("border: 1px solid #A0A0A0; background: white;");
}

void PerformanceGraph::addValue(float value)
{
    // Shift left
    for (int i = 0; i < m_maxPoints - 1; ++i) {
        m_history[i] = m_history[i+1];
    }
    m_history[m_maxPoints - 1] = value;
    update();
}

void PerformanceGraph::setLineColor(QColor color)
{
    m_lineColor = color;
    update();
}

void PerformanceGraph::setFillColor(QColor color)
{
    m_fillColor = color;
    update();
}

void PerformanceGraph::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Optional: Draw Grid
    painter.setPen(QColor(220, 220, 220)); // Light grid
    int gridStepX = width() / 10;
    int gridStepY = height() / 4;
    
    for(int x=0; x<width(); x += gridStepX) painter.drawLine(x, 0, x, height());
    for(int y=0; y<height(); y += gridStepY) painter.drawLine(0, y, width(), y);

    // Draw Graph
    if (m_history.isEmpty()) return;

    QPainterPath path;
    double xStep = (double)width() / (m_maxPoints - 1);
    
    // Start at bottom left
    path.moveTo(0, height());
    
    for (int i = 0; i < m_maxPoints; ++i) {
        double val = m_history[i]; // 0-100
        double y = height() - (val / 100.0 * height());
        double x = i * xStep;
        
        if (i == 0) path.lineTo(x, y); // First point
        else path.lineTo(x, y);
    }
    
    // Close path for fill
    path.lineTo(width(), height());
    path.closeSubpath();
    
    painter.setBrush(m_fillColor);
    painter.setPen(Qt::NoPen);
    painter.drawPath(path);
    
    // Draw Line on top
    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(m_lineColor, 2));
    
    // Recreate line path only
    QPainterPath linePath;
    for (int i = 0; i < m_maxPoints; ++i) {
        double val = m_history[i]; 
        double y = height() - (val / 100.0 * height());
        double x = i * xStep;
        if (i==0) linePath.moveTo(x, y);
        else linePath.lineTo(x, y);
    }
    painter.drawPath(linePath);
}
