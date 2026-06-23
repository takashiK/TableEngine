#include "TeTextSyntaxDialog.h"

#include "TeTextSyntaxHighlighter.h"

#include "panel/TeTextPanelList.h"
#include "panel/TeTextPanelSymbol.h"
#include "panel/TeTextPanelSyntax.h"
#include "panel/TeTextPanelRegion.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QListWidget>
#include <QScrollArea>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
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
    mp_ruleCombo = new QComboBox();
    groupLayout->addWidget(mp_ruleCombo);
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

    mp_suffixList = new QListWidget();
    mp_suffixList->setSelectionMode(QAbstractItemView::SingleSelection);
    // Constrain width to roughly 6 characters; the default QListWidget size hint
    // requests about 6x the font height regardless of content, which makes this
    // non-stretched column wider than intended.
    {
        const int charCount = 12;
        const QFontMetrics fm = mp_suffixList->fontMetrics();
        const int contentWidth = fm.averageCharWidth() * charCount;
        const int frame = mp_suffixList->frameWidth() * 2;
        const int scrollBar = mp_suffixList->style()->pixelMetric(QStyle::PM_ScrollBarExtent);
        mp_suffixList->setMaximumWidth(contentWidth + frame + scrollBar);
    }
    bodyLayout->addWidget(mp_suffixList, 1, 0);
    bodyLayout->setRowStretch(1, 1);

    // Syntax details
    QLabel* detailLabel = new QLabel(tr("Syntax details:"));
    bodyLayout->addWidget(detailLabel, 0, 1);

    QScrollArea* scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);

    QWidget* detailWidget = new QWidget();
    QVBoxLayout* detailLayout = new QVBoxLayout(detailWidget);

    // Symbols
    mp_symbolList = new TeTextPanelList(tr("Symbols"));
    detailLayout->addWidget(mp_symbolList);

    QFrame* separator1 = new QFrame();
    separator1->setFrameShape(QFrame::HLine);
    separator1->setFrameShadow(QFrame::Sunken);
    detailLayout->addWidget(separator1);

    // Syntaxs
    mp_syntaxList = new TeTextPanelList(tr("Syntaxs"));
    detailLayout->addWidget(mp_syntaxList);

    QFrame* separator2 = new QFrame();
    separator2->setFrameShape(QFrame::HLine);
    separator2->setFrameShadow(QFrame::Sunken);
    detailLayout->addWidget(separator2);

    // Regions
    mp_regionList = new TeTextPanelList(tr("Regions"));
    detailLayout->addWidget(mp_regionList);

    detailLayout->addStretch(1);

    scrollArea->setWidget(detailWidget);

    bodyLayout->addWidget(scrollArea, 1, 1);
    bodyLayout->setColumnStretch(1, 1);
    layout->addLayout(bodyLayout);

    setLayout(layout);

    // New empty entries created via each group's "Add" button.
    connect(mp_symbolList, &TeTextPanelList::itemAdded, this, [this]() {
        mp_symbolList->addPanel(new TeTextPanelSymbol());
    });
    connect(mp_syntaxList, &TeTextPanelList::itemAdded, this, [this]() {
        mp_syntaxList->addPanel(new TeTextPanelSyntax());
    });
    connect(mp_regionList, &TeTextPanelList::itemAdded, this, [this]() {
        mp_regionList->addPanel(new TeTextPanelRegion());
    });

    // Load highlight definitions and populate the rule selector.
    m_loader.loadAll();
    mp_ruleCombo->addItems(m_loader.titles());

    connect(mp_ruleCombo, &QComboBox::currentTextChanged, this, &TeTextSyntaxDialog::rebuildForTitle);

    if (mp_ruleCombo->count() > 0) {
        rebuildForTitle(mp_ruleCombo->currentText());
    }
}

TeTextSyntaxDialog::~TeTextSyntaxDialog()
{
}

void TeTextSyntaxDialog::setSyntaxHighlighter(TeTextSyntaxHighlighter *highlighter)
{
    mp_textHighlighter = highlighter;
}

void TeTextSyntaxDialog::rebuildForTitle(const QString& title)
{
    // Suffixes associated with the selected rule.
    mp_suffixList->clear();
    const QMap<QString, QString>& relations = m_loader.relations();
    for (auto itr = relations.begin(); itr != relations.end(); ++itr) {
        if (itr.value() == title) {
            mp_suffixList->addItem(itr.key());
        }
    }

    // Entry panels for the selected rule.
    mp_symbolList->clearPanels();
    mp_syntaxList->clearPanels();
    mp_regionList->clearPanels();

    if (title.isEmpty()) {
        return;
    }

    TeTextSyntax syntax = m_loader.entry(title);

    for (const auto& keywords : syntax.keywords()) {
        TeTextPanelSymbol* panel = new TeTextPanelSymbol();
        panel->setData(keywords);
        mp_symbolList->addPanel(panel);
    }

    for (const auto& regex : syntax.regexes()) {
        TeTextPanelSyntax* panel = new TeTextPanelSyntax();
        panel->setData(regex);
        mp_syntaxList->addPanel(panel);
    }

    for (const auto& region : syntax.regions()) {
        TeTextPanelRegion* panel = new TeTextPanelRegion();
        panel->setData(region);
        mp_regionList->addPanel(panel);
    }
}
