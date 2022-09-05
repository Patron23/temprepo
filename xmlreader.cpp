#include "xmlreader.h"

void XmlReader::ReadFiles(std::vector<QString> _from) {
    for (auto f : _from) {
        QFile* file = new QFile(f);
        if (file->open(QIODevice::ReadOnly | QIODevice::Text)) {
            QXmlStreamReader xmlReader(file);
            rData newData;

            newData.m_filename = QFileInfo(file->fileName()).fileName();

            while (!xmlReader.atEnd() && !xmlReader.hasError()) {
                QXmlStreamReader::TokenType token = xmlReader.readNext();

                if (token == QXmlStreamReader::StartDocument)
                    continue;

                if (token == QXmlStreamReader::StartElement) {
                    if (xmlReader.name() == QString("texteditor")) {
                        newData.m_texteditor = xmlReader.readElementText();
                    }
                    if (xmlReader.name() == QString("fileformats")) {
                        newData.m_fileformats = xmlReader.readElementText();
                    }
                    if (xmlReader.name() == QString("encoding")) {
                        newData.m_encoding = xmlReader.readElementText();
                    }
                    if (xmlReader.name() == QString("hasintellisense")) {
                        if (xmlReader.readElementText() == "true" || xmlReader.readElementText() == "false") {
                            newData.m_hasintellisense = xmlReader.readElementText() == "true" ? true : false;
                        }
                    }
                    if (xmlReader.name() == QString("hasplugins")) {
                        if (xmlReader.readElementText() == "true" || xmlReader.readElementText() == "false") {
                            newData.m_hasintellisense = xmlReader.readElementText() == "true" ? true : false;
                        }
                    }
                    if (xmlReader.name() == QString("cancompile")) {
                        if (xmlReader.readElementText() == "true" || xmlReader.readElementText() == "false") {
                            newData.m_hasintellisense = xmlReader.readElementText() == "true" ? true : false;
                        }
                    }
                }
            }

            m_data.push_back(newData);
            emit signalFileReaded();
        }
    }
}

void XmlReader::Read() {
    std::vector<QString> files = GetFiles();
    ReadFiles(files);
}

void XmlReader::Write(QFile* _to, rData _data) {
    QXmlStreamWriter xmlWriter(_to);
    xmlWriter.setAutoFormatting(true);
    xmlWriter.writeStartDocument();

    xmlWriter.writeStartElement("root");

    xmlWriter.writeTextElement("texteditor", _data.m_texteditor);
    xmlWriter.writeTextElement("fileformats", _data.m_fileformats);
    xmlWriter.writeTextElement("encoding", _data.m_encoding);

    xmlWriter.writeTextElement("hasintellisense", _data.m_hasintellisense ? "true" : "false");
    xmlWriter.writeTextElement("hasplugins", _data.m_hasplugins ? "true" : "false");
    xmlWriter.writeTextElement("cancompile", _data.m_cancompile ? "true" : "false");

    xmlWriter.writeEndElement();
}

std::vector<QString> XmlReader::GetFiles() {
    std::vector<QString> xml_files;

    QDirIterator it(QCoreApplication::applicationDirPath(), QStringList("*.xml"), QDir::Files);
    while (it.hasNext()){
        xml_files.push_back(it.next());
    }

    return xml_files;
}

std::vector<QString> XmlReader::GetFiles(QString _dir)
{
    std::vector<QString> xml_files;

    QDirIterator it(_dir, QStringList("*.xml"), QDir::Files);
    while (it.hasNext()){
        xml_files.push_back(it.next());
    }

    return xml_files;
}

std::vector<rData> &XmlReader::Get() {return m_data;}

void XmlReader::Get(QString _dir)
{
    std::thread thr;
    auto func = [this](QString _val) {
        m_data.clear();

        auto f = GetFiles(_val);
        ReadFiles(f);

        emit signalReadyExport();
    };

    thr = std::thread(func, _dir);
    thr.detach();
}
