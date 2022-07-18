#include "TeTextSyntax.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>

#include <valijson/adapters/qtjson_adapter.hpp>
#include <valijson/utils/qtjson_utils.hpp>
#include <valijson/schema.hpp>
#include <valijson/schema_parser.hpp>
#include <valijson/validator.hpp>

TeTextSyntax::TeTextSyntax()
{
    m_update_keywords = false;
}


TeTextSyntax::~TeTextSyntax()
{
}

namespace {
    QBrush makeBrush(const QJsonValue& color) {
        QString colorStr = color.toString();

        int r = colorStr.mid(1, 2).toInt(nullptr,16);
        int g = colorStr.mid(3, 2).toInt(nullptr, 16);
        int b = colorStr.mid(5, 2).toInt(nullptr, 16);

        return QBrush(QColor(r,g,b));
    }

    QTextCharFormat makeTextFormat( const QJsonValue& style ) {
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
            QString rgb = QString("#")
                + QString::number(format.foreground().color().red(), 16)
                + QString::number(format.foreground().color().green(), 16)
                + QString::number(format.foreground().color().blue(), 16);
            style["color"] = rgb;
        }

        if (format.background().style() != Qt::NoBrush) {
            QString rgb = QString("#")
                + QString::number(format.background().color().red(), 16)
                + QString::number(format.background().color().green(), 16)
                + QString::number(format.background().color().blue(), 16);
            style["bgcolor"] = rgb;
        }
        return style;
    }

    QRegularExpression listToRegularExpression(const QStringList& list) {
        QString packed;
        if (list.length() > 0) {
            for (const auto& s : list) {
                packed += s + QStringLiteral("|");
            }
            packed.chop(1);

            return QRegularExpression(QStringLiteral("\\b(") + packed + QStringLiteral(")\\b"));
        }

        return QRegularExpression();
    }
}

bool TeTextSyntax::load(QString path)
{
    clear();

    // Load JSON document using QtJSON with Valijson helper function
    QJsonValue jsonSchemaDoc;
    if (!valijson::utils::loadDocument("mySchema.json", jsonSchemaDoc)) {
        // Failed to load schema document
        return false;
    }
    QJsonValue jsonDoc;
    if (!valijson::utils::loadDocument("myTarget.json", jsonDoc)) {
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
        SyntaxKeywords syntax;
        syntax.format = makeTextFormat(style);
        for (const QJsonValueRef keyword : list) {
            syntax.keywords.append(keyword.toString());
        }
        addSyntaxKeywords(syntax);
    }

    //regexes
    QJsonArray regexes = jsonDoc["regexes"].toArray();
    for (const QJsonValueRef v : regexes) {
        QJsonValue style = v.toObject().value("style");
        QString regex = v.toObject().value("regex").toString();

        SyntaxRegex syntax;
        syntax.format = makeTextFormat(style);
        syntax.regex = QRegularExpression(regex);

        addSyntaxRegex(syntax);
    }

    //regions
    QJsonArray regions = jsonDoc["regions"].toArray();
    for (const QJsonValueRef v : regions) {
        QJsonValue style = v.toObject().value("style");
        QString startRegex = v.toObject().value("startRegex").toString();
        QString endRegex = v.toObject().value("endRegex").toString();

        SyntaxRegion syntax;
        syntax.format = makeTextFormat(style);
        syntax.startRegex = QRegularExpression(startRegex);
        syntax.endRegex = QRegularExpression(endRegex);

        addSyntaxRegion(syntax);
    }
}

bool TeTextSyntax::save(QString path)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        //failed open file
        return false;
    }

    QJsonArray keywords;
    for (const auto& syntax : m_keywords) {
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
    for (const auto& syntax : m_regexes) {
        QJsonObject regex;
        regex["style"] = makeStyleObject(syntax.format);
        regex["regex"] = syntax.regex.pattern();

        regexes.append(regex);
    }

    QJsonArray regions;
    for (const auto& syntax : m_regions) {
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
}

void TeTextSyntax::clear()
{
    m_keywords.clear();
    m_keywordRegexes.clear();
    m_regexes.clear();
    m_regions.clear();
}

void TeTextSyntax::update_keywords()
{
    if (m_update_keywords) {
        m_update_keywords = false;
        m_keywordRegexes.clear();
        for (const auto& v : m_keywords) {
            SyntaxRegex regex;
            regex.format = v.format;
            regex.regex = listToRegularExpression(v.keywords);
            m_keywordRegexes.append(regex);
        }
    }
}

int TeTextSyntax::addSyntaxKeywords(const SyntaxKeywords& keyword)
{
    if (keyword.keywords.length() == 0)
        return -1;

    //validation
    for (auto&& key : keyword.keywords) {
        if (key.length() > 30) {
            return -1;
        }
    }
    m_update_keywords = true;
    m_keywords.append(keyword);
    return m_keywords.count();
}

int TeTextSyntax::addSyntaxRegex(const SyntaxRegex& regex)
{
    if (regex.regex.pattern().length() > 200 || !regex.regex.isValid()) {
        return -1;
    }
    return m_regexes.count();
}

int TeTextSyntax::addSyntaxRegion(const SyntaxRegion& region)
{
    if (region.startRegex.pattern().length() > 200 || !region.startRegex.isValid()) {
        return -1;
    }
    if (region.endRegex.pattern().length() > 200 || !region.endRegex.isValid()) {
        return -1;
    }
    return m_regions.count();
}

bool TeTextSyntax::removeSyntaxKeywords(int index)
{
    if (index < m_keywords.count()) {
        m_keywords.removeAt(index);
        return true;
    }
    return false;
}

bool TeTextSyntax::removeSyntaxRegex(int index)
{
    if (index < m_regexes.count()) {
        m_regexes.removeAt(index);
        return true;
    }
    return false;
}

bool TeTextSyntax::removeSyntaxRegion(int index)
{
    if (index < m_regions.count()) {
        m_regions.removeAt(index);
        return true;
    }
    return false;
}
