#include "TeTextSyntax.h"

/**
 * @file TeTextSyntax.cpp
 * @brief Declaration of TeTextSyntax.
 * @ingroup viewer
 */


TeTextSyntax::TeTextSyntax()
{
}


TeTextSyntax::~TeTextSyntax()
{
}

namespace {
    QRegularExpression listToRegularExpression(const QStringList& list) {
        QString packed;
        if (list.size() > 0) {
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
    if (keyword.keywords.size() == 0)
        return -1;

    //validation
    for (auto&& key : keyword.keywords) {
        if (key.size() > 30) {
            return -1;
        }
    }
    m_update_keywords = true;
    m_keywords.append(keyword);
    return m_keywords.size();
}

int TeTextSyntax::addSyntaxRegex(const SyntaxRegex& regex)
{
    if (regex.regex.pattern().size() > 200 || !regex.regex.isValid()) {
        return -1;
    }
    m_regexes.append(regex);
    return m_regexes.size();
}

int TeTextSyntax::addSyntaxRegion(const SyntaxRegion& region)
{
    if (region.startRegex.pattern().size() > 200 || !region.startRegex.isValid()) {
        return -1;
    }
    if (region.endRegex.pattern().size() > 200 || !region.endRegex.isValid()) {
        return -1;
    }
    m_regions.append(region);
    return m_regions.size();
}

bool TeTextSyntax::removeSyntaxKeywords(int index)
{
    if (index >= 0 && index < m_keywords.size()) {
        m_keywords.removeAt(index);
        return true;
    }
    return false;
}

bool TeTextSyntax::removeSyntaxRegex(int index)
{
    if (index >= 0 && index < m_regexes.size()) {
        m_regexes.removeAt(index);
        return true;
    }
    return false;
}

bool TeTextSyntax::removeSyntaxRegion(int index)
{
    if (index >= 0 && index < m_regions.size()) {
        m_regions.removeAt(index);
        return true;
    }
    return false;
}

bool TeTextSyntax::moveSyntaxKeywords(int fromIndex, int toIndex)
{
    if (fromIndex >= 0 && fromIndex < m_keywords.size() && toIndex >= 0 && toIndex < m_keywords.size()) {
        m_keywords.move(fromIndex, toIndex);
        return true;
    }
    return false;
}

bool TeTextSyntax::moveSyntaxRegex(int fromIndex, int toIndex)
{
    if (fromIndex >= 0 && fromIndex < m_regexes.size() && toIndex >= 0 && toIndex < m_regexes.size()) {
        m_regexes.move(fromIndex, toIndex);
        return true;
    }
    return false;
}

bool TeTextSyntax::moveSyntaxRegion(int fromIndex, int toIndex)
{
    if (fromIndex >= 0 && fromIndex < m_regions.size() && toIndex >= 0 && toIndex < m_regions.size()) {
        m_regions.move(fromIndex, toIndex);
        return true;
    }
    return false;
}
