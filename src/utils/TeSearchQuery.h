#pragma once

#include <QString>
#include <QRegularExpression>
#include <QDateTime>

/**
 * @file TeSearchQuery.h
 * @brief Composite search condition used by TeFinder subclasses.
 * @ingroup utility
 *
 * @see doc/markdown/utils/TeSearchQuery.md
 */

class TeFindDialog;

/**
 * @struct TeSearchQuery
 * @brief Composite search condition built from TeFindDialog selections.
 * @ingroup utility
 *
 * @details All active filters are combined with AND logic: an entry matches
 * only when it satisfies every enabled filter.  Filters are individually
 * gated by @c hasNameFilter, @c hasSizeFilter, and @c hasDateFilter.
 *
 * @see TeFinder, TeFileFinder, TeArchiveFinder, TeFindDialog
 * @see doc/markdown/utils/TeSearchQuery.md
 */
struct TeSearchQuery
{
	QString targetPath;         ///< Root path (directory or archive) to search.
	bool    recursive = true;   ///< Whether to search subdirectories recursively.

	// --- File-name filter ---
	bool               hasNameFilter = false; ///< Enable name-pattern filtering.
	QRegularExpression namePattern;           ///< Regex to match against filenames.

	// --- File-size filter ---
	bool      hasSizeFilter = false; ///< Enable file-size filtering.
	qsizetype sizeThreshold = 0;    ///< Threshold size in bytes.
	bool      sizeLessThan  = true; ///< true: size <= threshold; false: size >= threshold.

	// --- Last-modified date filter (inclusive range) ---
	bool      hasDateFilter = false; ///< Enable date-range filtering.
	QDateTime dateFrom;              ///< Start of the inclusive date range.
	QDateTime dateTo;                ///< End of the inclusive date range.

	/** @brief Returns true when targetPath is non-empty (minimum requirement). */
	bool isValid() const;

	/**
	 * @brief Builds a query from the current state of a TeFindDialog.
	 *
	 * Wildcard patterns are automatically converted to a QRegularExpression.
	 * Size values are converted from the selected unit (KB/MB) to bytes.
	 * QDate values are expanded to the start / end of the respective day.
	 * @param dlg        The source dialog to read values from.
	 * @param targetPath The root path to search.
	 * @return A fully populated TeSearchQuery.
	 */
	static TeSearchQuery fromDialog(const TeFindDialog& dlg, const QString& targetPath);
};
