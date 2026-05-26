#include "TeDocument.h"
#include "TeDocumentSettings.h"
#include "utils/TeUtils.h"

#include <QFile>
#include <QStringDecoder>
#include <QSettings>

/**
 * @file TeDocument.cpp
 * @brief Declaration of TeDocument.
 * @ingroup viewer
 */


TeDocument::TeDocument(QObject* parent) : QObject(parent)
{
	m_codecList = { "UTF-8", "EUC-JP", "Shift_JIS", "ISO-2022-JP", "UTF-16LE", "UTF-16BE", "UTF-32LE", "UTF-32BE" };
	m_codecName = "UTF-8";
}

bool TeDocument::load(const QString& path, QString codecName)
{
	QSettings setting;

	clear();
	QFile file(path);
	if (file.open(QFile::ReadOnly)) {
		QByteArray data = file.readAll();
		if(codecName.isEmpty()) {
			codecName = detectTextCodec(data,m_codecList);
		}
		QStringDecoder decoder(codecName.toLatin1().constData());
		if (!decoder.isValid()) {
			decoder = QStringDecoder("UTF-8");
			m_codecName = "UTF-8";
			emit codecChanged(m_codecName);
		} else {
			m_codecName = codecName;
			emit codecChanged(codecName);
		}
		m_content = decoder(data);
		m_fileInfo.setFile(path);
		
		return true;
	}
	return false;
}

bool TeDocument::reload(const QString& codecName)
{
	return load(m_fileInfo.filePath(),codecName);
}

void TeDocument::clear()
{
	m_content.clear(); 
}

const QStringList& TeDocument::setCodecList(const QStringList& codecList)
{
	QStringList tmpList;
	for (const QString& codecName : codecList) {
		QStringDecoder decoder(codecName.toLatin1().constData());
		if (decoder.isValid()) {
			tmpList.append(codecName);
		}
	}
	if (!tmpList.isEmpty()) {
		m_codecList = tmpList;
		emit codecListChanged(m_codecList);
	}
	return m_codecList;
}
