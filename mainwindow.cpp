#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);

    m_xml_model = new QXmlTableModel(true);
    m_xml_reader = new XmlReader(this);

    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->tableView->setModel(m_xml_model);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView->setEditTriggers(QAbstractItemView::AllEditTriggers);

    connect(m_xml_reader, &XmlReader::signalReadyExport, this, &MainWindow::slotExport);
    connect(m_xml_reader, &XmlReader::signalFileReaded, this, &MainWindow::slotOnFileReaded);
    connect(ui->tableView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotCustomMenuRequested(QPoint)));
    connect(ui->tableView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));
}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_xml_model;
    delete m_xml_reader;
}

void MainWindow::slotCustomMenuRequested(QPoint pos) {
    QMenu* menu = new QMenu(this);

    if (ui->tableView->currentIndex().column() != 0) {
        QAction *edit_menu = new QAction(tr("Edit"), this);
        connect(edit_menu, SIGNAL(triggered()), this, SLOT(slotEditRecord()));
        menu->addAction(edit_menu);
    }

    QAction *export_menu = new QAction(tr("Export"), this);
    QAction *delete_menu = new QAction(tr("Delete"), this);

    connect(export_menu, SIGNAL(triggered()), this, SLOT(slotExportRecord()));
    connect(delete_menu, SIGNAL(triggered()), this, SLOT(slotRemoveRecord()));

    menu->addAction(export_menu);
    menu->addAction(delete_menu);

    menu->popup(ui->tableView->viewport()->mapToGlobal(pos));
}

void MainWindow::slotOnFileReaded() {
    if (m_progress != NULL) {
        m_progress->setValue(m_progress->value() + 1);
    } else {
        return;
    }

    if (m_progress->maximum() == m_progress->value()) {
        m_progress->hide();
        delete m_progress;
    }
}

void MainWindow::slotExport() {
    auto _data = m_xml_reader->Get();
    for (auto& d : _data) {
        m_xml_model->AddData(d);
    }
}

void MainWindow::on_pushButton_import_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, "", "");
    int files = m_xml_reader->GetFilesCount(dir);
    if (dir.isEmpty() || (files == 0)) {
        return;
    }

    if (m_progress == NULL) {
        return;
    }

    m_progress = new QProgressDialog("Import files...", "Abort", 0, files);
    m_progress->setMinimumDuration(500);
    m_progress->setValue(0);
    m_progress->topLevelWidget();
    m_progress->show();

    m_xml_reader->Get(dir);
}


void MainWindow::on_pushButton_clear_clicked()
{
    m_xml_model->Clear();
}

void MainWindow::slotEditRecord()
{
    auto field = ui->tableView->currentIndex();

    if (field.column() < 4) {
        auto value = QInputDialog::getText(this, "Edit", "new value");
        m_xml_model->ModifyValue(field.row(), field.column(), value);
    } else {
        auto value = QInputDialog::getText(this, "Edit", "new value (true / false)");
        m_xml_model->ModifyValue(field.row(), field.column(), value == "true" ? true : false);
    }
}

void MainWindow::slotRemoveRecord()
{
    auto index = ui->tableView->currentIndex().row();
    if (index < 0)
        return;

    if (QMessageBox::warning(this, tr("delete operation"), tr("Really delete this record?"),
                             QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) {
        return;
    } else {
        m_xml_model->RemoveFromBD(index);
        ui->tableView->resizeRowsToContents();
    }
}

void MainWindow::slotExportRecord()
{
    auto index = ui->tableView->currentIndex();
    if (index.row() == -1) {
        QMessageBox::warning(this, "Ошибка", "Для сохранения нужно выбрать данные!");
        return;
    }

    QString file_name = QFileDialog::getSaveFileName(this, tr("Save file"), "", tr("XML (*.xml);;txt (*.txt)"));
    QFile f(file_name);

    f.open(QIODevice::WriteOnly);
    m_xml_reader->Write(&f, m_xml_model->GetElement(index.row()));
    f.close();
}
