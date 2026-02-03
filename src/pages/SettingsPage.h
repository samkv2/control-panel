
#ifndef SETTINGSPAGE_H
#define SETTINGSPAGE_H

#include <QWidget>
#include <QComboBox>

class SettingsPage : public QWidget
{
    Q_OBJECT
public:
    explicit SettingsPage(QWidget *parent = nullptr);

signals:
    void themeChanged(const QString &themeName);

private slots:
    void applyTheme(int index);

private:
    void setupUi();
    QComboBox *themeCombo;
};

#endif // SETTINGSPAGE_H
