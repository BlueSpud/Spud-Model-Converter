#include "MMainWindow.h"
#include "ui_MMainWindow.h"

#include <iostream>

#include <QMessageBox>
#include <QSpacerItem>
#include <QGridLayout>
#include <QStandardItemModel>
#include <QStandardPaths>
#include <QFileDialog>

QString MMainWindow::resource_path;

MMainWindow::MMainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MMainWindow) {

    // Set up the UI file
    ui->setupUi(this);

    this->setWindowTitle("Spud Model Converter");

}

MMainWindow::~MMainWindow() {

    delete ui;
}

void MMainWindow::showEvent(QShowEvent* event) {

    QMainWindow::showEvent(event);

    while (!resource_path.length()) {

        // Tell the user they need to locate the directory
        QMessageBox find_directory_dialog;

        find_directory_dialog.setText("Find Directory");
        find_directory_dialog.setInformativeText("Please locate the Resources directory for the Spud Engine");

        QSpacerItem* horizontalSpacer = new QSpacerItem(300, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
        QGridLayout* layout = (QGridLayout*)find_directory_dialog.layout();
        layout->addItem(horizontalSpacer, layout->rowCount(), 0, 1, layout->columnCount());

        find_directory_dialog.exec();

        // Have the user select the spud engine directory
        QFileDialog file_dialog;

        file_dialog.setDirectory(QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first());
        file_dialog.setFileMode(QFileDialog::DirectoryOnly);

        if (file_dialog.exec()) {

            resource_path = file_dialog.selectedFiles().first();
            std::cout << "Resource path: " << resource_path.toStdString() << std::endl;

            break;

        }

    }

    // After this the UI will be shown

}

void MMainWindow::openNewFile() {

    // If we have already loaded something without saving it, ask the user

    // Open a file dialog looking for a .obj file
    QString file_to_load = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                    QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first(),
                                                    tr("Models (*.obj *.dae)"));

    // Set the name of the file
    ui->file_name_label->setText(file_to_load);

    // Load up a file
    QString extension = file_to_load.section(file_to_load.length() - 4, file_to_load.length());
    std::cout << extension.toStdString() << std::endl;

    if (extension.compare(extension, "obj"))
        file = new MOBJFile();

    if (extension.compare(extension, "dae"))
        file = new MDAEFile();

    file->loadFile(file_to_load);

    // Get the material count from the model
    int material_count = file->getMaterialCount();

    // Create a model for the list view
    QStandardItemModel* model = new QStandardItemModel(material_count, 1, this);
    for (int i = 0; i < material_count; i++) {

        model->setItem(i, new QStandardItem(QString::fromStdString(file->getMaterial(i)->material_domain_name)));
    }

    ui->listView->setModel(model);
    ui->listView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    QModelIndex index = model->index(0, 0);
    ui->listView->setCurrentIndex(index);
    selectMaterial(index);

}

void MMainWindow::saveFile() {

    // Open up a dialog to figure out where to save it
    QString file_name = QFileDialog::getSaveFileName(this, tr("Save File"),
                               resource_path + "/Models/model.smdl",
                               tr("Spud Model (*.smdl)"));

    if (file_name.length() && selected_material != nullptr)
        file->saveFile(file_name);

}

void MMainWindow::selectMaterial(QModelIndex index) {

    // Save which row was selected
    selected_material_index = index;
    selected_material = file->getMaterial(selected_material_index.row());

    std::cout << "Showing material" << selected_material_index.row() << std::endl;

    // Get the path of the material if we have it
    if (selected_material->material_path.length() && selected_material_index.row() >= 0) {

        // Show the name of the material
        ui->material_name->setText("<b>" + QString::fromStdString(selected_material->material_path) + "</b>");

        // Show the data that goes with the material


    } else ui->material_name->setText("<b>No material selected</b>");

}

void MMainWindow::chooseMaterial() {

    QString material_path_absolute;

    if (selected_material != nullptr) {

        while (true) {

            // Open up a file dialog starting at the spud engine resource path
            QFileDialog file_dialog;
            file_dialog.setDirectory(resource_path);
            file_dialog.setNameFilter(tr("Materials (*.mat)"));

            if (!file_dialog.exec())
                break;

            material_path_absolute = file_dialog.selectedFiles().first();

            if (material_path_absolute.startsWith(resource_path, Qt::CaseSensitive)) {

                std::cout << "Selected material: " << material_path_absolute.toStdString() << std::endl;

                // Save the material to the file, remove the starting path
                selected_material->material_path = material_path_absolute.remove(resource_path + "/", Qt::CaseSensitive).toStdString();
                selectMaterial(selected_material_index);

                break;

            }

        }

    }

}

void MMainWindow::chooseCollision() {

    // Open a file dialog looking for a .obj file
    QString file_to_load = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                    QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first(),
                                                    tr("Models (*.obj)"));

    // Load in the file
    if (file->collision_model)
        delete file->collision_model;

    file->collision_model = new MOBJFile();
    file->collision_model->loadFile(file_to_load);

}
