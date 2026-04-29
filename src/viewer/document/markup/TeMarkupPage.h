#pragma once

#include <QWebEnginePage>

/**
 * @file TeMarkupPage.h
 * @brief Declaration of TeMarkupPage.
 * @ingroup viewer
 */


class TeMarkupPage :
    public QWebEnginePage
{
    Q_OBJECT

public:
    TeMarkupPage(QObject* parent = nullptr);


private:

};

