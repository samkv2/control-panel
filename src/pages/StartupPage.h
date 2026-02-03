
#ifndef STARTUPPAGE_H
#define STARTUPPAGE_H

#include <QWidget>
#include <QAbstractTableModel>
#include <vector>

struct ServiceInfo {
    QString name;
    QString state;
    QString bootTime; // from systemd-analyze blame
    double timeSeconds; // for sorting
};

class StartupModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit StartupModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    
    void loadData();
    const std::vector<ServiceInfo>& services() const { return m_services; }

private:
    std::vector<ServiceInfo> m_services;
};

class StartupPage : public QWidget
{
    Q_OBJECT
public:
    explicit StartupPage(QWidget *parent = nullptr);

private:
    void setupUi();
    StartupModel *m_model;
};

#endif // STARTUPPAGE_H
