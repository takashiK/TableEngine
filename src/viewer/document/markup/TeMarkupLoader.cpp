#include "TeMarkupLoader.h"
#include "viewer/document/TeDocumentSettings.h"

#include <QSettings>
#include <QDir>
#include <QApplication>
#include <QJsonValue>
#include <QJsonObject>

#include <valijson/adapters/qtjson_adapter.hpp>
#include <valijson/utils/qtjson_utils.hpp>
#include <valijson/schema.hpp>
#include <valijson/schema_parser.hpp>
#include <valijson/validator.hpp>

namespace {
    const QString relationFile = "container_suffix.relation";
}


TeMarkupLoader::TeMarkupLoader()
{
}

TeMarkupLoader::~TeMarkupLoader()
{
}

bool TeMarkupLoader::loadAll()
{
    //Load syntaxes
    QSettings setting;
    QString path = setting.value(SETTING_CONTAINER_FOLDER, QApplication::applicationDirPath() + "/markdown").toString();
    QDir dir(path);
    if (!dir.exists()) {
        return false;
    }

    //Load relations
    if (dir.exists(relationFile)) {
        QJsonValue jsonDoc;
        if (!valijson::utils::loadDocument(dir.filePath(relationFile).toStdString(), jsonDoc)) {
            // Failed to load target document
            return false;
        }

        QJsonObject relations = jsonDoc["relation"].toObject();
        for (auto itr = relations.begin(); itr != relations.end(); ++itr) {
            if (dir.exists(itr.value().toString())) {
                addRelation(itr.key(), dir.filePath(itr.value().toString()), true);
            }
        }

        if (dir.exists(jsonDoc["default"].toString())) {
			setDefaultContainer(dir.filePath(jsonDoc["default"].toString()),true);
		}
    }

    return true;
}

bool TeMarkupLoader::saveAll()
{
    QSettings setting;
    QString path = setting.value(SETTING_CONTAINER_FOLDER, QApplication::applicationDirPath() + "/markdown").toString();
    QDir dir;
    dir.mkpath(path);
    dir.setPath(path);

    //save relations
    QFile file(dir.filePath(relationFile));
    if (file.open(QIODevice::WriteOnly)) {

        QJsonObject relations;
        for (auto itr = m_relations.begin(); itr != m_relations.end(); ++itr) {
            if(dir.exists(itr.value())) {
                relations[itr.key()] = itr.value();
            }
        }

        QJsonObject json;
        json["relation"] = relations;

        if (dir.exists(m_defaultContainer)) {
			json["default"] = m_defaultContainer;
		}

        QJsonDocument jsonDoc(json);
        file.write(jsonDoc.toJson());
        file.close();
    }

    return true;
}

/*!
* \brief Set default container
* \param path The path of default container
* \param overwrite If true, overwrite the old default container
* \return true if success, otherwise false
* 
* register the default container. if the path is not under the setting folder. it will be copy to the setting folder.
*/
bool TeMarkupLoader::setDefaultContainer(const QString& path, bool overwrite)
{
    QSettings setting;
    QString stting_path = setting.value(SETTING_CONTAINER_FOLDER, QApplication::applicationDirPath() + "/markdown").toString();
    QDir dir;
    dir.mkpath(stting_path);
    dir.setPath(stting_path);

    QFileInfo info(path);
    if (info.exists()) {
        //check if the path is not under the setting folder
        if (dir.absolutePath() != info.absolutePath()) {
            if (overwrite || !dir.exists(info.fileName())) {
				QFile::copy(path, dir.filePath(info.fileName()));
            }
            else {
                return false;
            }
        }
        m_defaultContainer = info.fileName();
        return true;
    }

	return false;
}

/*!
 * \brief Add a relation
 * \param suffix The suffix of the markdown file
 * \param path The path of the container
 * \param overwrite If true, overwrite the old relation and container
 * \return entries count of the relation if true then -1
 */
int TeMarkupLoader::addRelation(const QString& suffix, const QString& path, bool overwrite)
{
    QSettings setting;
    QString stting_path = setting.value(SETTING_CONTAINER_FOLDER, QApplication::applicationDirPath() + "/markdown").toString();
    QDir dir;
    dir.mkpath(stting_path);
    dir.setPath(stting_path);

    QFileInfo info(path);
    if (info.exists()) {
		//check if the path is not under the setting folder
		if (dir.absolutePath() != info.absolutePath()) {
			if (overwrite || !dir.exists(info.fileName())) {
				QFile::copy(path, dir.filePath(info.fileName()));
			}else{
				return -1;
			}
		}
		m_relations[suffix] = info.fileName();
		return m_relations.size();
	}

	return -1;
}

/*!
 * \brief Delete a relation
 * \param suffix The suffix of the markdown file
 * \return entries count of the relation
 */
int TeMarkupLoader::delRelation(const QString& suffix)
{
    m_relations.remove(suffix);
	return m_relations.size();
}

/*!
 * \brief Get the content of the container
 * \param suffix The suffix of the markdown file
 * \return cotent of the related container file
 */
const QString TeMarkupLoader::relatedContainer(const QString& suffix)
{
    QSettings setting;
    QString stting_path = setting.value(SETTING_CONTAINER_FOLDER, QApplication::applicationDirPath() + "/markdown").toString();
    QDir dir(stting_path);
    if (!dir.exists()) {
        return QString();
    }

    //Load relations
    QString path = m_defaultContainer;
    if (m_relations.contains(suffix)) {
        path = m_relations[suffix];
    }

    QFile file(dir.filePath(path));
    if (file.open(QIODevice::ReadOnly)) {
		return file.readAll();
	}
    return QString();
}