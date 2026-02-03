
#ifndef PROGRAMSPAGE_H
#define PROGRAMSPAGE_H

#include <QWidget>
#include <QAbstractTableModel>
#include <vector>

struct PackageInfo {
    QString name;
    QString version;
    QString size;
    QString maintainer;
    QString section;
    QString priority;
};

class PackageModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit PackageModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    
    void loadData();
    const std::vector<PackageInfo>& packages() const { return m_packages; }

private:
    std::vector<PackageInfo> m_packages;
};

class ProgramsProxy; // Forward decl

class ProgramsPage : public QWidget
{
    Q_OBJECT
public:
    explicit ProgramsPage(QWidget *parent = nullptr);

private:
    void setupUi();
    PackageModel *m_model;
    bool showSystemApps = false;
    
    // For refresh updates
    void updateStatus();
};

#endif // PROGRAMSPAGE_H
