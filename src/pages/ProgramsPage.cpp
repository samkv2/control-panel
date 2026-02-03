
#include "ProgramsPage.h"
#include <QVBoxLayout>
#include <QTableView>
#include <QHeaderView>
#include <QProcess>
#include <QDebug>
#include <QSortFilterProxyModel>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QCheckBox>
#include <QLabel>

// --- Custom Proxy for Filtering ---
class ProgramsProxy : public QSortFilterProxyModel {
public:
    ProgramsProxy(QObject* parent) : QSortFilterProxyModel(parent) {}
    bool showSystem = false;
    
protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override {
        // First check regex filter (Search bar)
        bool matchesSearch = QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
        if (!matchesSearch) return false;
        
        if (showSystem) return true;
        
        // System Filter Logic
        QModelIndex index = sourceModel()->index(source_row, 0, source_parent);
        PackageModel* model = qobject_cast<PackageModel*>(sourceModel());
        if (!model) return true;
        
        const auto& pkg = model->packages().at(source_row);
        
        // 1. Hide libs (start with lib... but allow libreoffice etc if section says so)
        if (pkg.name.startsWith("lib")) return false;
        
        // 2. Hide specific sections
        if (pkg.section == "libs" || pkg.section == "admin" || pkg.section == "kernel" || pkg.section == "base")
            return false;
            
        // 3. Hide required packages
        if (pkg.priority == "required") return false;
        
        return true;
    }
};

// --- Model Implementation ---

PackageModel::PackageModel(QObject *parent) : QAbstractTableModel(parent) {}

int PackageModel::rowCount(const QModelIndex &parent) const {
    return m_packages.size();
}

int PackageModel::columnCount(const QModelIndex &parent) const {
    return 4; // Name, Publisher, Size, Version
}

QVariant PackageModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_packages.size())
        return QVariant();

    const auto &pkg = m_packages[index.row()];

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
            case 0: return pkg.name;
            case 1: return pkg.maintainer;
            case 2: return pkg.size;
            case 3: return pkg.version;
        }
    }
    return QVariant();
}

QVariant PackageModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
        return QVariant();
        
    switch (section) {
        case 0: return "Name";
        case 1: return "Publisher";
        case 2: return "Size (KB)";
        case 3: return "Version";
    }
    return QVariant();
}

void PackageModel::loadData() {
    beginResetModel();
    m_packages.clear();

    QProcess process;
    // Format: Package|Version|Installed-Size|Maintainer|Section|Priority
    process.start("dpkg-query", QStringList() << "-W" << "-f=${binary:Package}|${Version}|${Installed-Size}|${Maintainer}|${Section}|${Priority}\\n");
    process.waitForFinished();
    
    while (process.canReadLine()) {
        QString line = process.readLine().trimmed();
        QStringList parts = line.split('|');
        if (parts.size() >= 6) {
             m_packages.push_back({
                 parts[0],
                 parts[1],
                 parts[2], 
                 parts[3].split('<').first().trimmed(),
                 parts[4], // Section
                 parts[5]  // Priority
             });
        }
    }
    endResetModel();
}

// --- Page Implementation ---

ProgramsPage::ProgramsPage(QWidget *parent) : QWidget(parent) {
    setupUi();
    m_model->loadData();
}

void ProgramsPage::setupUi() {
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(10);
    
    // Controls Row
    QHBoxLayout *controls = new QHBoxLayout();
    
    // Search Bar
    QLineEdit *searchBox = new QLineEdit(this);
    searchBox->setPlaceholderText("Search programs...");
    searchBox->setObjectName("SearchBox");
    
    QCheckBox *sysCheck = new QCheckBox("Show System Programs", this);
    sysCheck->setObjectName("SystemCheckBox");
    
    controls->addWidget(searchBox, 1);
    controls->addWidget(sysCheck);
    
    layout->addLayout(controls);

    m_model = new PackageModel(this);
    ProgramsProxy *proxy = new ProgramsProxy(this);
    proxy->setSourceModel(m_model);
    proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxy->setFilterKeyColumn(0); // Filter by Name column
    
    connect(searchBox, &QLineEdit::textChanged, proxy, &QSortFilterProxyModel::setFilterFixedString);
    connect(sysCheck, &QCheckBox::toggled, [proxy](bool checked){
        proxy->showSystem = checked;
        proxy->invalidate();
    });
    
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
        menu.addAction("Check for Updates", [this, index, proxy](){
            QString pkgName = proxy->data(index.siblingAtColumn(0)).toString();
             // Launch terminal to show update process for this specific app
            QString cmd = QString("pkexec sh -c \"apt-get install --only-upgrade -y %1; echo 'Done. Press Enter to close.'; read x\"").arg(pkgName);
            QProcess::startDetached("x-terminal-emulator", QStringList() << "-e" << "bash" << "-c" << cmd);
        });

        menu.addAction("Uninstall", [this, index, proxy](){
            QString pkgName = proxy->data(index.siblingAtColumn(0)).toString();
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this, "Uninstall Program", 
                                          "Are you sure you want to uninstall " + pkgName + "?\nThis will use pkexec (requires password).",
                                          QMessageBox::Yes|QMessageBox::No);
            if (reply == QMessageBox::Yes) {
                // Use blocking call to wait for finish, then refresh
                QProcess process;
                process.start("pkexec", QStringList() << "apt-get" << "remove" << "-y" << pkgName);
                if (process.waitForFinished(-1)) { // Wait indefinitely for user input
                    if (process.exitCode() == 0) {
                        m_model->loadData(); // Auto Refresh!
                        QMessageBox::information(this, "Success", pkgName + " uninstalled successfully.");
                    } else {
                         QMessageBox::warning(this, "Failed", "Uninstall failed or cancelled.");
                    }
                }
            }
        });
        menu.exec(table->viewport()->mapToGlobal(pos));
    });
    
    table->setColumnWidth(0, 250); // Name
    table->setColumnWidth(1, 200); // Publisher
    
    // Status Bar
    QLabel *statusBar = new QLabel(this);
    statusBar->setStyleSheet("font-weight: bold; padding: 5px;");
    layout->addWidget(statusBar);

    auto updateStatus = [this, proxy, statusBar](){
        int count = proxy->rowCount();
        long long totalSizeKB = 0;
        for(int i=0; i<count; ++i) {
            // Col 2 is size in KB
            QString sizeStr = proxy->data(proxy->index(i, 2)).toString();
            // Basic parsing needed? dpkg gives raw numbers usually with "dpkg-query -W -f=${Installed-Size}"
            // But verify if "kB" suffix is present. Based on model loading, it's just numbers.
            totalSizeKB += sizeStr.toLongLong();
        }
        
        double sizeMB = totalSizeKB / 1024.0;
        QString sizeText;
        if (sizeMB > 1024) {
            sizeText = QString::number(sizeMB/1024.0, 'f', 2) + " GB";
        } else {
            sizeText = QString::number(sizeMB, 'f', 2) + " MB";
        }
        
        statusBar->setText(QString("Total Programs: %1 | Total Size: %2").arg(count).arg(sizeText));
    };

    // Connections for status updates
    connect(proxy, &QAbstractItemModel::rowsInserted, updateStatus);
    connect(proxy, &QAbstractItemModel::rowsRemoved, updateStatus);
    connect(proxy, &QAbstractItemModel::modelReset, updateStatus);
    connect(sysCheck, &QCheckBox::toggled, updateStatus); // Also update on toggle
    connect(searchBox, &QLineEdit::textChanged, updateStatus); // Update on search filter
    
    // Initial update
    updateStatus();
    
    layout->addWidget(table);
    layout->addWidget(statusBar); // Move status bar to bottom
}
