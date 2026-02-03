#include "SystemPage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QFile>
#include <QTextStream>
#include <QPushButton>
#include <QProcess>
#include <QSysInfo>
#include <QGuiApplication>
#include <QScrollArea>
#include <QRegularExpression>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <unistd.h>

SystemPage::SystemPage(QWidget *parent) : QWidget(parent)
{
    setupUi();
}

void SystemPage::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    QScrollArea *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setStyleSheet("background: transparent;"); 
    
    QWidget *content = new QWidget();
    content->setObjectName("SystemPageContent");
    scroll->setWidget(content);
    mainLayout->addWidget(scroll);

    QVBoxLayout *layout = new QVBoxLayout(content);
    layout->setContentsMargins(20, 20, 20, 40);
    layout->setSpacing(20);
    layout->setAlignment(Qt::AlignHCenter);

    // --- Header Section ---
    QWidget *headerContainer = new QWidget(this);
    QVBoxLayout *headerLayout = new QVBoxLayout(headerContainer);
    
    QLabel *iconLabel = new QLabel(this);
    QPixmap icon(":/icons/shield.svg"); 
    iconLabel->setPixmap(icon.scaled(72, 72, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    iconLabel->setAlignment(Qt::AlignHCenter);
    headerLayout->addWidget(iconLabel);

    QLabel *distroLabel = new QLabel(QSysInfo::prettyProductName(), this);
    distroLabel->setStyleSheet("font-size: 22px; font-weight: bold; margin-top: 5px;");
    distroLabel->setAlignment(Qt::AlignHCenter);
    headerLayout->addWidget(distroLabel);

    QLabel *urlLabel = new QLabel(QString("<a href='https://www.google.com/search?q=%1'>Get Support</a>").arg(QSysInfo::prettyProductName()), this);
    urlLabel->setOpenExternalLinks(true);
    urlLabel->setAlignment(Qt::AlignHCenter);
    headerLayout->addWidget(urlLabel);
    
    layout->addWidget(headerContainer);

    // Helper to add Card sections
    auto addCard = [&](const QString &title, QList<QPair<QString, QString>> rows) {
        QFrame *card = new QFrame(this);
        card->setObjectName("InfoCard");
        // Inherits QSS style for background/border to support Light/Dark Themes
        
        // Add shadow for depth
        QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(card);
        shadow->setBlurRadius(10);
        shadow->setColor(QColor(0, 0, 0, 30));
        shadow->setOffset(0, 2);
        card->setGraphicsEffect(shadow);

        QVBoxLayout *cardLayout = new QVBoxLayout(card);
        cardLayout->setContentsMargins(20, 15, 20, 15);
        cardLayout->setSpacing(10);

        QLabel *header = new QLabel(title, card);
        header->setStyleSheet("font-size: 16px; font-weight: bold; margin-bottom: 5px;");
        header->setAlignment(Qt::AlignLeft);
        cardLayout->addWidget(header);
        
        QFrame *line = new QFrame();
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        // Line color can be handled via QSS better, but generic palette midlight is usually safe
        line->setStyleSheet("color: palette(midlight);");
        cardLayout->addWidget(line);

        QGridLayout *grid = new QGridLayout();
        grid->setColumnStretch(1, 1); 
        grid->setHorizontalSpacing(20);
        grid->setVerticalSpacing(8);

        int rowIdx = 0;
        for (const auto &pair : rows) {
            QLabel *key = new QLabel(pair.first, card);
            // Removed color: palette(text) to let QSS handle it
            key->setStyleSheet("font-weight: normal; opacity: 0.8; font-size: 13px;");
            key->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            
            QWidget *valWidget;
            if (pair.second == "__SERIAL_BUTTON__") {
                QWidget *w = new QWidget();
                QHBoxLayout *h = new QHBoxLayout(w);
                h->setContentsMargins(0,0,0,0);
                h->setSpacing(10);
                serialLabel = new QLabel("Hidden", w);
                serialLabel->setStyleSheet("color: palette(mid); font-style: italic;");
                QPushButton *btn = new QPushButton("Show", w);
                btn->setCursor(Qt::PointingHandCursor);
                btn->setFixedSize(60, 24);
                connect(btn, &QPushButton::clicked, this, &SystemPage::revealSerial);
                h->addWidget(serialLabel);
                h->addWidget(btn);
                h->addStretch();
                valWidget = w;
            } else if (pair.second == "__MEMORY_BUTTON__") {
                 QWidget *w = new QWidget();
                 QHBoxLayout *h = new QHBoxLayout(w);
                 h->setContentsMargins(0,0,0,0);
                 h->setSpacing(10);
                 memDetailLabel = new QLabel(this->memTotalString, w); 
                 // Removed explicit color style to inherit QSS
                 
                 QPushButton *btn = new QPushButton("Details", w);
                 btn->setCursor(Qt::PointingHandCursor);
                 btn->setFixedSize(60, 24);
                 connect(btn, &QPushButton::clicked, this, &SystemPage::revealMemory);
                 
                 h->addWidget(memDetailLabel);
                 h->addWidget(btn);
                 h->addStretch();
                 valWidget = w;
            } else {
                QLabel *val = new QLabel(pair.second, card);
                // Removed explicit color style to inherit QSS
                val->setStyleSheet("font-weight: bold; font-size: 13px;");
                val->setTextInteractionFlags(Qt::TextSelectableByMouse);
                val->setWordWrap(true);
                valWidget = val;
            }

            grid->addWidget(key, rowIdx, 0);
            grid->addWidget(valWidget, rowIdx, 1);
            rowIdx++;
        }
        cardLayout->addLayout(grid);
        
        layout->addWidget(card);
    };

    // --- Software Section ---
    QList<QPair<QString, QString>> softRows;
    QString desktop = qgetenv("XDG_CURRENT_DESKTOP");
    if (desktop.isEmpty()) desktop = "Unknown";
    else desktop.replace("KDE", "KDE Plasma"); 
    
    softRows << qMakePair(QString("Desktop Environment"), desktop);
    softRows << qMakePair(QString("Qt Version"), QString(qVersion()));
    softRows << qMakePair(QString("Kernel Version"), QSysInfo::kernelVersion());
    softRows << qMakePair(QString("Window System"), QGuiApplication::platformName());
    
    addCard("Software Spec", softRows);

    // --- Hardware Section ---
    QList<QPair<QString, QString>> hardRows;
    
    // CPU
    QString cpuModel = "Unknown Processor";
    QString sockets = "1";
    QString cores = "1";
    
    QProcess cpuProc;
    cpuProc.start("lscpu");
    cpuProc.waitForFinished();
    QString cpuOut = cpuProc.readAllStandardOutput();
    QStringList cpuLines = cpuOut.split('\n');
    for(const QString &line : cpuLines) {
        if (line.startsWith("Model name:")) cpuModel = line.split(":").last().trimmed();
        if (line.startsWith("CPU(s):")) cores = line.split(":").last().trimmed();
    }
    hardRows << qMakePair(QString("Processor"), QString("%1 × %2").arg(cores).arg(cpuModel));
    
    // RAM INITIAL
    QString memTotal = "Unknown";
    QProcess memProc;
    memProc.start("sh", QStringList() << "-c" << "awk '/MemTotal/ {print $2}' /proc/meminfo");
    memProc.waitForFinished();
    QString memStr = memProc.readAllStandardOutput().trimmed();
    bool memOk;
    double memKb = memStr.toDouble(&memOk);
    if (memOk && memKb > 0) {
         double gib = memKb / 1024.0 / 1024.0;
         memTotal = QString::number(gib, 'f', 1) + " GiB";
    }
    this->memTotalString = memTotal; // Store for label
    hardRows << qMakePair(QString("Installed RAM"), "__MEMORY_BUTTON__");

    // Graphics
    QString gpu = "Unknown";
    QProcess p;
    p.start("sh", QStringList() << "-c" << "lspci | grep -i 'vga\\|3d' | cut -d ':' -f3 | head -n 1");
    p.waitForFinished(1000);
    QString gpuOut = p.readAllStandardOutput().trimmed();
    if (!gpuOut.isEmpty()) {
        QRegularExpression re("\\[(.*?)\\]");
        QRegularExpressionMatch match = re.match(gpuOut);
        if (match.hasMatch()) gpu = match.captured(1);
        else gpu = gpuOut;
    }
    hardRows << qMakePair(QString("Graphics"), gpu);
    
    // DMI
    hardRows << qMakePair(QString("Manufacturer"), getDmiValue("sys_vendor"));
    hardRows << qMakePair(QString("Product Name"), getDmiValue("product_name"));
    hardRows << qMakePair(QString("Serial Number"), "__SERIAL_BUTTON__");

    addCard("Hardware Spec", hardRows);

    layout->addStretch();
}

QString SystemPage::getDmiValue(const QString &filename) {
    QFile file("/sys/class/dmi/id/" + filename);
    if (!file.open(QIODevice::ReadOnly)) return "Unknown";
    QString val = QString(file.readAll()).trimmed();
    return val.isEmpty() ? "Unknown" : val;
}

void SystemPage::revealSerial() {
    QString sn = getDmiValue("product_serial");
    if (sn != "Unknown" && !sn.isEmpty()) {
        serialLabel->setText(sn);
        serialLabel->setStyleSheet("font-style: normal; font-weight: bold;");
        return;
    }
    
    QString program = "pkexec";
    QStringList args;
    args << "dmidecode" << "-s" << "system-serial-number";
    
    QProcess *process = new QProcess(this);
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [this, process](int exitCode, QProcess::ExitStatus status){
        if (status == QProcess::NormalExit && exitCode == 0) {
            QString output = process->readAllStandardOutput().trimmed();
            if (!output.isEmpty()) {
                serialLabel->setText(output);
                serialLabel->setStyleSheet("font-style: normal; font-weight: bold;");
            } else {
                serialLabel->setText("Not Found");
            }
        } else {
            serialLabel->setText("Access Denied");
        }
        process->deleteLater();
    });
    process->start(program, args);
}

void SystemPage::revealMemory() {
    // Requires pkexec dmidecode -t memory
    QString program = "pkexec";
    QStringList args;
    args << "dmidecode" << "-t" << "memory";
    
    memDetailLabel->setText(this->memTotalString + " (Scanning...)");
    
    QProcess *process = new QProcess(this);
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [this, process](int exitCode, QProcess::ExitStatus status){
        if (status == QProcess::NormalExit && exitCode == 0) {
            QString output = process->readAllStandardOutput();
            // Parse logic
            QStringList lines = output.split('\n');
            QString details = "";
            int count = 0;
            for (int i = 0; i < lines.size(); ++i) {
                QString line = lines[i].trimmed();
                if (line.startsWith("Size:") && !line.contains("No Module")) {
                    // Found a stick
                    QString size = line.split(":").last().trimmed();
                    // Look ahead for Speed and Type
                    QString speed = "Unknown";
                    QString type = "Unknown";
                    
                    for (int j = i+1; j < qMin(i+20, lines.size()); j++) {
                        QString sub = lines[j].trimmed();
                        if (sub.startsWith("Type:") && !sub.contains("Detail")) type = sub.split(":").last().trimmed();
                        if (sub.startsWith("Speed:") && !sub.contains("Configured")) speed = sub.split(":").last().trimmed();
                        if (sub.isEmpty()) break; // End of section
                    }
                    if (count > 0) details += " + ";
                    details += QString("%1 %2 %3").arg(size).arg(type).arg(speed);
                    count++;
                }
            }
            
            if (!details.isEmpty()) {
                memDetailLabel->setText(this->memTotalString + "\n(" + details + ")");
            } else {
                 memDetailLabel->setText(this->memTotalString + " (No detailed info)");
            }
            
        } else {
            memDetailLabel->setText(this->memTotalString + " (Access Denied)");
        }
        process->deleteLater();
    });
    process->start(program, args);
}
