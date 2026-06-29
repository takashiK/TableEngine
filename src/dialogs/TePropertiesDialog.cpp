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

#include "TePropertiesDialog.h"
#include "TeSettings.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QFileInfo>
#include <QDir>
#include <QLocale>

/**
 * @file TePropertiesDialog.cpp
 * @brief Implementation of TePropertiesDialog.
 * @ingroup dialogs
 */

namespace {

/** @brief Returns the parent directory of a path (handles both real and virtual paths). */
QString locationOf(const QString& path)
{
	const int slash = path.lastIndexOf('/');
	if (slash < 0) {
		return QString();
	}
	return path.left(slash);
}

/** @brief Returns the last path component of a path. */
QString nameOf(const QString& path)
{
	const int slash = path.lastIndexOf('/');
	return (slash < 0) ? path : path.mid(slash + 1);
}

/** @brief Formats a byte count as a localized, human-readable size string. */
QString formatSize(qint64 bytes)
{
	const QString exact = QLocale().toString(bytes) + QObject::tr(" bytes");
	if (bytes < 1024) {
		return exact;
	}
	return QLocale().formattedDataSize(bytes) + " (" + exact + ")";
}

/** @brief Formats a permission bitmask as an rwx-style string, or empty if unset. */
QString formatPermissions(int permissions)
{
	if (permissions == 0) {
		return QString();
	}
	const QFile::Permissions p(static_cast<QFile::Permissions>(permissions));
	QString out;
	out += (p & QFile::ReadOwner) ? 'r' : '-';
	out += (p & QFile::WriteOwner) ? 'w' : '-';
	out += (p & QFile::ExeOwner) ? 'x' : '-';
	out += (p & QFile::ReadGroup) ? 'r' : '-';
	out += (p & QFile::WriteGroup) ? 'w' : '-';
	out += (p & QFile::ExeGroup) ? 'x' : '-';
	out += (p & QFile::ReadOther) ? 'r' : '-';
	out += (p & QFile::WriteOther) ? 'w' : '-';
	out += (p & QFile::ExeOther) ? 'x' : '-';
	return out;
}

} // namespace

TePropertiesDialog::TePropertiesDialog(const QList<TeFileInfo>& items, bool filesystem, QWidget* parent)
	: QDialog(parent)
{
	setWindowTitle(tr("Properties"));
	setMinimumWidth(TeSettings::dialogMinimumWidth());
	buildUi(items, filesystem);
}

TePropertiesDialog::~TePropertiesDialog()
{
}

QList<TeFileInfo> TePropertiesDialog::fromPaths(const QStringList& paths)
{
	QList<TeFileInfo> items;
	for (const QString& path : paths) {
		QFileInfo fi(path);
		TeFileInfo info;
		info.type = fi.isDir() ? TeFileInfo::EN_DIR : TeFileInfo::EN_FILE;
		info.path = QDir::fromNativeSeparators(fi.absoluteFilePath());
		info.size = fi.isDir() ? 0 : fi.size();
		info.lastModified = fi.lastModified();
		info.permissions = static_cast<int>(fi.permissions());
		items.append(info);
	}
	return items;
}

void TePropertiesDialog::buildUi(const QList<TeFileInfo>& items, bool filesystem)
{
	QVBoxLayout* topLayout = new QVBoxLayout(this);
	QFormLayout* form = new QFormLayout();
	topLayout->addLayout(form);

	auto addRow = [form](const QString& label, const QString& value) {
		QLabel* line = new QLabel(value);
		line->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
		form->addRow(label + QStringLiteral(":"), line);
	};

	if (items.size() == 1) {
		const TeFileInfo& info = items.first();
		const bool isDir = (info.type == TeFileInfo::EN_DIR);

		addRow(tr("Name"), nameOf(info.path));
		addRow(tr("Location"), locationOf(info.path));
		addRow(tr("Type"), isDir ? tr("Folder") : tr("File"));
		addRow(tr("Size"), isDir ? QString() : formatSize(info.size));
		addRow(tr("Modified"),
			info.lastModified.isValid() ? QLocale().toString(info.lastModified, QLocale::ShortFormat) : QString());
		addRow(tr("Permissions"), formatPermissions(info.permissions));
	}
	else {
		int files = 0;
		int folders = 0;
		qint64 total = 0;
		QString commonLocation;
		bool commonValid = true;
		for (const TeFileInfo& info : items) {
			if (info.type == TeFileInfo::EN_DIR) {
				++folders;
			}
			else {
				++files;
				total += info.size;
			}
			const QString loc = locationOf(info.path);
			if (commonLocation.isEmpty() && files + folders == 1) {
				commonLocation = loc;
			}
			else if (loc != commonLocation) {
				commonValid = false;
			}
		}

		addRow(tr("Items"), tr("%1 (files: %2, folders: %3)").arg(items.size()).arg(files).arg(folders));
		addRow(tr("Location"), commonValid ? commonLocation : QString());
		addRow(tr("Total size"), formatSize(total));
	}

	if (!filesystem) {
		QLabel* note = new QLabel(tr("Some attributes are unavailable for archive entries."));
		note->setWordWrap(true);
		topLayout->addWidget(note);
	}

	QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
	connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
	connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
	topLayout->addWidget(buttons);
}
