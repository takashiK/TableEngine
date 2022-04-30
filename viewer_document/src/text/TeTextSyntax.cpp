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
}

TeTextSyntax::~TeTextSyntax()
{
}

void TeTextSyntax::load(QString path)
{
    clear();

    // Load JSON document using QtJSON with Valijson helper function
    QJsonValue jsonSchemaDoc;
    if (!valijson::utils::loadDocument("mySchema.json", jsonSchemaDoc)) {
        // Failed to load schema document
    }
    QJsonValue jsonDoc;
    if (!valijson::utils::loadDocument("myTarget.json", jsonDoc)) {
        // Failed to load target document
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
    }


}

void TeTextSyntax::save(QString path)
{
}

void TeTextSyntax::clear()
{
    m_keywords.clear();
    m_regexes.clear();
    m_regions.clear();
}

int TeTextSyntax::addSyntaxKeywords(const SyntaxKeywords& keyword)
{
    //validation
    for (auto&& key : keyword.keywords) {
        if (key.length() > 30) {
            return -1;
        }
    }
    m_keywords.append(keyword);
    return m_keywords.count();
}

int TeTextSyntax::addSyntaxRegex(const SyntaxRegex& regex)
{
    if (regex.regex.pattern().length() > 200) {
        return -1;
    }
    return m_regexes.count();
}

int TeTextSyntax::addSyntaxRegion(const SyntaxRegion& region)
{
    if (region.startRegex.pattern().length() > 200) {
        return -1;
    }
    if (region.endRegex.pattern().length() > 200) {
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
