#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>
#include <QFileDialog>
#include <QHeaderView>
#include <QMessageBox>
#include <QProgressDialog>
#include <QMdiSubWindow>
#include <QInputDialog>
#include <QThread>
#include <QMenu>

#include <memory>

#include "qxmltablemodel.h"
#include "xmlreader.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void signal_export();

private slots:
    void on_pushButton_import_clicked();
    void on_pushButton_clear_clicked();

    void slotExport();
    void slotEditRecord();
    void slotRemoveRecord();
    void slotExportRecord();
    void slotCustomMenuRequested(QPoint pos);
    void slotOnFileReaded();

private:
    Ui::MainWindow*  ui;
    QXmlTableModel*  m_xml_model;
    XmlReader*       m_xml_reader;
    QProgressDialog* m_progress;
};
#endif // MAINWINDOW_H
