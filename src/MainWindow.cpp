#include "MainWindow.h"
#include "pages/HomePage.h"
#include "pages/ProgramsPage.h"
#include "pages/SystemPage.h" 
#include "pages/UserPage.h"
#include "pages/SettingsPage.h"
#include "pages/StartupPage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QLabel>
#include <QStackedWidget>
#include <QPushButton>
#include <QStyle>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Aero Control Panel");
    setWindowIcon(QIcon(":/app_icon.png"));
    resize(900, 650);

    // Main Widget
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Top Bar (Navigation)
    QWidget *topBar = new QWidget(this);
    topBar->setObjectName("TopBar");
    topBar->setStyleSheet("background: palette(window); border-bottom: 1px solid palette(mid);");
    topBar->setFixedHeight(50);
    
    QHBoxLayout *topLayout = new QHBoxLayout(topBar);
    topLayout->setContentsMargins(10, 0, 10, 0);
    
    backButton = new QPushButton(this);
    backButton->setIcon(style()->standardIcon(QStyle::SP_ArrowBack));
    backButton->setText(" Back");
    backButton->setCursor(Qt::PointingHandCursor);
    backButton->setStyleSheet("QPushButton { border: none; font-size: 14px; font-weight: bold; color: #003399; } QPushButton:hover { color: #0066CC; }");
    backButton->hide(); // Hidden on Home
    
    topLayout->addWidget(backButton);
    topLayout->addStretch();
    
    // Branding / Title
    QLabel *branding = new QLabel("Control Panel", this);
    branding->setStyleSheet("font-weight: bold; font-size: 14px; color: #404040;");
    topLayout->addWidget(branding);
    
    mainLayout->addWidget(topBar);

    // Stacked Pages
    stackedWidget = new QStackedWidget(this);
    
    HomePage *homePage = new HomePage(this);
    stackedWidget->addWidget(homePage); // Index 0
    
    ProgramsPage *programsPage = new ProgramsPage(this);
    stackedWidget->addWidget(programsPage); // Index 1
    
    SystemPage *systemPage = new SystemPage(this);
    stackedWidget->addWidget(systemPage); // Index 2
    
    UserPage *userPage = new UserPage(this);
    stackedWidget->addWidget(userPage); // Index 3
    
    SettingsPage *settingsPage = new SettingsPage(this); // Pass MainWindow
    // settingsPage->setMainWindow(this); // Not needed as it uses qApp
    stackedWidget->addWidget(settingsPage); // Index 4

    StartupPage *startupPage = new StartupPage(this);
    stackedWidget->addWidget(startupPage); // Index 5
    
    mainLayout->addWidget(stackedWidget);

    // Connections
    connect(backButton, &QPushButton::clicked, [this](){
        stackedWidget->setCurrentIndex(0);
    });
    
    connect(stackedWidget, &QStackedWidget::currentChanged, [this](int index){
        if (index == 0) {
            backButton->hide();
        } else {
            backButton->show();
        }
    });

    // Link HomePage navigation
    connect(homePage, &HomePage::requestNavigation, stackedWidget, &QStackedWidget::setCurrentIndex);
}

MainWindow::~MainWindow()
{
}
