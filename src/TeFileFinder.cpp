#include "TeFileFinder.h"

#include <QDir>
#include <QFileInfo>

TeFileFinder::TeFileFinder(const QString& path, QObject *parent)
	: TeFinder(parent)
{}

TeFileFinder::~TeFileFinder()
{}

bool TeFileFinder::isValid() const
{
	return false;
}

void TeFileFinder::findStep()
{
	if (m_targetPaths.isEmpty()) {
		emit finished();
		return;
	}

	QString targetPath = m_targetPaths.front();
	m_targetPaths.pop_front();


	QList<TeFileInfo> fileInfos;
	QDir dir(targetPath);

	for (const auto& info : dir.entryInfoList()) {
		bool result = false;

		if (info.isDir()) {
			//find sub dir
			m_targetPaths.push_back(info.filePath());
		}

		switch (type()) {
		case TeFinderType_ByName:
		{	result = regExp().match(info.fileName()).hasMatch();
			break;
		}
		case TeFinderType_BySize:
			if (info.isFile()) {
				if (lessthan()) {
					result = (info.size() <= filesize());
				}
				else {
					result = (info.size() >= filesize());
				}
			}
			break;
		case TeFinderType_ByDate:
			if (lessthan()) {
				result = (info.lastModified() <= date());
			}
			else {
				result = (info.lastModified() >= date());
			}
			break;
		default:
			result = false;
			break;
		}

		if (result) {
			TeFileInfo teInfo;
			teInfo.path = info.filePath();
			teInfo.size = info.size();
			teInfo.lastModified = info.lastModified();
			teInfo.permissions = info.permissions();
			if(info.isDir()) {
				teInfo.type = TeFileInfo::EN_DIR;
			}
			else if(info.isFile()){
				teInfo.type = TeFileInfo::EN_FILE;
			}
			else {
				teInfo.type = TeFileInfo::EN_NONE;
			}
			fileInfos.push_back(teInfo);
		}
	}
	if (!fileInfos.isEmpty()) {
		m_fileInfos.append(fileInfos);
		emit found(fileInfos);
	}

	emit stepFinished();
}
