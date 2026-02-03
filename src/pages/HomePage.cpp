
#include "HomePage.h"
#include <QVBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QLabel>
#include <QGraphicsDropShadowEffect>

HomePage::HomePage(QWidget *parent) : QWidget(parent)
{
    setupUi();
}

void HomePage::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(50, 40, 50, 40);
    mainLayout->setSpacing(30);

    // Centered Header
    QLabel *header = new QLabel("Adjust your computer's settings", this);
    header->setObjectName("HeaderLabel");
    header->setStyleSheet("font-size: 24px; margin-bottom: 10px;");
    header->setAlignment(Qt::AlignHCenter);
    mainLayout->addWidget(header);

    // Cards Grid
    QGridLayout *grid = new QGridLayout();
    grid->setHorizontalSpacing(30);
    grid->setVerticalSpacing(30);

    // Helper lambda to create Card Buttons
    auto createCategoryBtn = [this](const QString &title, const QString &desc, int targetIndex, const QString &iconPath) {
        QPushButton *btn = new QPushButton(this);
        btn->setObjectName("CategoryButton");
        btn->setCursor(Qt::PointingHandCursor);
        btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        btn->setFixedHeight(120); // Taller cards

        QHBoxLayout *layout = new QHBoxLayout(btn);
        layout->setContentsMargins(20, 20, 20, 20);
        layout->setSpacing(20);
        
        // Icon
        QLabel *icon = new QLabel(btn);
        icon->setFixedSize(64, 64);
        icon->setStyleSheet("background-color: transparent; border: none;");
        QPixmap pixmap(iconPath);
        icon->setPixmap(pixmap.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        
        QVBoxLayout *textLayout = new QVBoxLayout();
        textLayout->setSpacing(5);
        textLayout->setAlignment(Qt::AlignVCenter);
        
        QLabel *titleLbl = new QLabel(title, btn);
        titleLbl->setStyleSheet("font-size: 18px; font-weight: bold; background: transparent; border: none;");
        
        QLabel *descLbl = new QLabel(desc, btn);
        descLbl->setStyleSheet("font-size: 13px; opacity: 0.8; background: transparent; border: none;");
        descLbl->setWordWrap(true);
        
        textLayout->addWidget(titleLbl);
        textLayout->addWidget(descLbl);
        
        layout->addWidget(icon);
        layout->addLayout(textLayout);
        layout->addStretch(); // Push content left
        
        connect(btn, &QPushButton::clicked, [this, targetIndex](){
            emit requestNavigation(targetIndex);
        });
        
        // Drop Shadow for button
        QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(btn);
        shadow->setBlurRadius(15);
        shadow->setColor(QColor(0, 0, 0, 20)); // Soft shadow
        shadow->setOffset(0, 4);
        btn->setGraphicsEffect(shadow);
        
        return btn;
    };

    // Row 1
    grid->addWidget(createCategoryBtn("System Info", "View detailed hardware & software specs", 2, ":/icons/shield.svg"), 0, 0);
    grid->addWidget(createCategoryBtn("Programs", "Uninstall or update applications", 1, ":/icons/programs.svg"), 0, 1);
    
    // Row 2
    grid->addWidget(createCategoryBtn("Startup Manager", "Control apps that start with your PC", 5, ":/icons/startup.svg"), 1, 0);
    grid->addWidget(createCategoryBtn("Personalization", "Change theme and colors", 4, ":/icons/settings.svg"), 1, 1);
    
    // Row 3 (User Accounts - Centered or just left)
    grid->addWidget(createCategoryBtn("User Accounts", "Manage user profiles and security", 3, ":/icons/user.svg"), 2, 0);

    mainLayout->addLayout(grid);
    mainLayout->addStretch();
}
