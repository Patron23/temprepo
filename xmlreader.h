#ifndef XMLREADER_H
#define XMLREADER_H

#include <QFile>
#include <QXmlStreamReader>
#include <QCoreApplication>
#include <QDirIterator>
#include <QThread>
#include <QDir>

#include "qxmltablemodel.h"

class XmlReader : public QObject
{
    Q_OBJECT

public:
    XmlReader(QObject* _parent) :
        QObject(_parent){};

    void Read();
    void Write(QFile* _to, rData _data);

    int GetFilesCount(QString _dir) {
        int count = 0;

        QDirIterator it(_dir, QStringList("*.xml"), QDir::Files);
        while (it.hasNext()){
            it.next();
            count++;
        }

        return count;
    }
    std::vector<QString> GetFiles();
    std::vector<QString> GetFiles(QString _dir);
    std::vector<rData>& Get();
    void Get(QString _dir);

signals:
    void signalReadyExport();
    void signalFileReaded();


private:
    std::vector<rData> m_data;

    void ReadFiles(std::vector<QString> _from);
};

#endif // XMLREADER_H
