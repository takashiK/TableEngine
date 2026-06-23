#include "TeTextSyntaxDialog.h"

#include "TeTextSyntaxHighlighter.h"

#include "panel/TeTextPanelList.h"
#include "panel/TeTextPanelSymbol.h"
#include "panel/TeTextPanelSyntax.h"
#include "panel/TeTextPanelRegion.h"
#include "viewer/document/TeDocumentSettings.h"

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
#include <QDialogButtonBox>
#include <QInputDialog>
#include <QMessageBox>
#include <QLineEdit>
#include <QSettings>

/**
 * @file TeTextSyntaxDialog.cpp
 * @brief Declaration of TeTextSyntaxDialog.
 * @ingroup viewer
 */


TeTextSyntaxDialog::TeTextSyntaxDialog(QWidget* parent) : QDialog(parent)
{
    QSettings settings;
    int width = settings.value(SETTING_TEXT_HIGHLIGHT_DIALOG_WIDTH, 800).toInt();
    int height = settings.value(SETTING_TEXT_HIGHLIGHT_DIALOG_HEIGHT, 600).toInt();
    resize(width, height);

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

    QVBoxLayout* suffixLayout = new QVBoxLayout();

    mp_suffixList = new QListWidget();
    mp_suffixList->setSelectionMode(QAbstractItemView::SingleSelection);
    // Let the list follow the Add/Del button row width below instead of the
    // default QListWidget size hint (~6x font height), which would otherwise
    // make this non-stretched column wider than the buttons.
    mp_suffixList->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Expanding);
    suffixLayout->addWidget(mp_suffixList);

    // Add / delete buttons for the suffix relations of the current rule.
    QHBoxLayout* suffixBtnLayout = new QHBoxLayout();
    QPushButton* suffixAddBtn = new QPushButton(tr("Add"));
    QPushButton* suffixDelBtn = new QPushButton(tr("Del"));
    suffixBtnLayout->addWidget(suffixAddBtn);
    suffixBtnLayout->addWidget(suffixDelBtn);

    suffixLayout->addLayout(suffixBtnLayout);
    bodyLayout->addLayout(suffixLayout, 1, 0);
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

    // OK / Cancel buttons.
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    layout->addWidget(buttonBox);

    setLayout(layout);

    connect(buttonBox, &QDialogButtonBox::accepted, this, [this]() {
        commitCurrent();
        m_loader.saveAll();
        accept();
    });
    connect(buttonBox, &QDialogButtonBox::rejected, this, &TeTextSyntaxDialog::reject);

    // Rule add / delete buttons.
    connect(addBtn, &QPushButton::clicked, this, &TeTextSyntaxDialog::onAddEntry);
    connect(delBtn, &QPushButton::clicked, this, &TeTextSyntaxDialog::onDelEntry);

    // Suffix relation add / delete buttons.
    connect(suffixAddBtn, &QPushButton::clicked, this, &TeTextSyntaxDialog::onAddSuffix);
    connect(suffixDelBtn, &QPushButton::clicked, this, &TeTextSyntaxDialog::onDelSuffix);

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

    // Commit the currently displayed rule before switching to another one.
    connect(mp_ruleCombo, &QComboBox::currentTextChanged, this, [this](const QString& title) {
        if (m_suppressComboSignal) {
            return;
        }
        commitCurrent();
        m_currentTitle = title;
        rebuildForTitle(title);
    });

    if (mp_ruleCombo->count() > 0) {
        m_currentTitle = mp_ruleCombo->currentText();
        rebuildForTitle(m_currentTitle);
    }

}

TeTextSyntaxDialog::~TeTextSyntaxDialog()
{
}

void TeTextSyntaxDialog::setSyntaxHighlighter(TeTextSyntaxHighlighter *highlighter)
{
    mp_textHighlighter = highlighter;
}

void TeTextSyntaxDialog::done(int result)
{
    // Persist the dialog size on every close path (OK / Cancel / window close).
    const QSize size = isMaximized() ? normalGeometry().size() : this->size();
    if (size.isValid() && size.width() > 0 && size.height() > 0) {
        QSettings settings;
        settings.setValue(SETTING_TEXT_HIGHLIGHT_DIALOG_WIDTH, size.width());
        settings.setValue(SETTING_TEXT_HIGHLIGHT_DIALOG_HEIGHT, size.height());
    }

    QDialog::done(result);
}

void TeTextSyntaxDialog::rebuildForTitle(const QString& title)
{
    // Suffixes associated with the selected rule.
    reloadSuffixList(title);

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

void TeTextSyntaxDialog::reloadSuffixList(const QString& title)
{
    mp_suffixList->clear();
    const QMap<QString, QString>& relations = m_loader.relations();
    for (auto itr = relations.begin(); itr != relations.end(); ++itr) {
        if (itr.value() == title) {
            mp_suffixList->addItem(itr.key());
        }
    }
}

TeTextSyntax TeTextSyntaxDialog::collectCurrentSyntax() const
{
    // Rebuild a TeTextSyntax from the current panel widgets. Iterating the
    // panels in display order naturally captures edits, additions and
    // reordering performed in the dialog.
    TeTextSyntax syntax;

    for (int i = 0; i < mp_symbolList->count(); ++i) {
        TeTextPanelSymbol* panel = static_cast<TeTextPanelSymbol*>(mp_symbolList->panelAt(i));
        syntax.addSyntaxKeywords(panel->data());
    }

    for (int i = 0; i < mp_syntaxList->count(); ++i) {
        TeTextPanelSyntax* panel = static_cast<TeTextPanelSyntax*>(mp_syntaxList->panelAt(i));
        syntax.addSyntaxRegex(panel->data());
    }

    for (int i = 0; i < mp_regionList->count(); ++i) {
        TeTextPanelRegion* panel = static_cast<TeTextPanelRegion*>(mp_regionList->panelAt(i));
        syntax.addSyntaxRegion(panel->data());
    }

    return syntax;
}

void TeTextSyntaxDialog::commitCurrent()
{
    if (m_currentTitle.isEmpty()) {
        return;
    }
    m_loader.addEntry(m_currentTitle, collectCurrentSyntax(), true);
}

void TeTextSyntaxDialog::onAddEntry()
{
    bool ok = false;
    const QString name = QInputDialog::getText(this, tr("Add rule"), tr("Rule name:"),
                                               QLineEdit::Normal, QString(), &ok).trimmed();
    if (!ok || name.isEmpty()) {
        return;
    }

    if (m_loader.titles().contains(name)) {
        QMessageBox::warning(this, tr("Add rule"), tr("A rule named \"%1\" already exists.").arg(name));
        return;
    }

    // Save the entry currently displayed before switching to the new one.
    commitCurrent();

    // Tentatively register an empty entry under the new name.
    m_loader.addEntry(name, TeTextSyntax());

    m_suppressComboSignal = true;
    mp_ruleCombo->addItem(name);
    mp_ruleCombo->setCurrentIndex(mp_ruleCombo->count() - 1);
    m_suppressComboSignal = false;

    m_currentTitle = name;
    rebuildForTitle(name);
}

void TeTextSyntaxDialog::onDelEntry()
{
    if (m_currentTitle.isEmpty()) {
        return;
    }

    const int index = mp_ruleCombo->currentIndex();

    // Drop the displayed entry without writing it back.
    m_loader.delEntry(m_currentTitle);

    m_suppressComboSignal = true;
    mp_ruleCombo->removeItem(index);

    // Keep the same index; when the last entry was removed, fall back to the
    // new last entry.
    int newIndex = index;
    if (newIndex >= mp_ruleCombo->count()) {
        newIndex = mp_ruleCombo->count() - 1;
    }
    if (newIndex >= 0) {
        mp_ruleCombo->setCurrentIndex(newIndex);
    }
    m_suppressComboSignal = false;

    m_currentTitle = (newIndex >= 0) ? mp_ruleCombo->currentText() : QString();
    rebuildForTitle(m_currentTitle);
}

void TeTextSyntaxDialog::onAddSuffix()
{
    if (m_currentTitle.isEmpty()) {
        return;
    }

    bool ok = false;
    const QString suffix = QInputDialog::getText(this, tr("Add suffix"), tr("Suffix:"),
                                                QLineEdit::Normal, QString(), &ok).trimmed();
    if (!ok || suffix.isEmpty()) {
        return;
    }

    // A suffix maps to exactly one rule; overwrite any existing mapping.
    m_loader.addRelation(suffix, m_currentTitle, true);
    reloadSuffixList(m_currentTitle);
}

void TeTextSyntaxDialog::onDelSuffix()
{
    QListWidgetItem* item = mp_suffixList->currentItem();
    if (!item) {
        return;
    }

    m_loader.delRelation(item->text());
    reloadSuffixList(m_currentTitle);
}
