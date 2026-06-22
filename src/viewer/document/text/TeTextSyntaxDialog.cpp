#include "TeTextSyntaxDialog.h"

#include "TeTextSyntaxHighlighter.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QScrollArea>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include <QStyle>

/**
 * @file TeTextSyntaxDialog.cpp
 * @brief Declaration of TeTextSyntaxDialog.
 * @ingroup viewer
 */


TeTextSyntaxDialog::TeTextSyntaxDialog(QWidget* parent) : QDialog(parent)
{
    QVBoxLayout* layout = new QVBoxLayout();

    // Syntax list
    QHBoxLayout* groupLayout = new QHBoxLayout();
    QLabel* label = new QLabel(tr("Rule name:"));
    groupLayout->addWidget(label);
    QComboBox* combo = new QComboBox();
    groupLayout->addWidget(combo);
    groupLayout->setStretch(1, 1);
    QPushButton* addBtn = new QPushButton(tr("Add"));
    groupLayout->addWidget(addBtn);
    QPushButton* delBtn = new QPushButton(tr("Del"));
    groupLayout->addWidget(delBtn);

    layout->addLayout(groupLayout);

    // Syntax Body
    QGridLayout* bodyLayout = new QGridLayout();

    // Related file suffix list
    QLabel* suffixLabel = new QLabel(tr("suffixes:"));
    bodyLayout->addWidget(suffixLabel, 0, 0);
    
    QListWidget* suffixList = new QListWidget();
    suffixList->setSelectionMode(QAbstractItemView::SingleSelection);
    // Constrain width to roughly 6 characters; the default QListWidget size hint
    // requests about 6x the font height regardless of content, which makes this
    // non-stretched column wider than intended.
    {
        const int charCount = 12;
        const QFontMetrics fm = suffixList->fontMetrics();
        const int contentWidth = fm.averageCharWidth() * charCount;
        const int frame = suffixList->frameWidth() * 2;
        const int scrollBar = suffixList->style()->pixelMetric(QStyle::PM_ScrollBarExtent);
        suffixList->setMaximumWidth(contentWidth + frame + scrollBar);
    }
    bodyLayout->addWidget(suffixList, 1, 0);
    bodyLayout->setRowStretch(1, 1);

    // Syntax details
    QLabel* detailLabel = new QLabel(tr("Syntax details:"));
    bodyLayout->addWidget(detailLabel, 0, 1);

    QScrollArea* scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    QVBoxLayout* symbolLayout = new QVBoxLayout();

    // Symbols
    QGroupBox* symbolGroup = new QGroupBox(tr("Symbols"));

    symbolLayout->addWidget(symbolGroup);

    // Syntaxs
    QGroupBox* syntaxGroup = new QGroupBox(tr("Syntaxs"));

    symbolLayout->addWidget(syntaxGroup);

    // Regions
    QGroupBox* regionGroup = new QGroupBox(tr("Regions"));

    symbolLayout->addWidget(regionGroup);

    scrollArea->setLayout(symbolLayout);

    bodyLayout->addWidget(scrollArea, 1, 1);
    bodyLayout->setColumnStretch(1, 1);
    layout->addLayout(bodyLayout);

    setLayout(layout);
}

TeTextSyntaxDialog::~TeTextSyntaxDialog()
{
}

void TeTextSyntaxDialog::setSyntaxHighlighter(TeTextSyntaxHighlighter *highlighter)
{
    mp_textHighlighter = highlighter;
}
