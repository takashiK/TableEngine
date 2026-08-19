#include "TeEditUserCommands.h"
#include "TeSettings.h"
#include "utils/TeToolCommand.h"
#include "utils/TeAdaptiveIconEngine.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QToolButton>
#include <QDialogButtonBox>
#include <QScrollArea>
#include <QFrame>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QSettings>
#include <QSet>
#include <limits>

/**
 * @file TeEditUserCommands.cpp
 * @brief Implementation of TeEditUserCommands.
 * @ingroup dialogs
 */

TeEditUserCommands::TeEditUserCommands(QWidget *parent)
	: QDialog(parent)
{
	setWindowTitle(tr("User Commands"));
	setMinimumWidth(qMax(500, TeSettings::dialogMinimumWidth()));
	setMinimumHeight(300);

	loadSettings();

	QVBoxLayout* mainLayout = new QVBoxLayout(this);

	QWidget* container = new QWidget();
	mp_userLayout = new QVBoxLayout(container);
	mp_userLayout->addStretch(1);

	QScrollArea* scrollArea = new QScrollArea();
	scrollArea->setWidgetResizable(true);
	scrollArea->setWidget(container);
	mainLayout->addWidget(scrollArea, 1);

	for (const UserCommandSetting& entry : m_userCommands) {
		QWidget* frame = createUserEntry(entry);
		mp_userLayout->insertWidget(mp_userLayout->count() - 1, frame);
	}

	QPushButton* addButton = new QPushButton(tr("Add"));
	connect(addButton, &QPushButton::clicked, this, [this]() { addUserEntry(); });
	mainLayout->addWidget(addButton);

	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	buttonBox->setCenterButtons(true);
	connect(buttonBox, &QDialogButtonBox::accepted, this, &TeEditUserCommands::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
	mainLayout->addWidget(buttonBox);

	setLayout(mainLayout);
}

TeEditUserCommands::~TeEditUserCommands()
{}

QWidget* TeEditUserCommands::createUserEntry(const UserCommandSetting& initial)
{
	QFrame* frame = new QFrame();
	frame->setFrameShape(QFrame::StyledPanel);

	QHBoxLayout* mainLayout = new QHBoxLayout(frame);

    QVBoxLayout* leftLayout = new QVBoxLayout();

    QLabel *idLabel = new QLabel(tr("ID:")+QString::number(initial.id));
    idLabel->setMinimumWidth(40);
    leftLayout->addWidget(idLabel);

    leftLayout->addStretch(2);
	QLabel* iconPreviewLabel = new QLabel();
	iconPreviewLabel->setFixedSize(32, 32);
	iconPreviewLabel->setAlignment(Qt::AlignCenter);
	leftLayout->addWidget(iconPreviewLabel);
    leftLayout->addStretch(3);

	mainLayout->addLayout(leftLayout);

	QGridLayout* frameLayout = new QGridLayout();
	mainLayout->addLayout(frameLayout);

	QString initialPath, initialArgs;
	splitCommand(initial.command, initialPath, initialArgs);

    frameLayout->addWidget(new QLabel(tr("Name:")), 0, 0);
	QHBoxLayout* row1 = new QHBoxLayout();
	QLineEdit* nameEdit = new QLineEdit(initial.name);
	row1->addWidget(nameEdit, 1);
	frameLayout->addLayout(row1, 0, 1);

    frameLayout->addWidget(new QLabel(tr("Icon:")), 1, 0);
	QHBoxLayout* row2 = new QHBoxLayout();
	QLineEdit* iconEdit = new QLineEdit(initial.icon);
	row2->addWidget(iconEdit, 1);

	QToolButton* iconBrowseButton = new QToolButton();
	iconBrowseButton->setText("...");
	row2->addWidget(iconBrowseButton);
	connect(iconBrowseButton, &QToolButton::clicked, this, [this, iconEdit]() {
		const QString currentPath = iconEdit->text().trimmed();
		const QString startDir = currentPath.isEmpty() ? QString() : QFileInfo(currentPath).absolutePath();
		const QString path = QFileDialog::getOpenFileName(this, tr("Select Icon"), startDir, tr("PNG Files (*.png)"));
		if (path.isEmpty()) {
			return; // Cancel: leave the field untouched.
		}
		iconEdit->setText(path);
	});
	frameLayout->addLayout(row2, 1, 1);

	auto updateIconPreview = [iconPreviewLabel](const QString& path) {
		const QString effectivePath = path.trimmed().isEmpty() ? QStringLiteral(":/TableEngine/userCommand.png") : path.trimmed();
		const QIcon icon(new TeAdaptiveIconEngine(effectivePath));
		iconPreviewLabel->setPixmap(icon.pixmap(iconPreviewLabel->size()));
	};
	updateIconPreview(initial.icon);
	connect(iconEdit, &QLineEdit::textChanged, iconPreviewLabel, updateIconPreview);

    frameLayout->addWidget(new QLabel(tr("Command:")), 2, 0);
	QHBoxLayout* row3 = new QHBoxLayout();
	QLineEdit* commandEdit = new QLineEdit(initialPath);
	row3->addWidget(commandEdit, 1);

	QToolButton* browseButton = new QToolButton();
	browseButton->setText("...");
	row3->addWidget(browseButton);
	connect(browseButton, &QToolButton::clicked, this, [this, commandEdit]() {
		const QString currentPath = commandEdit->text().trimmed();
		const QString startDir = currentPath.isEmpty() ? QString() : QFileInfo(currentPath).absolutePath();
		const QString path = QFileDialog::getOpenFileName(this, tr("Select Command"), startDir);
		if (path.isEmpty()) {
			return; // Cancel: leave the field untouched.
		}
		commandEdit->setText(path);
	});
	frameLayout->addLayout(row3, 2, 1);

    frameLayout->addWidget(new QLabel(tr("Args:")), 3, 0);
    QHBoxLayout* row4 = new QHBoxLayout();
	QLineEdit* argsEdit = new QLineEdit(initialArgs);
	row4->addWidget(argsEdit, 1);
	frameLayout->addLayout(row4, 3, 1);

	QHBoxLayout* row5 = new QHBoxLayout();
	QCheckBox* shellCheck = new QCheckBox(tr("Run via shell"));
	shellCheck->setChecked(initial.shell);
	row5->addWidget(shellCheck);
	QCheckBox* outputCheck = new QCheckBox(tr("Show output"));
	outputCheck->setChecked(initial.output);
	row5->addWidget(outputCheck);
	row5->addStretch(1);
	frameLayout->addLayout(row5, 4, 0, 1, 2);

	QToolButton* removeButton = new QToolButton();
	removeButton->setIcon(QIcon(":/TableEngine/delete.png"));
	mainLayout->addWidget(removeButton);

	auto updateValidity = [this, commandEdit, argsEdit]() {
		applyValidityStyle(commandEdit, isRowValid(commandEdit->text(), argsEdit->text()));
	};
	connect(commandEdit, &QLineEdit::textChanged, this, updateValidity);
	connect(argsEdit, &QLineEdit::textChanged, this, updateValidity);
	connect(removeButton, &QToolButton::clicked, this, [this, frame]() { removeUserEntry(frame); });

	UserCommandRow row;
	row.frame       = frame;
	row.nameEdit    = nameEdit;
	row.iconEdit    = iconEdit;
	row.commandEdit = commandEdit;
	row.argsEdit    = argsEdit;
	row.shellCheck  = shellCheck;
	row.outputCheck = outputCheck;
	row.id          = initial.id;
	m_userRows.append(row);

	return frame;
}

void TeEditUserCommands::addUserEntry()
{
	addUserEntry(UserCommandSetting{});
}

void TeEditUserCommands::addUserEntry(const UserCommandSetting& initial)
{
	if (m_userRows.size() >= TeSettings::MAX_USER_COMMANDS) {
		QMessageBox::information(this, tr("User Commands"),
			tr("Maximum number of user commands (%1) reached.").arg(TeSettings::MAX_USER_COMMANDS));
		return;
	}

	UserCommandSetting entry = initial;
	if (entry.id == 0) {
		entry.id = allocateNewId();
		m_usedIds.insert(entry.id); // Never released for the rest of this dialog instance's lifetime.
	}

	QWidget* frame = createUserEntry(entry);
	mp_userLayout->insertWidget(mp_userLayout->count() - 1, frame);
}

void TeEditUserCommands::removeUserEntry(QWidget* frame)
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

void TeEditUserCommands::applyValidityStyle(QLineEdit* edit, bool valid) const
{
	edit->setStyleSheet(valid ? QString() : QStringLiteral("QLineEdit { border: 1px solid red; }"));
}

bool TeEditUserCommands::isRowValid(const QString& path, const QString& args) const
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

quint16 TeEditUserCommands::allocateNewId()
{
	// Candidates are drawn from m_usedIds, which is seeded from QSettings at
	// load time and only ever grows for the lifetime of this dialog instance
	// (see the class-level doc comment on m_usedIds). Consequently, removing a
	// row does not free its id for reuse within this session: deleting a
	// command and registering a new one will not reissue the deleted id, so
	// any stale key/menu binding still referencing that id cannot be silently
	// re-attached to an unrelated new command. This is a best-effort, single
	// session mitigation; ids freed by a previous dialog session (i.e. after
	// save+close) remain eligible for reuse the next time the dialog opens.
	for (quint32 candidate = 1; candidate <= std::numeric_limits<quint16>::max(); ++candidate) {
		if (!m_usedIds.contains(static_cast<quint16>(candidate))) {
			return static_cast<quint16>(candidate);
		}
	}
	return 0; // Unreachable in practice: MAX_USER_COMMANDS is far smaller than 65535.
}

void TeEditUserCommands::splitCommand(const QString& full, QString& path, QString& args)
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

QString TeEditUserCommands::combineCommand(const QString& path, const QString& args)
{
	const QString trimmedPath = path.trimmed();
	const QString trimmedArgs = args.trimmed();
	if (trimmedPath.isEmpty()) {
		return trimmedArgs; // Edge case (invalid): arguments without a command path.
	}
	const QString quotedPath = trimmedPath.contains(QLatin1Char(' ')) ? QString("\"%1\"").arg(trimmedPath) : trimmedPath;
	return trimmedArgs.isEmpty() ? quotedPath : quotedPath + QLatin1Char(' ') + trimmedArgs;
}

QString TeEditUserCommands::ensureItemMacro(const QString& args)
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

void TeEditUserCommands::loadSettings()
{
	m_userCommands.clear();
	m_usedIds.clear();
	QSettings settings;
	settings.beginGroup(SETTING_USER);
	for (int i = 0; i < TeSettings::MAX_USER_COMMANDS; ++i) {
		const QString key = QString("command%1").arg(i, 2, 10, QLatin1Char('0'));
		if (!settings.contains(key)) {
			continue;
		}
		const QString raw = settings.value(key).toString();

		UserCommandSetting entry;
		entry.id      = static_cast<quint16>(raw.section(QLatin1Char(';'), 0, 0).toUInt());
		entry.shell   = raw.section(QLatin1Char(';'), 1, 1).toInt() != 0;
		entry.output  = raw.section(QLatin1Char(';'), 2, 2).toInt() != 0;
		entry.name    = raw.section(QLatin1Char(';'), 3, 3);
		entry.icon    = raw.section(QLatin1Char(';'), 4, 4);
		entry.command = raw.section(QLatin1Char(';'), 5);
		m_userCommands.append(entry);
		m_usedIds.insert(entry.id); // Seed: ids already on disk are never re-handed-out this session.
	}
	settings.endGroup();
}

void TeEditUserCommands::saveSettings()
{
	QSettings settings;
	settings.beginGroup(SETTING_USER);
	settings.remove(""); // Clear all existing command entries before rewriting.
	int idx = 0;
	for (const UserCommandSetting& entry : m_userCommands) {
		if (idx >= TeSettings::MAX_USER_COMMANDS) {
			break;
		}
		const QString value = QStringLiteral("%1;%2;%3;%4;%5;%6")
			.arg(entry.id)
			.arg(entry.shell ? 1 : 0)
			.arg(entry.output ? 1 : 0)
			.arg(entry.name)
			.arg(entry.icon)
			.arg(entry.command);
		settings.setValue(QString("command%1").arg(idx, 2, 10, QLatin1Char('0')), value);
		++idx;
	}
	settings.endGroup();
}

void TeEditUserCommands::accept()
{
	QList<UserCommandSetting> newUserCommands;
	for (const UserCommandRow& row : m_userRows) {
		const QString path = row.commandEdit->text().trimmed();
		const QString args = row.argsEdit->text().trimmed();
		const QString name = row.nameEdit->text().trimmed();

		if (path.isEmpty() && args.isEmpty() && name.isEmpty()) {
			continue; // Entirely empty row: not meaningful, skip.
		}

		if (!isRowValid(path, args)) {
			QMessageBox::warning(this, tr("User Commands"),
				tr("Invalid command format:\n%1").arg(combineCommand(path, args)));
			return;
		}
		if (path.isEmpty()) {
			continue; // A rule without a command is not meaningful: skip the whole row.
		}
		if (name.isEmpty()) {
			QMessageBox::warning(this, tr("User Commands"),
				tr("Name is required for command:\n%1").arg(combineCommand(path, args)));
			return;
		}

		UserCommandSetting entry;
		entry.id      = row.id;
		entry.name    = name;
		entry.icon    = row.iconEdit->text().trimmed();
		//entry.command = combineCommand(path, ensureItemMacro(args));
        entry.command = combineCommand(path, args);
		entry.shell   = row.shellCheck->isChecked();
		entry.output  = row.outputCheck->isChecked();
		newUserCommands.append(entry);
	}

	m_userCommands = newUserCommands;
	saveSettings();
	QDialog::accept();
}
