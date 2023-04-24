#include "TeTextSyntaxLoader.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QDir>
#include <QApplication>

#include <QSettings>
#include <TeDocumentSettings.h>

#include <valijson/adapters/qtjson_adapter.hpp>
#include <valijson/utils/qtjson_utils.hpp>
#include <valijson/schema.hpp>
#include <valijson/schema_parser.hpp>
#include <valijson/validator.hpp>

namespace {
    const QString relationPath = "text_suffix.relation";

    QBrush makeBrush(const QJsonValue& color) {
        QString colorStr = color.toString();

        int r = colorStr.mid(1, 2).toInt(nullptr, 16);
        int g = colorStr.mid(3, 2).toInt(nullptr, 16);
        int b = colorStr.mid(5, 2).toInt(nullptr, 16);

        return QBrush(QColor(r, g, b));
    }

    QTextCharFormat makeTextFormat(const QJsonValue& style) {
        QTextCharFormat format;

        if (style["bold"].toBool()) {
            format.setFontWeight(QFont::Bold);
        }
        if (style["italic"].toBool()) {
            format.setFontItalic(true);
        }
        if (style["underline"].toBool()) {
            format.setFontUnderline(true);
        }
        if (!style["color"].isUndefined()) {
            format.setForeground(makeBrush(style["color"]));
        }
        if (!style["bgcolor"].isUndefined()) {
            format.setBackground(makeBrush(style["bgcolor"]));
        }
        return format;
    }

    QJsonObject makeStyleObject(const QTextCharFormat& format) {
        QJsonObject style;

        if (format.fontWeight() == QFont::Bold) {
            style["bold"] = true;
        }
        if (format.fontItalic()) {
            style["italic"] = true;
        }
        if (format.fontUnderline()) {
            style["underline"] = true;
        }

        if (format.foreground().style() != Qt::NoBrush) {
            QString rgb = QString("#%1%2%3")
                .arg(format.foreground().color().red(), 2, 16, QChar(u'0'))
                .arg(format.foreground().color().green(), 2, 16, QChar(u'0'))
                .arg(format.foreground().color().blue(), 2, 16, QChar(u'0'));
            style["color"] = rgb;
        }

        if (format.background().style() != Qt::NoBrush) {
            QString rgb = QString("#%1%2%3")
                .arg(format.background().color().red(), 2, 16, QChar(u'0'))
                .arg(format.background().color().green(), 2, 16, QChar(u'0'))
                .arg(format.background().color().blue(), 2, 16, QChar(u'0'));
            style["bgcolor"] = rgb;
        }
        return style;
    }
}

TeTextSyntaxLoader::TeTextSyntaxLoader()
{
	m_isUpdateRelation = false;
}

TeTextSyntaxLoader::~TeTextSyntaxLoader()
{
}

void TeTextSyntaxLoader::clear()
{
    m_isUpdateRelation = false;
    m_relations.clear();
    m_syntaxes.clear();
}

bool TeTextSyntaxLoader::loadAll()
{
    //Load syntaxes
    QSettings setting;
    QString path = setting.value(SETTING_TEXT_HIGHLIGHT_FOLDER, QApplication::applicationDirPath() + "/highlight").toString();
    QDir dir(path);
    if (!dir.exists()) {
        return false;
    }

    QStringList filters;
    filters << "*.highlight";
    QFileInfoList list = dir.entryInfoList(filters, QDir::Files);
    for (const auto& info : list) {
        addEntry(info.baseName(), info.filePath(),true);
    }

    //Load relations
    if (dir.exists(relationPath)) {
        QJsonValue jsonDoc;
        if (!valijson::utils::loadDocument(dir.filePath(relationPath).toStdString(), jsonDoc)) {
            // Failed to load target document
            return false;
        }

        QJsonObject relations = jsonDoc["relation"].toObject();
        for (auto itr = relations.begin(); itr != relations.end(); ++itr) {
            addRelation(itr.key(), itr.value().toString(), true);
        }
    }

	return true;
}

bool TeTextSyntaxLoader::saveAll()
{
    QSettings setting;
    QString path = setting.value(SETTING_TEXT_HIGHLIGHT_FOLDER, QApplication::applicationDirPath() + "/highlight").toString();
    QDir dir;
    dir.mkpath(path);
    dir.setPath(path);
    
    //delete unentryed content
    QStringList filters;
    filters << "*.highlight";
    QFileInfoList list = dir.entryInfoList(filters, QDir::Files);
    for (const auto& info : list) {
        if (!m_syntaxes.contains(info.baseName())) {
            dir.remove(info.filePath());
        }
    }

    //save syntaxes
    for (auto itr = m_syntaxes.begin(); itr != m_syntaxes.end(); ++itr) {
        if (itr.value().isUpdate) {
            saveSyntax(path + "/" + itr.key() + ".highlight", itr.value().textSyntax);
        }
    }

    //save relations
    QFile file(dir.filePath(relationPath));
    if (file.open(QIODevice::WriteOnly)) {

        QJsonObject relations;
        for (auto itr = m_relations.begin(); itr != m_relations.end(); ++itr) {
            if (m_syntaxes.contains(itr.value())) {
                relations[itr.key()] = itr.value();
            }
        }

        QJsonObject json;
        json["relation"] = relations;

        QJsonDocument jsonDoc(json);
        file.write(jsonDoc.toJson());
        file.close();
    }

    return true;
}

bool TeTextSyntaxLoader::loadSyntax(const QString& path, TeTextSyntax* p_textSyntax, bool append)
{
    if(!append){
        p_textSyntax->clear();
    }

    // Load JSON document using QtJSON with Valijson helper function
    QSettings setting;

    QJsonValue jsonSchemaDoc;
    if (!valijson::utils::loadDocument(setting.value(SETTING_TEXT_HIGHLIGHT_SCHEMA).toString().toStdString(), jsonSchemaDoc)) {
        // Failed to load schema document
        return false;
    }
    QJsonValue jsonDoc;
    if (!valijson::utils::loadDocument(path.toStdString(), jsonDoc)) {
        // Failed to load target document
        return false;
    }

    // Parse JSON schema content using valijson
    valijson::Schema jsonSchema;
    valijson::SchemaParser parser;
    valijson::adapters::QtJsonAdapter jsonSchemaAdapter(jsonSchemaDoc);
    parser.populateSchema(jsonSchemaAdapter, jsonSchema);

    // Validate a document
    valijson::Validator validator;
    valijson::adapters::QtJsonAdapter jsonAdapter(jsonDoc);
    if (!validator.validate(jsonSchema, jsonAdapter, NULL)) {
        // Validation failed.
        return false;
    }

    //keywords
    QJsonArray keywords = jsonDoc["keywords"].toArray();
    for (const QJsonValueRef v : keywords) {
        QJsonValue style = v.toObject().value("style");
        QJsonArray list = v.toObject().value("list").toArray();
        TeTextSyntax::SyntaxKeywords syntax;
        syntax.format = makeTextFormat(style);
        for (const QJsonValueRef keyword : list) {
            syntax.keywords.append(keyword.toString());
        }
        p_textSyntax->addSyntaxKeywords(syntax);
    }

    //regexes
    QJsonArray regexes = jsonDoc["regexes"].toArray();
    for (const QJsonValueRef v : regexes) {
        QJsonValue style = v.toObject().value("style");
        QString regex = v.toObject().value("regex").toString();

        TeTextSyntax::SyntaxRegex syntax;
        syntax.format = makeTextFormat(style);
        syntax.regex = QRegularExpression(regex);

        p_textSyntax->addSyntaxRegex(syntax);
    }

    //regions
    QJsonArray regions = jsonDoc["regions"].toArray();
    for (const QJsonValueRef v : regions) {
        QJsonValue style = v.toObject().value("style");
        QString startRegex = v.toObject().value("startRegex").toString();
        QString endRegex = v.toObject().value("endRegex").toString();

        TeTextSyntax::SyntaxRegion syntax;
        syntax.format = makeTextFormat(style);
        syntax.startRegex = QRegularExpression(startRegex);
        syntax.endRegex = QRegularExpression(endRegex);

        p_textSyntax->addSyntaxRegion(syntax);
    }

    return true;
}

bool TeTextSyntaxLoader::saveSyntax(const QString& path, const TeTextSyntax& textSyntax)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        //failed open file
        return false;
    }

    QJsonArray keywords;
    for (const auto& syntax : textSyntax.keywords()) {
        QJsonObject keyword;
        QJsonArray  list;
        for (const auto& key : syntax.keywords) {
            list.append(key);
        }
        keyword["style"] = makeStyleObject(syntax.format);
        keyword["list"] = list;

        keywords.append(keyword);
    }

    QJsonArray regexes;
    for (const auto& syntax : textSyntax.regexes()) {
        QJsonObject regex;
        regex["style"] = makeStyleObject(syntax.format);
        regex["regex"] = syntax.regex.pattern();

        regexes.append(regex);
    }

    QJsonArray regions;
    for (const auto& syntax : textSyntax.regions()) {
        QJsonObject region;
        region["style"] = makeStyleObject(syntax.format);
        region["startRegex"] = syntax.startRegex.pattern();
        region["endRegex"] = syntax.endRegex.pattern();

        regions.append(region);
    }

    QJsonObject json;
    json["keywords"] = keywords;
    json["regexes"] = regexes;
    json["regions"] = regions;

    QJsonDocument jsonDoc(json);
    file.write(jsonDoc.toJson());
    file.close();

    return true;
}

int TeTextSyntaxLoader::addEntry(const QString& title, const QString& path, bool overwrite)
{
	TeTextSyntax syntax;
	if (loadSyntax(path,&syntax)) {
		return addEntry(title, syntax, overwrite);
	}
	else {
		return -1;
	}
}

int TeTextSyntaxLoader::addEntry(const QString& title, const TeTextSyntax& syntax, bool overwrite)
{
	Syntax container;
	container.isUpdate = true;
	container.textSyntax = syntax;

	auto itr = m_syntaxes.find(title);
	if (itr != m_syntaxes.end() ) {
		if (!overwrite) {
			return -1;
		}

		itr.value().isUpdate = true;
		itr.value().textSyntax = syntax;
		return m_syntaxes.count();
	}

	m_syntaxes.insert(title, container);
	return m_syntaxes.count();
}

int TeTextSyntaxLoader::delEntry(const QString& title)
{
	auto itr = m_syntaxes.find(title);
	if (itr != m_syntaxes.end()) {
		m_syntaxes.erase(itr); 
	}
	return m_syntaxes.count();
}

TeTextSyntax TeTextSyntaxLoader::entry(const QString& title)
{
	auto itr = m_syntaxes.find(title);
	if (itr != m_syntaxes.end()) {
		return itr.value().textSyntax;
	}
	return TeTextSyntax();
}

QStringList TeTextSyntaxLoader::titles()
{
	return m_syntaxes.keys();
}

int TeTextSyntaxLoader::addRelation(const QString& suffix, const QString& title, bool overwrite)
{
	auto itr = m_relations.find(suffix);
	if (itr != m_relations.end()) {
		if (!overwrite) {
			return -1;
		}
		itr.value() = title;
	}
	else {
		m_relations.insert(suffix, title);
	}
	return m_relations.count();
}

int TeTextSyntaxLoader::delRelation(const QString& suffix)
{
	auto itr = m_relations.find(suffix);
	if (itr != m_relations.end()) {
		m_relations.erase(itr);
	}
	return m_relations.count();
}

TeTextSyntax TeTextSyntaxLoader::relatedEntry(const QString& suffix)
{
    auto itr = m_relations.find(suffix);
    if (itr != m_relations.end()) {
        auto sitr = m_syntaxes.find(itr.value());
        if (sitr != m_syntaxes.end()) {
            return sitr.value().textSyntax;
        }
        else {
            //related syntax entry is not found. so delete entry.
            m_relations.erase(itr);
        }
    }

	return TeTextSyntax();
}

const QMap<QString, QString>& TeTextSyntaxLoader::relations()
{
	return m_relations;
}
