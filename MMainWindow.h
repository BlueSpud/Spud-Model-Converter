#ifndef MMAINWINDOW_H
#define MMAINWINDOW_H

#include <QMainWindow>
#include <QModelIndex>

#include "MOBJFile.h"
#include "MDAEFile.h"

namespace Ui { class MMainWindow; }

/******************************************************************************
 *  Definition for main window                                                *
 ******************************************************************************/

class MMainWindow : public QMainWindow {

    Q_OBJECT

    public:

         explicit MMainWindow(QWidget *parent = 0);

         void showEvent(QShowEvent* event);

        ~MMainWindow();

    public slots:

        void openNewFile();
        void saveFile();

        void selectMaterial(QModelIndex index);
        void chooseMaterial();

        void chooseCollision();

    private:

         Ui::MMainWindow *ui;

         static QString resource_path;
         MFile* file;

         QModelIndex selected_material_index;
         MMaterial* selected_material;

};

#endif // MMAINWINDOW_H
