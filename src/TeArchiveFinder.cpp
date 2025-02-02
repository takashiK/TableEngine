#include "TeArchiveFinder.h"
#include "TeArchive.h"

#include <QFileInfo>

TeArchiveFinder::TeArchiveFinder(const QString& path, QObject* parent)
	: TeFinder(parent)
{
	mp_reader = new TeArchive::Reader();
	if (!mp_reader->open(path)) {
		delete mp_reader;
		mp_reader = nullptr;
	}

}

TeArchiveFinder::~TeArchiveFinder()
{}

bool TeArchiveFinder::isValid() const
{
	return (mp_reader != nullptr);
}

void TeArchiveFinder::findStep()
{
	if (mp_reader == nullptr || m_targetPaths.isEmpty()) {
		emit finished();
		return;
	}

	QString targetPath = m_targetPaths.front();
	m_targetPaths.pop_front();

	for (const auto& entry : *mp_reader) {
		bool result = false;
		if (entry.path.startsWith(targetPath)) {
			switch (type()) {
			case TeFinderType_ByName:
			{
				QFileInfo info(entry.path);
				result = regExp().match(info.fileName()).hasMatch();
				break;
			}
			case TeFinderType_BySize:
				if (entry.type == TeFileInfo::EN_FILE){
					if (lessthan()){
						result = (entry.size <= filesize());
					}
					else {
						result = (entry.size >= filesize());
					}
				}
				break;
			case TeFinderType_ByDate:
				if(lessthan()){
					result = (entry.lastModified <= date());
				}
				else {
					result = (entry.lastModified >= date());
				}
				break;
			default:
				result = false;
				break;
			}
		}

		if (result) {
			TeFileInfo teInfo;
			teInfo.path = entry.path;
			teInfo.size = entry.size;
			teInfo.lastModified = entry.lastModified;
			teInfo.permissions = entry.permissions;
			switch (entry.type) {
				case TeFileInfo::EN_FILE:
					teInfo.type = TeFileInfo::EN_FILE;
					break;
				case TeFileInfo::EN_DIR:
					teInfo.type = TeFileInfo::EN_DIR;
					break;
				default:
					teInfo.type = TeFileInfo::EN_NONE;
					break;
			}
			m_fileInfos.push_back(teInfo);
		}
	}
	if (!m_fileInfos.isEmpty()) {
		emit found(m_fileInfos);
	}
	emit finished();
}
