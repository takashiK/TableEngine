#pragma once

#include "TeTextSyntax.h"

class TeTextSyntaxLoader
{
public:
    TeTextSyntaxLoader();
    virtual ~TeTextSyntaxLoader();

    void clear();

    bool loadAll();
    bool saveAll();

    static bool loadSyntax(const QString& path, TeTextSyntax* p_textSyntax, bool append=false);
    static bool saveSyntax(const QString& path, const TeTextSyntax& textSyntax);

    int addEntry(const QString& title, const QString& path, bool overwrite=false);
    int addEntry(const QString& title, const TeTextSyntax& syntax, bool overwrite=false);
    int delEntry(const QString& title);

    TeTextSyntax entry(const QString& title);
    QStringList titles();

    int addRelation(const QString& suffix, const QString& title, bool overwrite=false);
    int delRelation(const QString& suffix);
    TeTextSyntax relatedEntry(const QString& suffix);
    const QMap<QString, QString>& relations();

private:
    struct Syntax {
        Syntax(): isUpdate(false) {}
        bool isUpdate;
        TeTextSyntax textSyntax;
    };

    QMap<QString, Syntax> m_syntaxes;
    QMap<QString, QString> m_relations;
    bool m_isUpdateRelation;
};

