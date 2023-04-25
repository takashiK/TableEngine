#pragma once

#include <QObject>
#include <QFileInfo>

class QTextCodec;

class TeDocument :
    public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString content READ content)

public:
    TeDocument(QObject* parent = nullptr);
    bool load(const QString& path, QString codecName=QString());
    bool reload(const QString& codecName=QString());
    void clear();

    const QStringList& setCodecList(const QStringList& codecList);
    const QStringList& codecList() const { return m_codecList; }

    const QFileInfo& fileinfo() const { return m_fileInfo; }
    const QString& codecName() const { return m_codecName; }

    const QString& content() const { return m_content; }

signals:
    void codecChanged(const QString& codecName);
    void codecListChanged(const QStringList& codecList);

private:
    QFileInfo m_fileInfo;
    QString m_content;
    QString m_codecName;
    QStringList m_codecList;
};

