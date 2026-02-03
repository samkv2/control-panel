
#include "StartupPage.h"
#include <QVBoxLayout>
#include <QTableView>
#include <QHeaderView>
#include <QProcess>
#include <QDebug>
#include <QRegularExpression>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QSortFilterProxyModel> 
#include <QPushButton> // Important for sorting

// --- Model Implementation ---

StartupModel::StartupModel(QObject *parent) : QAbstractTableModel(parent) {}

int StartupModel::rowCount(const QModelIndex &parent) const {
    return m_services.size();
}

int StartupModel::columnCount(const QModelIndex &parent) const {
    return 3; // Name, State, Boot Impact
}

QVariant StartupModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_services.size())
        return QVariant();

    const auto &svc = m_services[index.row()];

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
            case 0: return svc.name;
            case 1: return svc.state;
            case 2: return svc.bootTime.isEmpty() ? "Undefined" : svc.bootTime;
        }
    } else if (role == Qt::ForegroundRole) {
        if (index.column() == 1) { // State
            if (svc.state == "enabled") return QColor("#228B22"); // Green
            if (svc.state == "disabled") return QColor("#B22222"); // Red
        }
        if (index.column() == 2) { // Impact
            if (svc.timeSeconds > 1.0) return QColor("#B22222"); // High Impact (Red)
            if (svc.timeSeconds > 0.3) return QColor("#DAA520"); // Medium Impact (Gold)
            if (svc.timeSeconds > 0) return QColor("#228B22");   // Low Impact (Green)
        }
    }
    return QVariant();
}

QVariant StartupModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
        return QVariant();
        
    switch (section) {
        case 0: return "Service Unit";
        case 1: return "Status";
        case 2: return "Boot Impact";
    }
    return QVariant();
}

void StartupModel::loadData() {
    beginResetModel();
    m_services.clear();

    // 1. Get Blame Times Map
    QMap<QString, QString> blameMap; // name -> timeStr
    QMap<QString, double> timeMap;   // name -> seconds

    QProcess blameProc;
    blameProc.start("systemd-analyze", QStringList() << "blame");
    blameProc.waitForFinished();
    while (blameProc.canReadLine()) {
        QString line = blameProc.readLine().trimmed();
        int spaceIdx = line.indexOf(' ');
        if (spaceIdx != -1) {
             QString timeStr = line.left(spaceIdx);
             QString name = line.mid(spaceIdx).trimmed();
             blameMap[name] = timeStr;
             
             // Simple parsing: if ends in "ms", /1000. If "s", as is.
             double sec = 0.0;
             if (timeStr.endsWith("ms")) {
                 sec = timeStr.left(timeStr.length()-2).toDouble() / 1000.0;
             } else if (timeStr.endsWith("s")) {
                 sec = timeStr.left(timeStr.length()-1).toDouble();
             }
             timeMap[name] = sec;
        }
    }

    // 2. List Units
    QProcess listProc;
    listProc.start("systemctl", QStringList() << "list-unit-files" << "--type=service");
    listProc.waitForFinished();
    
    // Skip Header
    listProc.readLine(); 

    while (listProc.canReadLine()) {
        QString line = listProc.readLine().trimmed();
        if (line.isEmpty() || line.startsWith("UNIT FILE")) continue;
        
        // Split by whitespace
        QStringList parts = line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
        if (parts.size() >= 2) {
            QString name = parts[0];
            QString state = parts[1];
            
            // Only care about enabled/disabled usually, checking masked etc might be noise
            if (state == "enabled" || state == "disabled") {
                 ServiceInfo svc;
                 svc.name = name;
                 svc.state = state;
                 svc.bootTime = blameMap.value(name, "");
                 svc.timeSeconds = timeMap.value(name, 0.0);
                 
                 // If bootTime is empty and it's enabled, it might just be fast or not run this boot
                 m_services.push_back(svc);
            }
        }
    }
    endResetModel();
}

// --- Page Implementation ---

StartupPage::StartupPage(QWidget *parent) : QWidget(parent) {
    setupUi();
    m_model->loadData();
}

void StartupPage::setupUi() {
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(10);
    
    QLabel *header = new QLabel("Manage Startup Services", this);
    header->setObjectName("HeaderLabel");
    layout->addWidget(header);
    
    QLabel *desc = new QLabel("Disable unnecessary services to speed up boot time.\nHigh Impact: > 1s | Medium: > 300ms | Low: < 300ms", this);
    desc->setStyleSheet("color: #505050; margin-bottom: 10px;"); // Hardcoded slightly but follows theme generally
    // Better to rely on QSS but quick styling for desc text
    layout->addWidget(desc);

    m_model = new StartupModel(this);
    
    QSortFilterProxyModel *proxy = new QSortFilterProxyModel(this);
    proxy->setSourceModel(m_model);
    proxy->setSortCaseSensitivity(Qt::CaseInsensitive);

    QTableView *table = new QTableView(this);
    table->setModel(proxy);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setAlternatingRowColors(true);
    table->verticalHeader()->setVisible(false);
    table->horizontalHeader()->setStretchLastSection(true);
    table->setSortingEnabled(true);
    table->setContextMenuPolicy(Qt::CustomContextMenu);
    
    // Context Menu
    connect(table, &QTableView::customContextMenuRequested, [this, table, proxy](const QPoint &pos){
        QModelIndex index = table->indexAt(pos);
        if (!index.isValid()) return;
        
        QMenu menu;
        menu.setStyleSheet("QMenu { background: white; border: 1px solid #A0A0A0; color: black; } QMenu::item:selected { background: #CDE8FF; color: black; }");
        
        // Get service name
        // Column 0 is name
        QString svcName = proxy->data(index.siblingAtColumn(0)).toString();
        QString state = proxy->data(index.siblingAtColumn(1)).toString();
        
        if (state == "disabled") {
            menu.addAction("Enable Service", [this, svcName](){
                 QProcess::startDetached("pkexec", QStringList() << "systemctl" << "enable" << svcName);
                 // Reload delayed?
            });
        } else {
            menu.addAction("Disable Service", [this, svcName](){
                 QMessageBox::StandardButton reply;
                 reply = QMessageBox::warning(this, "Disable Service", 
                                               "Disabling system services can break functionality.\nAre you sure you want to disable " + svcName + "?",
                                               QMessageBox::Yes|QMessageBox::No);
                 if (reply == QMessageBox::Yes) {
                     QProcess::startDetached("pkexec", QStringList() << "systemctl" << "disable" << svcName);
                 }
            });
        }
        
        menu.addAction("Refresh List", [this](){
            m_model->loadData();
        });

        menu.exec(table->viewport()->mapToGlobal(pos));
    });
    
    table->setColumnWidth(0, 300); // Name
    table->setColumnWidth(1, 100); // Status
    
    layout->addWidget(table);
    
    // Bottom Refresh Button
    QPushButton *refreshBtn = new QPushButton("Refresh Data", this);
    refreshBtn->setFixedWidth(150);
    connect(refreshBtn, &QPushButton::clicked, [this](){ m_model->loadData(); });
    layout->addWidget(refreshBtn, 0, Qt::AlignRight);
}
