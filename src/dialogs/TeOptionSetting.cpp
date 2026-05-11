/****************************************************************************
**
** Copyright (C) 2021 Takashi Kuwabara.
** Contact: laffile@gmail.com
**
**  This program is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 2 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program.  If not, see <https://www.gnu.org/licenses/>.
**
****************************************************************************/

#include "TeOptionSetting.h"
#include "TeSettings.h"
#include "TeSelectPathDialog.h"
#include "TeFontDialog.h"
#include "../widgets/TeFileListView.h"

#include <QApplication>
#include <QColorDialog>
#include <QRegularExpression>
#include <QSettings>
#include <QDir>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QCheckBox>
#include <QGroupBox>
#include <QRadioButton>
#include <QLineEdit>
#include <QPushButton>
#include <QGridLayout>
#include <QLabel>

/**
 * @file TeOptionSetting.cpp
 * @brief Declaration of TeOptionSetting.
 * @ingroup dialogs
 */

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------
namespace {

/**
 * @brief Extracts the last CSS hex colour value for a given selector+property.
 *
 * Scans the merged stylesheet text (qApp->styleSheet()) for the last rule
 * block matching @p selector, then looks for @p property inside that block.
 * Only hex colours (#RGB / #RRGGBB / #AARRGGBB) are recognised.
 *
 * Sub-element rules such as "QListView::item:selected" are NOT reflected in
 * QPalette by Qt's polish() mechanism, so we must parse the CSS directly to
 * recover those colours.
 *
 * @return A valid QColor when found, or QColor{} otherwise.
 */
static QColor colorFromCss(const QString& css,
                           const QString& selector,
                           const QString& property)
{
	QColor result;
	int pos = 0;
	while (true) {
		int selPos = css.indexOf(selector, pos);
		if (selPos == -1)
			break;
		// Ensure the match is a whole token (not a substring of a longer selector)
		const int afterSel = selPos + selector.length();
		if (afterSel < css.length()) {
			const QChar next = css.at(afterSel);
			if (next.isLetterOrNumber() || next == QLatin1Char('_') || next == QLatin1Char('-')) {
				pos = afterSel;
				continue;
			}
		}
		int braceOpen = css.indexOf(QLatin1Char('{'), afterSel);
		if (braceOpen == -1)
			break;
		// Reject if there is a non-whitespace character between selector end and '{'
		if (!css.mid(afterSel, braceOpen - afterSel).trimmed().isEmpty()) {
			pos = afterSel;
			continue;
		}
		int braceClose = css.indexOf(QLatin1Char('}'), braceOpen);
		if (braceClose == -1)
			break;
		const QString block = css.mid(braceOpen + 1, braceClose - braceOpen - 1);
		QRegularExpression propRe(
			QRegularExpression::escape(property) +
			QLatin1String(R"(\s*:\s*(#[0-9a-fA-F]{3,8})\s*;?)"),
			QRegularExpression::CaseInsensitiveOption);
		const auto m = propRe.match(block);
		if (m.hasMatch()) {
			const QColor c(m.captured(1));
			if (c.isValid())
				result = c; // keep scanning — last rule wins (CSS cascade)
		}
		pos = braceClose + 1;
	}
	return result;
}

} // anonymous namespace


TeOptionSetting::TeOptionSetting(QWidget *parent, TeFileListView* p_listView)
	: QDialog(parent)
	, mp_listView(p_listView)
{
	//Helpボタン削除
	Qt::WindowFlags flags = windowFlags();
	setWindowFlags(flags & ~Qt::WindowContextHelpButtonHint);
	setWindowTitle(tr("Option"));
	setMinimumWidth(380);

	QVBoxLayout* layout = new QVBoxLayout();

	QTabWidget* tab = new QTabWidget();
	tab->addTab(createPageStartup(), tr("Startup"));
	tab->addTab(createPageFolder(), tr("Folder"));
	tab->addTab(createPageView(), tr("View"));

	layout->addWidget(tab);

	//OK Cancelボタン登録
	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	buttonBox->setCenterButtons(true);
	connect(buttonBox, &QDialogButtonBox::accepted, this, &TeOptionSetting::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &TeOptionSetting::reject);
	layout->addWidget(buttonBox);

	setLayout(layout);
}

TeOptionSetting::~TeOptionSetting()
{
}

void TeOptionSetting::updateSettings()
{
	QSettings settings;
	for (auto item = m_option.begin(); item != m_option.end(); ++item) {
		settings.setValue(item.key(),item.value());
	}
}

void TeOptionSetting::storeDefaultSettings(bool force)
{
	QSettings settings;
	if (settings.contains(SETTING_STARTUP)) {
		if (!force) {
			return;
		}
		settings.remove(SETTING_STARTUP);
	}

#define SETTING( str, default_value)  settings.setValue( str , settings.value( str , default_value ))

	SETTING(SETTING_STARTUP_MultiInstance, true);
	SETTING(SETTING_STARTUP_InitialFolderMode, TeSettings::INIT_FOLDER_MODE_SELECTED);
	SETTING(SETTING_STARTUP_InitialFolder, QDir::rootPath() );

#undef SETTING
}

void TeOptionSetting::accept()
{
	// Folder appearance is persisted directly via TeFolderAppearance::toSettings().
	m_folderAppearance.toSettings();

	updateSettings();
	QDialog::accept();
}

QWidget * TeOptionSetting::createPageStartup()
{
	QSettings settings;
	QWidget* page = new QWidget();
	QVBoxLayout* layout = new QVBoxLayout();

	QCheckBox* cbox = new QCheckBox(tr("Allow multi instance."));
	cbox->setChecked(settings.value(SETTING_STARTUP_MultiInstance).toBool());
	connect(cbox, &QCheckBox::stateChanged, [this](int state) { m_option[SETTING_STARTUP_MultiInstance] =  (state == Qt::Checked); });
	layout->addWidget(cbox);

	QGroupBox* groupBox = new QGroupBox(tr("Startup Folder"));
	QVBoxLayout* boxLayout = new QVBoxLayout();

	QRadioButton* radio = new QRadioButton(tr("Hold previous state."));
	radio->setChecked(settings.value(SETTING_STARTUP_InitialFolderMode).toInt() == TeSettings::INIT_FOLDER_MODE_PREVIOUS);
	connect(radio, &QRadioButton::clicked, [this](bool checked) { if (checked) m_option[SETTING_STARTUP_InitialFolderMode] = TeSettings::INIT_FOLDER_MODE_PREVIOUS; });
	boxLayout->addWidget(radio);
	radio = new QRadioButton(tr("Selected Folder."));
	radio->setChecked(settings.value(SETTING_STARTUP_InitialFolderMode).toInt() == TeSettings::INIT_FOLDER_MODE_SELECTED);
	connect(radio, &QRadioButton::clicked, [this](bool checked) { if (checked) m_option[SETTING_STARTUP_InitialFolderMode] = TeSettings::INIT_FOLDER_MODE_SELECTED; });
	boxLayout->addWidget(radio);

	QHBoxLayout* editLayout = new QHBoxLayout();
	QLineEdit* edit = new QLineEdit(settings.value(SETTING_STARTUP_InitialFolder).toString());
	connect(edit, &QLineEdit::textChanged, [this](const QString& text) {m_option[SETTING_STARTUP_InitialFolder] = text; });
	editLayout->addWidget(edit);
	QPushButton* button = new QPushButton(tr("Find"));
	connect(button, &QPushButton::clicked, [edit,this](bool /*checked*/) 
	{
		TeSelectPathDialog dlg(this); 
		dlg.setTargetPath(edit->text());
		if (dlg.exec() == QDialog::Accepted) {
			edit->setText(dlg.targetPath());
		}
	});
	editLayout->addWidget(button);
	boxLayout->addLayout(editLayout);

	groupBox->setLayout(boxLayout);
	layout->addWidget(groupBox);

	page->setLayout(layout);
	return page;
}

QWidget * TeOptionSetting::createPageFolder()
{
	m_folderAppearance = loadFolderAppearance(true);

	// ---------------------------------------------------------------
	// Build UI
	// ---------------------------------------------------------------
	QWidget* page = new QWidget();
	QVBoxLayout* layout = new QVBoxLayout();

	QCheckBox* prioCheck = new QCheckBox(tr("Use priority stylesheet (ignore other settings)"));
	prioCheck->setChecked(m_folderAppearance.prio_stylesheet);
	connect(prioCheck, &QCheckBox::stateChanged, [this](int state) {
		m_folderAppearance.prio_stylesheet = (state == Qt::Checked);
	});
	layout->addWidget(prioCheck);

	QGroupBox* groupBox = new QGroupBox(tr("Colors"));
	QGridLayout* grid = new QGridLayout();

	// --- Normal row ---
	grid->addWidget(new QLabel(tr("Normal:")), 0, 0);
	QPushButton* normal = new QPushButton();
	normal->setFixedWidth(200);
	grid->addWidget(normal, 0, 1);
	// Load setting from TeFileListView's palette and css.
	QPushButton* resetButton = new QPushButton(tr("Reset"));
	grid->addWidget(resetButton, 0, 2);

	// --- Selected row ---
	grid->addWidget(new QLabel(tr("Selected:")), 1, 0);
	QPushButton* selected = new QPushButton();
	selected->setFixedWidth(200);
	grid->addWidget(selected, 1, 1);

	// --- Accent colour ---
	grid->addWidget(new QLabel(tr("Accent:")), 1, 2);
	QPushButton* accent = new QPushButton();
	accent->setFixedWidth(50);
	accent->setStyleSheet(QString("background-color: %1").arg(m_folderAppearance.accentColor.name()));
	connect(accent, &QPushButton::clicked, [this, accent](bool) {
		QColor color = QColorDialog::getColor(m_folderAppearance.accentColor, this, tr("Select Accent Color"));
		if (color.isValid()) {
			m_folderAppearance.accentColor = color;
			accent->setStyleSheet(QString("background-color: %1").arg(color.name()));
		}
	});
	grid->addWidget(accent, 1, 3);

	// --- Focused row ---
	grid->addWidget(new QLabel(tr("Focused:")), 2, 0);
	QPushButton* focused = new QPushButton();
	focused->setFixedWidth(200);
	grid->addWidget(focused, 2, 1);

	// --- Focus prioritize ---
	grid->addWidget(new QLabel(tr("Prioritize:")), 2, 2);
	QCheckBox* prioritize = new QCheckBox();
	prioritize->setChecked(m_folderAppearance.focusPriority == TeFolderAppearance::FocusFirst);
	connect(prioritize, &QCheckBox::toggled, [this](bool checked) {
		m_folderAppearance.focusPriority = checked
			? TeFolderAppearance::FocusFirst : TeFolderAppearance::SelectedFirst;
	});
	grid->addWidget(prioritize, 2, 3);
	groupBox->setLayout(grid);
	layout->addWidget(groupBox);


	// Helper: reflect current font/colour state onto a button's label and style.
	auto updateBtnPreview = [normal, selected, focused](const TeFolderAppearance& a) {
		QFont f = a.font;
		normal->setText(f.family() + QString(" %1pt").arg(f.pointSize()));
		normal->setStyleSheet(QString("color: %1; background-color: %2").arg(a.normalFg.name(), a.normalBg.name()));
		selected->setText(f.family() + QString(" %1pt").arg(f.pointSize()));
		selected->setStyleSheet(QString("color: %1; background-color: %2").arg(a.selectedFg.name(), a.selectedBg.name()));
		focused->setText(f.family() + QString(" %1pt").arg(f.pointSize()));
		focused->setStyleSheet(QString("color: %1; background-color: %2").arg(a.normalFg.name(), a.focusBg.name()));
	};

	updateBtnPreview(m_folderAppearance);
	connect(normal, &QPushButton::clicked, [this, normal, updateBtnPreview](bool) {
		TeFontDialog dlg;
		QFont f = m_folderAppearance.font;
		dlg.setFont(f);
		dlg.setColor(m_folderAppearance.normalFg);
		dlg.setBackgroundColor(m_folderAppearance.normalBg);
		if (dlg.exec() == QDialog::Accepted) {
			const QFont result = dlg.font();
			m_folderAppearance.font.setFamily(result.family());
			m_folderAppearance.font.setPointSize(result.pointSize());
			m_folderAppearance.normalFg   = dlg.color();
			m_folderAppearance.normalBg   = dlg.backgroundColor();
			updateBtnPreview(m_folderAppearance);
		}
	});
	connect(selected, &QPushButton::clicked, [this, selected, updateBtnPreview](bool) {
		TeFontDialog dlg;
		QFont f = m_folderAppearance.font;
		dlg.setFont(f);
		dlg.setColor(m_folderAppearance.selectedFg);
		dlg.setBackgroundColor(m_folderAppearance.selectedBg);
		if (dlg.exec() == QDialog::Accepted) {
			const QFont result = dlg.font();
			m_folderAppearance.font.setFamily(result.family());
			m_folderAppearance.font.setPointSize(result.pointSize());
			m_folderAppearance.selectedFg   = dlg.color();
			m_folderAppearance.selectedBg   = dlg.backgroundColor();
			updateBtnPreview(m_folderAppearance);
		}
	});
	connect(focused, &QPushButton::clicked, [this, focused, updateBtnPreview](bool) {
		TeFontDialog dlg;
		QFont f = m_folderAppearance.font;
		dlg.setFont(f);
		dlg.setColor(m_folderAppearance.normalFg);
		dlg.setBackgroundColor(m_folderAppearance.focusBg);
		if (dlg.exec() == QDialog::Accepted) {
			const QFont result = dlg.font();
			m_folderAppearance.font.setFamily(result.family());
			m_folderAppearance.font.setPointSize(result.pointSize());
			m_folderAppearance.focusBg   = dlg.backgroundColor();
			updateBtnPreview(m_folderAppearance);
		}
	});

	// --- Reset button ---
	connect(resetButton, &QPushButton::clicked, [this, normal, selected, focused, accent, prioritize, updateBtnPreview](bool) {
		m_folderAppearance = loadFolderAppearance(false);
		updateBtnPreview(m_folderAppearance);
		accent->setStyleSheet(QString("background-color: %1").arg(m_folderAppearance.accentColor.name()));
		prioritize->setChecked(m_folderAppearance.focusPriority == TeFolderAppearance::FocusFirst);
	});

	page->setLayout(layout);
	return page;
}

QWidget * TeOptionSetting::createPageView()
{
	QWidget* page = new QWidget();
	QVBoxLayout* layout = new QVBoxLayout();



	page->setLayout(layout);
	return page;
}

TeFolderAppearance TeOptionSetting::loadFolderAppearance(bool useSettings) const
{
	TeFolderAppearance result;

	// Read explicitly saved settings.  Fields not present in QSettings are
	// returned with sentinel values (invalid QColor{}, fontSize==-1, empty string).
	TeFolderAppearance sa;
	if (useSettings) {
		sa = TeFolderAppearance::fromSettings();
	}

	result.prio_stylesheet = sa.prio_stylesheet;

	// Widget-level stylesheet rules (QListView { color: background-color: selection-color: })
	// are propagated to the palette via Qt's polish() and ARE readable from palette().
	// Sub-element rules (QListView::item:selected, QListView::item:focus) are applied only
	// during paint and are NOT reflected in the palette.  We therefore parse the merged
	// stylesheet string directly for those values.
	const QPalette pal      = mp_listView ? mp_listView->palette() : QApplication::palette();
	const QFont defaultFont = mp_listView ? mp_listView->font()    : QApplication::font();
	const QString css       = qApp->styleSheet();

	// ---------------------------------------------------------------
	// Populate result:
	//   priority 1: explicitly saved QSettings value  (sa.*  is valid)
	//   priority 2: color parsed from merged stylesheet CSS
	//   priority 3: system palette fallback (last resort)
	// ---------------------------------------------------------------
	// font: fromSettings() leaves family empty / pointSize -1 when not saved
	result.font = defaultFont;
	if (!sa.font.family().isEmpty())
		result.font.setFamily(sa.font.family());
	if (sa.font.pointSize() > 0)
		result.font.setPointSize(sa.font.pointSize());

	// Normal — QPalette::Text is reliably polished from the widget-level "color:" CSS rule
	result.normalFg   = sa.normalFg.isValid()
		? sa.normalFg : pal.color(QPalette::Normal, QPalette::Text);
	result.normalBg   = sa.normalBg.isValid()
		? sa.normalBg : pal.color(QPalette::Normal, QPalette::Base);

	// Selected — fg comes from "selection-color:" in QListView {}; bg from sub-element rule
	if (sa.selectedFg.isValid()) {
		result.selectedFg = sa.selectedFg;
	} else {
		const QColor css_fg = colorFromCss(css, QStringLiteral("QListView"),
		                                        QStringLiteral("selection-color"));
		result.selectedFg = css_fg.isValid()
			? css_fg : pal.color(QPalette::Normal, QPalette::HighlightedText);
	}
	if (sa.selectedBg.isValid()) {
		result.selectedBg = sa.selectedBg;
	} else {
		const QColor css_bg = colorFromCss(css, QStringLiteral("QListView::item:selected"),
		                                        QStringLiteral("background-color"));
		result.selectedBg = css_bg.isValid()
			? css_bg : pal.color(QPalette::Normal, QPalette::Highlight);
	}

	// Focus — fg equals normal text color (focus only changes background);
	// bg comes from the sub-element rule.
	if (sa.focusBg.isValid()) {
		result.focusBg = sa.focusBg;
	} else {
		const QColor css_bg = colorFromCss(css, QStringLiteral("QListView::item:focus"),
		                                        QStringLiteral("background-color"));
		result.focusBg = css_bg.isValid()
			? css_bg : pal.color(QPalette::Active, QPalette::Highlight);
	}

	// Accent — use the platform accent color exposed by QPalette::accent()
	result.accentColor = sa.accentColor.isValid()
		? sa.accentColor : pal.accent().color();

	// focusPriority is carried over directly from fromSettings() (enum value)
	result.focusPriority = sa.focusPriority;

	return result;
}
