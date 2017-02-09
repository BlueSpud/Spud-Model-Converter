/********************************************************************************
** Form generated from reading UI file 'MMainWindow.ui'
**
** Created by: Qt User Interface Compiler version 5.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MMAINWINDOW_H
#define UI_MMAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MMainWindow
{
public:
    QAction *actionOpen;
    QAction *actionSave_File;
    QAction *actionSave_Animation;
    QWidget *centralWidget;
    QListView *listView;
    QLabel *material_list_label;
    QLabel *collision_label;
    QPushButton *collision_button;
    QLabel *material_name;
    QPushButton *pushButton_2;
    QLabel *file_name_label;
    QMenuBar *menuBar;
    QMenu *menuFile;
    QMenu *menuEdit;

    void setupUi(QMainWindow *MMainWindow)
    {
        if (MMainWindow->objectName().isEmpty())
            MMainWindow->setObjectName(QStringLiteral("MMainWindow"));
        MMainWindow->resize(699, 449);
        actionOpen = new QAction(MMainWindow);
        actionOpen->setObjectName(QStringLiteral("actionOpen"));
        actionSave_File = new QAction(MMainWindow);
        actionSave_File->setObjectName(QStringLiteral("actionSave_File"));
        actionSave_Animation = new QAction(MMainWindow);
        actionSave_Animation->setObjectName(QStringLiteral("actionSave_Animation"));
        centralWidget = new QWidget(MMainWindow);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        listView = new QListView(centralWidget);
        listView->setObjectName(QStringLiteral("listView"));
        listView->setGeometry(QRect(10, 110, 221, 311));
        material_list_label = new QLabel(centralWidget);
        material_list_label->setObjectName(QStringLiteral("material_list_label"));
        material_list_label->setGeometry(QRect(10, 90, 60, 16));
        material_list_label->setTextFormat(Qt::RichText);
        collision_label = new QLabel(centralWidget);
        collision_label->setObjectName(QStringLiteral("collision_label"));
        collision_label->setGeometry(QRect(10, 40, 60, 16));
        collision_label->setTextFormat(Qt::RichText);
        collision_button = new QPushButton(centralWidget);
        collision_button->setObjectName(QStringLiteral("collision_button"));
        collision_button->setGeometry(QRect(0, 60, 141, 32));
        material_name = new QLabel(centralWidget);
        material_name->setObjectName(QStringLiteral("material_name"));
        material_name->setGeometry(QRect(240, 110, 451, 20));
        material_name->setTextFormat(Qt::RichText);
        pushButton_2 = new QPushButton(centralWidget);
        pushButton_2->setObjectName(QStringLiteral("pushButton_2"));
        pushButton_2->setGeometry(QRect(230, 130, 141, 32));
        file_name_label = new QLabel(centralWidget);
        file_name_label->setObjectName(QStringLiteral("file_name_label"));
        file_name_label->setGeometry(QRect(10, 10, 691, 16));
        file_name_label->setTextFormat(Qt::RichText);
        MMainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MMainWindow);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 699, 22));
        menuFile = new QMenu(menuBar);
        menuFile->setObjectName(QStringLiteral("menuFile"));
        menuEdit = new QMenu(menuBar);
        menuEdit->setObjectName(QStringLiteral("menuEdit"));
        MMainWindow->setMenuBar(menuBar);

        menuBar->addAction(menuFile->menuAction());
        menuBar->addAction(menuEdit->menuAction());
        menuFile->addAction(actionOpen);
        menuFile->addAction(actionSave_File);
        menuFile->addAction(actionSave_Animation);

        retranslateUi(MMainWindow);
        QObject::connect(actionOpen, SIGNAL(triggered()), MMainWindow, SLOT(openNewFile()));
        QObject::connect(actionSave_File, SIGNAL(triggered()), MMainWindow, SLOT(saveFile()));
        QObject::connect(listView, SIGNAL(clicked(QModelIndex)), MMainWindow, SLOT(selectMaterial(QModelIndex)));
        QObject::connect(pushButton_2, SIGNAL(clicked()), MMainWindow, SLOT(chooseMaterial()));
        QObject::connect(collision_button, SIGNAL(clicked()), MMainWindow, SLOT(chooseCollision()));
        QObject::connect(actionSave_Animation, SIGNAL(triggered()), MMainWindow, SLOT(saveAnimation()));

        QMetaObject::connectSlotsByName(MMainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MMainWindow)
    {
        MMainWindow->setWindowTitle(QApplication::translate("MMainWindow", "MMainWindow", 0));
        actionOpen->setText(QApplication::translate("MMainWindow", "Open File...", 0));
        actionSave_File->setText(QApplication::translate("MMainWindow", "Save File...", 0));
        actionSave_Animation->setText(QApplication::translate("MMainWindow", "Save Animation...", 0));
        material_list_label->setText(QApplication::translate("MMainWindow", "<b>Materials", 0));
        collision_label->setText(QApplication::translate("MMainWindow", "<html><head/><body><p><span style=\" font-weight:600;\">Collision</span></p></body></html>", 0));
        collision_button->setText(QApplication::translate("MMainWindow", "Choose Collision", 0));
        material_name->setText(QApplication::translate("MMainWindow", "<html><head/><body><p><span style=\" font-weight:600;\">No material selected</span><br/></p></body></html>", 0));
        pushButton_2->setText(QApplication::translate("MMainWindow", "Choose Material", 0));
        file_name_label->setText(QApplication::translate("MMainWindow", "<html><head/><body><p>No file loaded</p></body></html>", 0));
        menuFile->setTitle(QApplication::translate("MMainWindow", "File", 0));
        menuEdit->setTitle(QApplication::translate("MMainWindow", "Edit", 0));
    } // retranslateUi

};

namespace Ui {
    class MMainWindow: public Ui_MMainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MMAINWINDOW_H
