#pragma once

#include "TeFinder.h"

/**
 * @file TeFileFinder.h
 * @brief TeFinder implementation for local file-system searches.
 * @ingroup utility
 *
 * @see doc/markdown/utils/TeFinder.md
 */

class QFileInfo;

/**
 * @class TeFileFinder
 * @brief Asynchronous finder that searches a local directory tree.
 * @ingroup utility
 *
 * @details Walks the directory tree rooted at the path supplied to the
 * constructor using recursive QDirIterator.  Each entry is tested against
 * the TeSearchQuery and, on a match, appended to the result list via
 * TeFinder::reportItems().
 *
 * @see TeFinder, TeArchiveFinder, TeSearchQuery
 * @see doc/markdown/utils/TeFinder.md
 */
class TeFileFinder : public TeFinder
{
	Q_OBJECT

public:
	/**
	 * @brief Constructs a finder rooted at @p path.
	 * @param path   Absolute path to the directory to search.
	 * @param parent Parent QObject.
	 */
	explicit TeFileFinder(const QString& path, QObject* parent = nullptr);
	~TeFileFinder();

	/** @brief Returns true when the root path exists and is a directory. */
	bool isValid() const override;

protected:
	/**
	 * @brief Recursively searches the directory tree.
	 * @param query Search parameters including name pattern and file-type filter.
	 */
	void doSearch(const TeSearchQuery& query) override;

private:
	/**
	 * @brief Returns true when @p info matches the search query.
	 * @param info  File-system entry to test.
	 * @param query The search parameters.
	 */
	bool matchesQuery(const QFileInfo& info, const TeSearchQuery& query) const;

	QString m_rootPath; ///< Absolute root path for the search.
};
