
#include "SettingsPage.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QApplication>
#include <QFile>
#include <QDebug>

SettingsPage::SettingsPage(QWidget *parent) : QWidget(parent)
{
    setupUi();
}

void SettingsPage::setupUi()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(40, 40, 40, 40);
    layout->setSpacing(20);

    QLabel *header = new QLabel("Personalization", this);
    header->setObjectName("HeaderLabel");
    layout->addWidget(header);

    QLabel *desc = new QLabel("Choose a theme for the Control Panel:", this);
    layout->addWidget(desc);

    themeCombo = new QComboBox(this);
    themeCombo->addItem("Light Theme (Aero)", "aero.qss");
    themeCombo->addItem("Dark Theme", "dark.qss");
    
    themeCombo->setFixedWidth(300);
    layout->addWidget(themeCombo);

    connect(themeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsPage::applyTheme);
    
    layout->addStretch();
}

void SettingsPage::applyTheme(int index)
{
    QString qssFile = themeCombo->itemData(index).toString();
    QFile file(":/"+qssFile);
    if (file.open(QIODevice::ReadOnly)) {
        qApp->setStyleSheet(file.readAll());
    }
}
