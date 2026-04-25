#pragma once

#include <QString>
#include <QRegularExpression>
#include <QDateTime>

class TeFindDialog;

/*!
 * \brief Composite search condition built from TeFindDialog selections.
 *
 * All active filters are combined with AND logic: an entry matches only when
 * it satisfies every enabled filter.
 */
struct TeSearchQuery
{
	QString targetPath;
	bool    recursive = true;

	// --- File-name filter ---
	bool               hasNameFilter = false;
	QRegularExpression namePattern;

	// --- File-size filter ---
	bool      hasSizeFilter = false;
	qsizetype sizeThreshold = 0;
	bool      sizeLessThan  = true;   //!< true: size <= threshold, false: size >= threshold

	// --- Last-modified date filter (inclusive range) ---
	bool      hasDateFilter = false;
	QDateTime dateFrom;
	QDateTime dateTo;

	/*! Returns true when targetPath is non-empty (minimum requirement). */
	bool isValid() const;

	/*!
	 * \brief Build a query from the current state of a TeFindDialog.
	 *
	 * Wildcard patterns are automatically converted to a QRegularExpression.
	 * Size values are converted from the selected unit (KB/MB) to bytes.
	 * QDate values are expanded to the start / end of the respective day.
	 */
	static TeSearchQuery fromDialog(const TeFindDialog& dlg, const QString& targetPath);
};
