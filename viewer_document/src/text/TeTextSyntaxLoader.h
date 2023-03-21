#pragma once
#include <qobject.h>
#include "TeTextSyntax.h"

class TeTextSyntaxLoader :
    public QObject
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

    const TeTextSyntax& entry(const QString& title);
    const QStringList& titles();

    int addRelation(const QString& suffix, const QString& title, bool overwrite=false);
    int delRelation(const QString& suffix);
    const TeTextSyntax& relatedEntry(const QString& suffix);
    const QMap<QString, QString>& relations();

private:
    struct Syntax {
        bool isUpdate;
        TeTextSyntax textSyntax;
    };

    QMap<QString, Syntax> m_syntaxes;
    QMap<QString, QString> m_relations;
    bool m_isUpdateRelation;
};

