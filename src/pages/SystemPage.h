#ifndef SYSTEMPAGE_H
#define SYSTEMPAGE_H

#include <QWidget>
#include <QLabel>

class SystemPage : public QWidget
{
    Q_OBJECT
public:
    explicit SystemPage(QWidget *parent = nullptr);

private slots:
    void revealSerial();
    void revealMemory();

private:
    void setupUi();
    QString getDmiValue(const QString &filename);
    
    QLabel *serialLabel;
    QLabel *memDetailLabel;
    QString memTotalString;
};

#endif // SYSTEMPAGE_H
