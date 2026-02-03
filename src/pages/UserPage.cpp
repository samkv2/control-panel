
#include "UserPage.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFile>
#include <QTextStream>
#include <QProcess>
#include <QMessageBox>
#include <QDialog>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QCheckBox>
#include <QTimer>

UserPage::UserPage(QWidget *parent) : QWidget(parent)
{
    setupUi();
    loadUsers();
}

void UserPage::setupUi()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(40, 40, 40, 40);
    layout->setSpacing(20);

    QLabel *header = new QLabel("Manage User Accounts", this);
    header->setObjectName("HeaderLabel");
    layout->addWidget(header);

    userList = new QListWidget(this);
    userList->setIconSize(QSize(48, 48));
    userList->setObjectName("UserList"); // Style via QSS
    layout->addWidget(userList);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *addBtn = new QPushButton("Create New User", this);
    QPushButton *refreshBtn = new QPushButton("Refresh List", this);
    QPushButton *delBtn = new QPushButton("Delete Selected User", this);
    
    addBtn->setObjectName("ActionBtn");
    refreshBtn->setObjectName("ActionBtn");
    delBtn->setObjectName("ActionBtn");

    btnLayout->addStretch();
    btnLayout->addWidget(addBtn);
    btnLayout->addWidget(refreshBtn);
    btnLayout->addWidget(delBtn);

    layout->addLayout(btnLayout);
    layout->addStretch();

    connect(addBtn, &QPushButton::clicked, this, &UserPage::createNewUser);
    connect(refreshBtn, &QPushButton::clicked, this, &UserPage::loadUsers);
    connect(delBtn, &QPushButton::clicked, this, &UserPage::deleteSelectedUser);
}

void UserPage::loadUsers()
{
    userList->clear();
    QFile file("/etc/passwd");
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine();
            QStringList parts = line.split(':');
            if (parts.size() > 2) {
                int uid = parts[2].toInt();
                // Filter for normal users (UID >= 1000 and < 65534 usually)
                if (uid >= 1000 && uid < 65534) {
                    QString username = parts[0];
                    QString fullName = parts[4].split(',').first();
                    
                    QListWidgetItem *item = new QListWidgetItem(username + (fullName.isEmpty() ? "" : " (" + fullName + ")"));
                    item->setData(Qt::UserRole, username); // Store username for deletion
                    
                    // Try to find profile picture
                    QString iconPath;
                    QString accountIcon = "/var/lib/AccountsService/icons/" + username;
                    QString homeIcon = "/home/" + username + "/.face";

                    if (QFile::exists(accountIcon)) {
                        iconPath = accountIcon;
                    } else if (QFile::exists(homeIcon)) {
                        iconPath = homeIcon;
                    }

                    if (!iconPath.isEmpty()) {
                        // Load and scale properly
                        QPixmap pix(iconPath);
                        if (!pix.isNull()) {
                             // Mask it to be circular or rounded? 
                             // For now, QListWidget just shows the icon square-ish.
                             // We can mask it if we want distinct style, but let's stick to simple first.
                             item->setIcon(QIcon(pix));
                        } else {
                             item->setIcon(QIcon(":/icons/user.svg"));
                        }
                    } else {
                        item->setIcon(QIcon(":/icons/user.svg"));
                    }
                    
                    userList->addItem(item);
                }
            }
        }
    }
}

void UserPage::deleteSelectedUser()
{
    QListWidgetItem *item = userList->currentItem();
    if (!item) return;

    QString username = item->data(Qt::UserRole).toString();
    QMessageBox::StandardButton reply;
    reply = QMessageBox::warning(this, "Delete User", 
                                  "Are you sure you want to delete user '" + username + "'?\nThis will remove their home directory.",
                                  QMessageBox::Yes|QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        // pkexec userdel -r <user>
        QProcess::startDetached("pkexec", QStringList() << "userdel" << "-r" << username);
        // We might want to wait or just refresh after a few seconds
        // Simple delay to allow command to finish before refresh
        QTimer::singleShot(1000, this, &UserPage::loadUsers);
    }
}

void UserPage::createNewUser()
{
    QDialog dlg(this);
    dlg.setWindowTitle("Create New User");
    dlg.setObjectName("UserDialog"); // Main ID for styling
    dlg.setModal(true);
    dlg.setFixedSize(400, 320);
    // Removed inline style to allow QSS control

    QVBoxLayout *layout = new QVBoxLayout(&dlg);
    layout->setSpacing(15);
    layout->setContentsMargins(30, 30, 30, 30);

    QLabel *header = new QLabel("Enter User Details:", &dlg);
    header->setObjectName("DialogLabel");
    
    QLineEdit *userEdit = new QLineEdit(&dlg);
    userEdit->setPlaceholderText("Username (e.g. john)");
    userEdit->setObjectName("DialogInput");

    QLineEdit *fullEdit = new QLineEdit(&dlg);
    fullEdit->setPlaceholderText("Full Name");
    fullEdit->setObjectName("DialogInput");

    QLineEdit *passEdit = new QLineEdit(&dlg);
    passEdit->setPlaceholderText("Password");
    passEdit->setEchoMode(QLineEdit::Password);
    passEdit->setObjectName("DialogInput");

    QCheckBox *adminCheck = new QCheckBox("Administrator (Sudo Access)", &dlg);
    adminCheck->setObjectName("DialogCheck");

    QDialogButtonBox *bbox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    bbox->setObjectName("DialogButtons");
    
    layout->addWidget(header);
    layout->addWidget(userEdit);
    layout->addWidget(fullEdit);
    layout->addWidget(passEdit);
    layout->addWidget(adminCheck);
    layout->addStretch();
    layout->addWidget(bbox);

    connect(bbox, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(bbox, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    if (dlg.exec() == QDialog::Accepted) {
        QString user = userEdit->text().trimmed();
        QString pass = passEdit->text();
        QString name = fullEdit->text().trimmed();
        bool isAdmin = adminCheck->isChecked();

        if (user.isEmpty() || pass.isEmpty()) {
            QMessageBox::warning(this, "Error", "Username and Password are required.");
            return;
        }

        if (user.contains(" ")) {
            QMessageBox::warning(this, "Error", "Username cannot contain spaces.");
            return;
        }

        // Construct command
        // useradd -m -c "Name" -s /bin/bash user && echo user:pass | chpasswd
        QString cmd = QString("useradd -m -c \"%1\" -s /bin/bash %2 && echo \"%2:%3\" | chpasswd").arg(name).arg(user).arg(pass);
        if (isAdmin) {
            cmd += QString(" && usermod -aG sudo %1").arg(user); // Debian uses 'sudo' group
        }

        // Execute via pkexec
        QProcess *proc = new QProcess(this);
        connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [this, proc, user](int exit, QProcess::ExitStatus){
            if (exit == 0) {
                QMessageBox::information(this, "Success", "User '" + user + "' created successfully.");
                loadUsers();
            } else {
                QMessageBox::critical(this, "Failed", "Failed to create user.\nMake sure you authenticated correctly.");
            }
            proc->deleteLater();
        });

        // Use sh -c to execute the chain
        proc->start("pkexec", QStringList() << "sh" << "-c" << cmd);
    }
}
