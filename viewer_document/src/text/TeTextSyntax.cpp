#include "TeTextSyntax.h"

TeTextSyntax::TeTextSyntax()
{
    m_update_keywords = false;
}


TeTextSyntax::~TeTextSyntax()
{
}

namespace {
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
    m_regexes.append(regex);
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
    m_regions.append(region);
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
