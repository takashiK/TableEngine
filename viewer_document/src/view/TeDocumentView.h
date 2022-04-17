#pragma once
#include <QPlainTextEdit>

class TeDocumentView :
    public QPlainTextEdit
{
    Q_OBJECT

public:
    TeDocumentView(QWidget* parent = nullptr);
};

