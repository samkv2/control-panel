
#ifndef USERPAGE_H
#define USERPAGE_H

#include <QWidget>
#include <QListWidget>

class UserPage : public QWidget
{
    Q_OBJECT
public:
    explicit UserPage(QWidget *parent = nullptr);

private slots:
    void loadUsers();
    void deleteSelectedUser();
    void createNewUser();

private:
    void setupUi();
    QListWidget *userList;
};

#endif // USERPAGE_H
