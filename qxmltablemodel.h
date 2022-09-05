#ifndef QXMLTABLEMODEL_H
#define QXMLTABLEMODEL_H

#include <QAbstractTableModel>
#include <QFile>
#include <QFileInfo>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QCoreApplication>
#include <QException>
#include <QVariant>
#include <QThread>

#include <mutex>
#include <vector>

struct rData {
    QString m_filename;
    QString m_texteditor;
    QString m_fileformats;
    QString m_encoding;
    bool m_hasintellisense;
    bool m_hasplugins;
    bool m_cancompile;

    rData(QString _filename, QString _text, QString _filef, QString _encode, bool _intelli = false, bool _plugins = false, bool _compile = false) {
        m_filename        = _filename;
        m_texteditor      = _text;
        m_fileformats     = _filef;
        m_encoding        = _encode;
        m_hasintellisense = _intelli;
        m_hasplugins      = _plugins;
        m_cancompile      = _compile;
    }
    rData() {};
};

class QXmlTableModel : public QAbstractTableModel
{
public:
    QXmlTableModel(bool _async = true);
    ~QXmlTableModel();

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    void AddData(rData _data, bool _write_to_db = true);
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    rData GetElement(int _index);
    void Clear();

    void CreateDBFile();
    void OpenDBFile();
    bool CheckDBFile();

    void ReadDbFile();
    void WriteToDB(rData _data);
    void EditDBLine(rData _data);
    void RemoveFromBD(int _index);
    int GetIndexByName(QString _name);

    void ModifyValue(int _index_row, int _index_col, QString _value);
    void ModifyValue(int _index_row, int _index_col, bool _value);

private:
    QList<rData> m_rows;
    QSqlDatabase m_db;

    bool m_db_async = true;
    std::mutex m_mutex_db;
};

#endif // QXMLTABLEMODEL_H
