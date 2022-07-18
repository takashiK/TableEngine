#pragma once
#include <qobject.h>
#include "TeTextSyntax.h"

class TeTextSyntaxLoader :
    public QObject
{
public:
    TeTextSyntaxLoader();
    virtual ~TeTextSyntaxLoader();

    int loadAll();
    int saveAll();

    int addEntry(const QString& title, const QString& path);
    int addEntry(const QString& title, const TeTextSyntax& syntax);
    int delEntry(const QString& title);

    const TeTextSyntax& entry(const QString& title);
    const QStringList titles();

    int addRelation(const QString& suffix, const QString& title);
    int delRelation(const QString& suffix);
    const TeTextSyntax& relatedEntry(const QString& suffix);
    const QMap<QString, TeTextSyntax>& relations();

private:
    struct Syntax {
        bool isUpdate;
        TeTextSyntax textSyntax;
    };

    QMap<QString, Syntax> m_syntaxes;
    QMap<QString, QString> m_relations;
    bool m_isUpdateRelation;
};

