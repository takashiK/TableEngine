#pragma once

#include "TeFinder.h"

/**
 * @file TeArchiveFinder.h
 * @brief TeFinder implementation for searches inside archive files.
 * @ingroup utility
 *
 * @see doc/markdown/utils/TeFinder.md
 */

namespace TeArchive
{
	class Reader;
}

/**
 * @class TeArchiveFinder
 * @brief Asynchronous finder that searches entries within an archive file.
 * @ingroup utility
 *
 * @details Opens the archive at the path supplied to the constructor using
 * TeArchive::Reader and iterates over all entries.  Each entry is tested
 * against the TeSearchQuery and matching entries are published via
 * TeFinder::reportItems().
 *
 * @see TeFinder, TeFileFinder, TeArchive::Reader
 * @see doc/markdown/utils/TeFinder.md
 */
class TeArchiveFinder : public TeFinder
{
	Q_OBJECT

public:
	/**
	 * @brief Constructs a finder for the archive at @p path.
	 * @param path   Absolute path to the archive file.
	 * @param parent Parent QObject.
	 */
	explicit TeArchiveFinder(const QString& path, QObject* parent = nullptr);
	~TeArchiveFinder();

	/** @brief Returns true when the archive file was opened successfully. */
	bool isValid() const override;

protected:
	/**
	 * @brief Iterates over the archive and matches entries against @p query.
	 * @param query Search parameters.
	 */
	void doSearch(const TeSearchQuery& query) override;

private:
	/**
	 * @brief Returns true when @p entry matches the search query.
	 * @param entry Archive entry metadata.
	 * @param query The search parameters.
	 */
	bool matchesEntry(const TeFileInfo& entry, const TeSearchQuery& query) const;

	TeArchive::Reader* mp_reader = nullptr; ///< Opened archive reader (owned).
};
