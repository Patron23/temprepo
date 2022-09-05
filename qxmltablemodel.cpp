#include "qxmltablemodel.h"


QXmlTableModel::QXmlTableModel(bool _async)
{
    m_db_async = _async;


    if (!CheckDBFile()) {
        CreateDBFile();
    } else {
        ReadDbFile();
    }
}

QXmlTableModel::~QXmlTableModel()
{
}

int QXmlTableModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    return m_rows.size();
}

int QXmlTableModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    return 6;
}

QVariant QXmlTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    auto row = m_rows.at(index.row());
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch(index.column()) {
        case 0: return row.m_filename;
        case 1: return row.m_texteditor;
        case 2: return row.m_fileformats;
        case 3: return row.m_encoding;
        case 4: return row.m_hasintellisense ? "true" : "false";
        case 5: return row.m_hasplugins ? "true" : "false";
        case 6: return row.m_cancompile ? "true" : "false";
        }
    }

    return QVariant();
}

bool QXmlTableModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (!index.isValid()) return false;

    if (role == Qt::EditRole) {
        switch(index.column()) {
        case 0:
            break;
           // m_rows[index.row()].m_filename = value.toString();
        case 1:
            m_rows[index.row()].m_texteditor  = value.toString();
            EditDBLine(m_rows[index.row()]);
            break;
        case 2:
            m_rows[index.row()].m_fileformats = value.toString();
            EditDBLine(m_rows[index.row()]);
            break;
        case 3:
            m_rows[index.row()].m_encoding    = value.toString();
            EditDBLine(m_rows[index.row()]);
            break;
        case 4:
            m_rows[index.row()].m_hasintellisense = value.toBool();
            EditDBLine(m_rows[index.row()]);
            break;
        case 5:
            m_rows[index.row()].m_hasplugins      = value.toBool();
            EditDBLine(m_rows[index.row()]);
            break;
        case 6:
            m_rows[index.row()].m_cancompile      = value.toBool();
            EditDBLine(m_rows[index.row()]);
            break;
        default:
            throw;
        }
    }
    return true;
}

Qt::ItemFlags QXmlTableModel::flags(const QModelIndex &index) const {
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }

    if (index.column() == 0)
        return Qt::ItemIsEnabled;// | Qt::ItemIsSelectable;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;// | Qt::ItemIsEditable;
}

void QXmlTableModel::AddData(rData _data, bool _write_to_db) {
    beginResetModel();

    if (_write_to_db)
        WriteToDB(_data);

    bool dublicate = false;
    for (auto& r : m_rows) {
        if (_data.m_filename == r.m_filename) {
            dublicate = true;
            break;
        }
    }

    if (!dublicate) {
        m_rows.append(std::move(_data));
    }

    endResetModel();


}

QVariant QXmlTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section)
        {
        case 0: return "file name";
        case 1: return "text editor";
        case 2: return "file formats";
        case 3: return "encoding";
        case 4: return "has intellisense";
        case 5: return "has plugins";
        case 6: return "can compile";
        default : return QAbstractItemModel::headerData(section, orientation, role);
        }
    }

    return QAbstractItemModel::headerData(section, orientation, role);
}

rData QXmlTableModel::GetElement(int _index) {
    return m_rows.at(_index);
}

void QXmlTableModel::Clear() {
    beginResetModel();
    m_rows.clear();
    endResetModel();
}

void QXmlTableModel::CreateDBFile() {
    QString path = "dbdata.db";

    {
    QSqlDatabase _db;
    _db = QSqlDatabase::addDatabase("QSQLITE", "createDB");
    _db.setDatabaseName(path);
    _db.open();

    QSqlQuery query(_db);
    query.exec("create table data"
               "(id integer primary key,"
               "filename varchar(30),"
               "texteditor varchar(30),"
               "fileformats varchar(30),"
               "encoding varchar(30),"
               "hasintellisense bool,"
               "hasplugins bool,"
               "cancompile bool)");

    _db.close();
    }

    QSqlDatabase::removeDatabase("createDB");
}

void QXmlTableModel::OpenDBFile() {
    QString path = "dbdata.db";

    m_db = QSqlDatabase::addDatabase("QSQLITE", "openDB");
    m_db.setDatabaseName(path);
    m_db.open();
}

bool QXmlTableModel::CheckDBFile() {
    return QFileInfo::exists("dbdata.db");
}

void QXmlTableModel::ReadDbFile() {

    auto func = [this]()
    {
        m_mutex_db.lock();
        QString path = "dbdata.db";

        {
        QSqlDatabase _db;
        _db = QSqlDatabase::addDatabase("QSQLITE", "readDB");
        _db.setDatabaseName(path);
        _db.open();

        if (!_db.isOpen()) {
            return;
        }

        QSqlQuery query(_db);
        rData temp_data;

        if (!query.exec("SELECT * FROM data")) {
            return;
        }

        if (query.first()) {
            temp_data.m_filename    = query.value("filename").toString();
            temp_data.m_texteditor  = query.value("texteditor").toString();
            temp_data.m_fileformats = query.value("fileformats").toString();
            temp_data.m_encoding    = query.value("encoding").toString();

            temp_data.m_hasintellisense = query.value("hasintellisense").toBool();
            temp_data.m_hasplugins      = query.value("hasplugins").toBool();
            temp_data.m_cancompile      = query.value("cancompile").toBool();

            AddData(temp_data, false);

            while (query.next()) {
                temp_data.m_filename    = query.value("filename").toString();
                temp_data.m_texteditor  = query.value("texteditor").toString();
                temp_data.m_fileformats = query.value("fileformats").toString();
                temp_data.m_encoding    = query.value("encoding").toString();

                temp_data.m_hasintellisense = query.value("hasintellisense").toBool();
                temp_data.m_hasplugins      = query.value("hasplugins").toBool();
                temp_data.m_cancompile      = query.value("cancompile").toBool();

                AddData(temp_data, false);
            }
        }

        _db.close();
        }
        QSqlDatabase::removeDatabase("readDB");
        m_mutex_db.unlock();
    };

    if (m_db_async) {
        try
        {
            std::thread thr(func);
            thr.detach();
        }
        catch(...)
        {
            m_mutex_db.unlock();
        }
    } else
    {
        func();
    }

}

void QXmlTableModel::WriteToDB(rData _data) {
    auto func = [this](rData _data)
    {
        m_mutex_db.lock();
        QString path = "dbdata.db";
        {
        QSqlDatabase _db;
        _db = QSqlDatabase::addDatabase("QSQLITE", "writeDB");
        _db.setDatabaseName(path);
        _db.open();

        if (!_db.isOpen()) {
            return;
        }

        QSqlQuery query(_db);

        // check for dublicate
        int id = -1;
        if (!query.exec("SELECT ID FROM data WHERE filename = '" + _data.m_filename + "'")) {

            qDebug() << "EditDBLine: query.exec return error while SELECT ID";
            return;
        }
        query.first();
        id = query.value("ID").toInt();

        if (id == 0) {
            query.prepare("INSERT INTO data (filename, texteditor, fileformats, encoding, hasintellisense, hasplugins, cancompile) "
                          "VALUES(?, ?, ?, ?, ?, ?, ?)");
            query.addBindValue(_data.m_filename);
            query.addBindValue(_data.m_texteditor);
            query.addBindValue(_data.m_fileformats);
            query.addBindValue(_data.m_encoding);
            query.addBindValue(_data.m_hasintellisense);
            query.addBindValue(_data.m_hasplugins);
            query.addBindValue(_data.m_cancompile);

            query.exec();
        }

        _db.close();
        }
        QSqlDatabase::removeDatabase("writeDB");
        m_mutex_db.unlock();
    };

    if (m_db_async) {


        try
        {
            std::thread thr(func, _data);
            thr.detach();
        }
        catch(...)
        {
            m_mutex_db.unlock();
        }
    } else
    {
        func(_data);
    }
}

void QXmlTableModel::EditDBLine(rData _data) {
    auto func = [this](rData _data) {
        m_mutex_db.lock();
        QString path = "dbdata.db";
        {
        QSqlDatabase _db;
        _db = QSqlDatabase::addDatabase("QSQLITE", "editDB");
        _db.setDatabaseName(path);
        _db.open();

        if (!_db.isOpen()) {
            return;
        }

        QSqlQuery query(_db);
        int id = -1;

        if (!query.exec("SELECT ID FROM data WHERE filename = '" + _data.m_filename + "'")) {\

            qDebug() << "EditDBLine: query.exec return error while SELECT ID";
            return;
        }

        query.first();

        id = query.value("ID").toInt();

        if (id != -1) {
            query.prepare("UPDATE data SET texteditor=:texteditor, fileformats=:fileformats, encoding=:encoding, hasintellisense=:hasintellisense, hasplugins=:hasplugins, cancompile=:cancompile WHERE ID=:ID  ");

            query.bindValue(":texteditor",    _data.m_texteditor);
            query.bindValue(":fileformats", _data.m_fileformats);
            query.bindValue(":encoding",  _data.m_encoding);
            query.bindValue(":hasintellisense", _data.m_hasintellisense);
            query.bindValue(":hasplugins",      _data.m_hasplugins);
            query.bindValue(":cancompile", _data.m_cancompile);
            query.bindValue(":ID", id);

            if (!query.exec()) {
                qDebug() << "EditDBLine: error when UPDATE. ";
            }
        }

        _db.close();
        }
        QSqlDatabase::removeDatabase("editDB");
        m_mutex_db.unlock();
    };

    if (m_db_async) {
        try
        {
            std::thread thr(func, _data);
            thr.detach();
        }
        catch(...)
        {
            m_mutex_db.unlock();
        }
    } else
    {
        func(_data);
    }
}

void QXmlTableModel::RemoveFromBD(int _index) {
    auto func = [this](int index)
    {
        m_mutex_db.lock();
        QString path = "dbdata.db";

        {
            QSqlDatabase _db;
            _db = QSqlDatabase::addDatabase("QSQLITE", "deleteDB");
            _db.setDatabaseName(path);
            _db.open();

            if (!_db.isOpen()) {
                return;
            }

            QSqlQuery query(_db);

            query.prepare("DELETE FROM data where filename = '" + m_rows.at(index).m_filename + "'");

            if (!query.exec()) {
                return;
            }
            beginResetModel();
            m_rows.removeAt(index);
            endResetModel();
            _db.close();
        }
        QSqlDatabase::removeDatabase("deleteDB");
        m_mutex_db.unlock();
    };

    if (m_db_async) {
        try
        {
            std::thread thr(func, _index);
            thr.detach();
        }
        catch(...)
        {
            m_mutex_db.unlock();
        }
    } else
    {
        func(_index);
    }
}

int QXmlTableModel::GetIndexByName(QString _name) {
    for (int i = 0; i < m_rows.size(); ++i) {
        if (m_rows.at(i).m_filename == _name) {
            return i;
        }
    }

    return -1;
}

void QXmlTableModel::ModifyValue(int _index_row, int _index_col, QString _value) {
    switch(_index_col) {
    case 0:
        m_rows[_index_row].m_filename = _value;
        break;
    case 1:
        m_rows[_index_row].m_texteditor = _value;
        break;
    case 2:
        m_rows[_index_row].m_fileformats = _value;
        break;
    case 3:
        m_rows[_index_row].m_encoding = _value;
        break;
    default:
        break;
    }

    EditDBLine(m_rows.at(_index_row));
}

void QXmlTableModel::ModifyValue(int _index_row, int _index_col, bool _value) {
    switch(_index_col) {
    case 4:
        m_rows[_index_row].m_hasintellisense = _value;
        break;
    case 5:
        m_rows[_index_row].m_hasplugins = _value;
        break;
    case 6:
        m_rows[_index_row].m_cancompile = _value;
        break;
    default:
        break;
    }

    EditDBLine(m_rows.at(_index_row));
}
