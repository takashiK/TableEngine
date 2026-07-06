#include "TeToolDialog.h"
#include "TeSettings.h"
#include "utils/TeToolCommand.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QToolButton>
#include <QDialogButtonBox>
#include <QTabWidget>
#include <QScrollArea>
#include <QFrame>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QSettings>

/**
 * @file TeToolDialog.cpp
 * @brief Implementation of TeToolDialog.
 * @ingroup dialogs
 */

TeToolDialog::TeToolDialog(QWidget *parent)
	: QDialog(parent)
{
	setWindowTitle(tr("Tool Settings"));
	setMinimumWidth(500);
    setMinimumHeight(300);

	loadSettings();

	QVBoxLayout* mainLayout = new QVBoxLayout(this);

	QTabWidget* tab = new QTabWidget();
	tab->addTab(createPageGeneral(), tr("General"));
	tab->addTab(createPageUser(), tr("User"));
	mainLayout->addWidget(tab);

	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	buttonBox->setCenterButtons(true);
	connect(buttonBox, &QDialogButtonBox::accepted, this, &TeToolDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
	mainLayout->addWidget(buttonBox);

	setLayout(mainLayout);
}

TeToolDialog::~TeToolDialog()
{}

QWidget* TeToolDialog::createPageGeneral()
{
	QWidget* page = new QWidget();
	QGridLayout* layout = new QGridLayout(page);
	layout->setColumnStretch(1, 1);

	// Creates a command-path row plus an arguments row below it, spanning 2 grid rows.
	auto makeRow = [this, layout](int row, const QString& label, const QString& initialValue,
	                               QLineEdit** outPathEdit, QLineEdit** outArgsEdit) {
		QString initialPath, initialArgs;
		splitCommand(initialValue, initialPath, initialArgs);

		layout->addWidget(new QLabel(label), row, 0);
		QLineEdit* pathEdit = new QLineEdit(initialPath);
		layout->addWidget(pathEdit, row, 1);
		QToolButton* browseButton = new QToolButton();
		browseButton->setText("...");
		layout->addWidget(browseButton, row, 2);

		layout->addWidget(new QLabel(tr("Args :")), row + 1, 0);
		QLineEdit* argsEdit = new QLineEdit(initialArgs);
		layout->addWidget(argsEdit, row + 1, 1, 1, 2);

		connect(browseButton, &QToolButton::clicked, this, [this, pathEdit]() {
			const QString currentPath = pathEdit->text().trimmed();
			const QString startDir = currentPath.isEmpty() ? QString() : QFileInfo(currentPath).absolutePath();
			const QString path = QFileDialog::getOpenFileName(this, tr("Select Command"), startDir);
			if (path.isEmpty()) {
				return; // Cancel: leave the field untouched.
			}
			pathEdit->setText(path);
		});

		*outPathEdit = pathEdit;
		*outArgsEdit = argsEdit;
	};

	makeRow(0, tr("Text :"),   m_textTool,   &mp_textEdit,   &mp_textArgsEdit);
	makeRow(2, tr("Binary :"), m_binaryTool, &mp_binaryEdit, &mp_binaryArgsEdit);
	makeRow(4, tr("Image :"),  m_imageTool,  &mp_imageEdit,  &mp_imageArgsEdit);

	layout->setRowStretch(6, 1); // Add stretch to push rows to the top.

	auto connectTool = [this](QLineEdit* pathEdit, QLineEdit* argsEdit, QString* target) {
		auto update = [this, pathEdit, argsEdit, target]() {
			*target = combineCommand(pathEdit->text(), argsEdit->text());
			applyValidityStyle(pathEdit, isRowValid(pathEdit->text(), argsEdit->text()));
		};
		connect(pathEdit, &QLineEdit::textChanged, this, update);
		connect(argsEdit, &QLineEdit::textChanged, this, update);
	};
	connectTool(mp_textEdit,   mp_textArgsEdit,   &m_textTool);
	connectTool(mp_binaryEdit, mp_binaryArgsEdit, &m_binaryTool);
	connectTool(mp_imageEdit,  mp_imageArgsEdit,  &m_imageTool);

	return page;
}

QWidget* TeToolDialog::createPageUser()
{
	QWidget* page = new QWidget();
	QVBoxLayout* pageLayout = new QVBoxLayout(page);

	QWidget* container = new QWidget();
	mp_userLayout = new QVBoxLayout(container);
	mp_userLayout->addStretch(1);

	QScrollArea* scrollArea = new QScrollArea();
	scrollArea->setWidgetResizable(true);
	scrollArea->setWidget(container);
	pageLayout->addWidget(scrollArea, 1);

	for (const ToolSetting& tool : m_userTools) {
		QWidget* frame = createUserEntry(tool);
		mp_userLayout->insertWidget(mp_userLayout->count() - 1, frame);
	}

	QPushButton* addButton = new QPushButton(tr("Add"));
	connect(addButton, &QPushButton::clicked, this, [this]() { addUserEntry(); });
	pageLayout->addWidget(addButton);

	return page;
}

QWidget* TeToolDialog::createUserEntry(const ToolSetting& initial)
{
	QFrame* frame = new QFrame();
	frame->setFrameShape(QFrame::StyledPanel);

    QHBoxLayout* mainLayout = new QHBoxLayout(frame);

    QVBoxLayout* frameLayout = new QVBoxLayout();
    mainLayout->addLayout(frameLayout);

	QString initialPath, initialArgs;
	splitCommand(initial.command, initialPath, initialArgs);

	QHBoxLayout* row1 = new QHBoxLayout();
	row1->addWidget(new QLabel(tr("Command:")));
	QLineEdit* commandEdit = new QLineEdit(initialPath);
	row1->addWidget(commandEdit, 1);

    QToolButton* browseButton = new QToolButton();
    browseButton->setText("...");
    row1->addWidget(browseButton);
    connect(browseButton, &QToolButton::clicked, this, [this, commandEdit]() {
        const QString currentPath = commandEdit->text().trimmed();
        const QString startDir = currentPath.isEmpty() ? QString() : QFileInfo(currentPath).absolutePath();
        const QString path = QFileDialog::getOpenFileName(this, tr("Select Command"), startDir);
        if (path.isEmpty()) {
            return; // Cancel: leave the field untouched.
        }
        commandEdit->setText(path);
    });

    frameLayout->addLayout(row1);

	QHBoxLayout* row2 = new QHBoxLayout();
	row2->addWidget(new QLabel(tr("Args:")));
	QLineEdit* argsEdit = new QLineEdit(initialArgs);
	row2->addWidget(argsEdit, 1);
	frameLayout->addLayout(row2);

	QHBoxLayout* row3 = new QHBoxLayout();
	row3->addWidget(new QLabel(tr("Suffixes:")));
	QLineEdit* suffixEdit = new QLineEdit(initial.suffixes.join("; "));
	suffixEdit->setPlaceholderText(tr("suffix1; suffix2; suffix3"));
	row3->addWidget(suffixEdit, 1);
	frameLayout->addLayout(row3);

    QToolButton* removeButton = new QToolButton();
	removeButton->setIcon(QIcon(":/TableEngine/delete.png"));
    mainLayout->addWidget(removeButton);


	auto updateValidity = [this, commandEdit, argsEdit]() {
		applyValidityStyle(commandEdit, isRowValid(commandEdit->text(), argsEdit->text()));
	};
	connect(commandEdit, &QLineEdit::textChanged, this, updateValidity);
	connect(argsEdit, &QLineEdit::textChanged, this, updateValidity);
	connect(removeButton, &QToolButton::clicked, this, [this, frame]() { removeUserEntry(frame); });

	UserRow row;
	row.frame = frame;
	row.commandEdit = commandEdit;
	row.argsEdit = argsEdit;
	row.suffixEdit = suffixEdit;
	m_userRows.append(row);

	return frame;
}

void TeToolDialog::addUserEntry(const ToolSetting& initial)
{
	if (m_userRows.size() >= TeSettings::MAX_USER_TOOLS) {
		QMessageBox::information(this, tr("Tool Settings"),
			tr("Maximum number of user tools (%1) reached.").arg(TeSettings::MAX_USER_TOOLS));
		return;
	}
	QWidget* frame = createUserEntry(initial);
	mp_userLayout->insertWidget(mp_userLayout->count() - 1, frame);
}

void TeToolDialog::removeUserEntry(QWidget* frame)
{
	for (int i = 0; i < m_userRows.size(); ++i) {
		if (m_userRows.at(i).frame == frame) {
			mp_userLayout->removeWidget(frame);
			frame->deleteLater();
			m_userRows.removeAt(i);
			break;
		}
	}
}

void TeToolDialog::applyValidityStyle(QLineEdit* edit, bool valid) const
{
	edit->setStyleSheet(valid ? QString() : QStringLiteral("QLineEdit { border: 1px solid red; }"));
}

bool TeToolDialog::isRowValid(const QString& path, const QString& args) const
{
	const bool pathEmpty = path.trimmed().isEmpty();
	if (pathEmpty && args.trimmed().isEmpty()) {
		return true; // Not configured yet - treated as valid.
	}
	if (pathEmpty) {
		return false; // Arguments without a command path.
	}
	return TeToolCommand::isValidFormat(path);
}

void TeToolDialog::splitCommand(const QString& full, QString& path, QString& args)
{
	const QString trimmed = full.trimmed();
	path.clear();
	args.clear();
	if (trimmed.isEmpty()) {
		return;
	}

	int tokenEnd = 0;
	if (trimmed.at(0) == QLatin1Char('"')) {
		const int closingQuote = trimmed.indexOf(QLatin1Char('"'), 1);
		if (closingQuote < 0) {
			path = trimmed; // Unterminated quote: treat the whole string as the path.
			return;
		}
		path = trimmed.mid(1, closingQuote - 1);
		tokenEnd = closingQuote + 1;
	} else {
		const int firstSpace = trimmed.indexOf(QLatin1Char(' '));
		if (firstSpace < 0) {
			path = trimmed;
			return;
		}
		path = trimmed.left(firstSpace);
		tokenEnd = firstSpace;
	}
	args = trimmed.mid(tokenEnd).trimmed();
}

QString TeToolDialog::combineCommand(const QString& path, const QString& args)
{
	const QString trimmedPath = path.trimmed();
	const QString trimmedArgs = args.trimmed();
	if (trimmedPath.isEmpty()) {
		return trimmedArgs; // Edge case (invalid): arguments without a command path.
	}
	const QString quotedPath = trimmedPath.contains(QLatin1Char(' ')) ? QString("\"%1\"").arg(trimmedPath) : trimmedPath;
	return trimmedArgs.isEmpty() ? quotedPath : quotedPath + QLatin1Char(' ') + trimmedArgs;
}

QString TeToolDialog::ensureItemMacro(const QString& args)
{
	// Guards against forgetting to add a target-item macro: if none of %f/%F/%m/%M
	// is present, append %f (current item file name) as a sensible default.
	const QString trimmed = args.trimmed();
	static const QStringList itemMacros = { "%f", "%F", "%m", "%M" };
	for (const QString& macro : itemMacros) {
		if (trimmed.contains(macro)) {
			return trimmed;
		}
	}
	return trimmed.isEmpty() ? QStringLiteral("%f") : trimmed + QLatin1String(" %f");
}

void TeToolDialog::loadSettings()
{
	QSettings settings;
	m_textTool   = settings.value(SETTING_TOOLS_TEXT_EDIT).toString();
	m_imageTool  = settings.value(SETTING_TOOLS_IMAGE_EDIT).toString();
	m_binaryTool = settings.value(SETTING_TOOLS_BINARY_EDIT).toString();

	m_userTools.clear();
	settings.beginGroup(SETTING_TOOLS_USER);
	for (int i = 1; i <= TeSettings::MAX_USER_TOOLS; ++i) {
		const QString key = QString("tool%1").arg(i, 2, 10, QLatin1Char('0'));
		if (!settings.contains(key)) {
			continue;
		}
		const QStringList fields = settings.value(key).toString().split(QLatin1Char(';'));
		if (fields.isEmpty()) {
			continue;
		}
		ToolSetting tool;
		tool.command = fields.first().trimmed();
		for (int j = 1; j < fields.size(); ++j) {
			const QString suffix = fields.at(j).trimmed();
			if (!suffix.isEmpty()) {
				tool.suffixes.append(suffix);
			}
		}
		m_userTools.append(tool);
	}
	settings.endGroup();
}

void TeToolDialog::saveSettings()
{
	QSettings settings;
	settings.setValue(SETTING_TOOLS_TEXT_EDIT, m_textTool);
	settings.setValue(SETTING_TOOLS_IMAGE_EDIT, m_imageTool);
	settings.setValue(SETTING_TOOLS_BINARY_EDIT, m_binaryTool);

	settings.beginGroup(SETTING_TOOLS_USER);
	settings.remove(""); // Clear all existing tool entries before rewriting.
	int idx = 1;
	for (const ToolSetting& tool : m_userTools) {
		if (idx > TeSettings::MAX_USER_TOOLS) {
			break;
		}
		QStringList fields;
		fields << tool.command;
		fields.append(tool.suffixes);
		settings.setValue(QString("tool%1").arg(idx, 2, 10, QLatin1Char('0')), fields.join(QLatin1String("; ")));
		++idx;
	}
	settings.endGroup();
}

void TeToolDialog::accept()
{
	struct GeneralRow { QLineEdit* pathEdit; QLineEdit* argsEdit; };
	const QList<GeneralRow> generalRows = {
		{ mp_textEdit,   mp_textArgsEdit   },
		{ mp_binaryEdit, mp_binaryArgsEdit },
		{ mp_imageEdit,  mp_imageArgsEdit  },
	};
	for (const GeneralRow& row : generalRows) {
		if (!isRowValid(row.pathEdit->text(), row.argsEdit->text())) {
			QMessageBox::warning(this, tr("Tool Settings"),
				tr("Invalid command format:\n%1").arg(combineCommand(row.pathEdit->text(), row.argsEdit->text())));
			return;
		}
	}

	QList<ToolSetting> newUserTools;
	for (const UserRow& row : m_userRows) {
		const QString path = row.commandEdit->text().trimmed();
		const QString args = row.argsEdit->text().trimmed();
		if (!isRowValid(path, args)) {
			QMessageBox::warning(this, tr("Tool Settings"),
				tr("Invalid command format:\n%1").arg(combineCommand(path, args)));
			return;
		}
		if (path.isEmpty()) {
			continue; // A rule without a command is not meaningful: skip the whole row (including suffixes).
		}

		ToolSetting tool;
		tool.command = combineCommand(path, ensureItemMacro(args));
		for (const QString& part : row.suffixEdit->text().split(QLatin1Char(';'))) {
			QString suffix = part.trimmed();
			if (suffix.startsWith(QLatin1Char('.'))) {
				suffix.remove(0, 1);
			}
			if (!suffix.isEmpty()) {
				tool.suffixes.append(suffix);
			}
		}
		newUserTools.append(tool);
	}

	auto finalizeCommand = [](QLineEdit* pathEdit, QLineEdit* argsEdit) {
		const QString path = pathEdit->text().trimmed();
		const QString args = argsEdit->text().trimmed();
		return combineCommand(path, path.isEmpty() ? args : ensureItemMacro(args));
	};

	m_textTool   = finalizeCommand(mp_textEdit, mp_textArgsEdit);
	m_binaryTool = finalizeCommand(mp_binaryEdit, mp_binaryArgsEdit);
	m_imageTool  = finalizeCommand(mp_imageEdit, mp_imageArgsEdit);
	m_userTools  = newUserTools;

	saveSettings();
	QDialog::accept();
}

