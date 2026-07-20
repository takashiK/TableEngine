#pragma once

/**
 * @file TeUtils.h
 * @brief Free utility functions and constants used throughout TableEngine.
 * @ingroup utility
 */

class TeViewStore;
class QAbstractItemModel;
class QModelIndex;

#include <QStringList>

/**
 * @brief Internal classification of file types by content kind.
 * @ingroup utility
 */
enum TeFileType {
	TE_FILE_UNKNOWN, ///< Type could not be determined.
	TE_FILE_FOLDER,  ///< Item is a directory.
	TE_FILE_TEXT,    ///< Item is a plain-text file.
	TE_FILE_IMAGE,   ///< Item is a supported image file.
	TE_FILE_ARCHIVE, ///< Item is a supported archive file.
};

/**
 * @brief Collects the paths of all currently selected items in the active list view.
 * @param p_store  The application-wide TeViewStore.
 * @param p_paths  Output list populated with absolute item paths.
 * @return true if at least one path was collected.
 */
bool getSelectedItemList(TeViewStore* p_store, QStringList* p_paths);

/**
 * @brief Same as getSelectedItemList(), but excludes directories.
 * @param p_store  The application-wide TeViewStore.
 * @param p_paths  Output list populated with absolute file paths (directories skipped).
 * @return true if at least one file path was collected.
 */
bool getSelectedFileList(TeViewStore* p_store, QStringList* p_paths);

/**
 * @brief Returns the path of the focused (current) item in the active list view.
 * @param p_store The application-wide TeViewStore.
 * @return Absolute path string, or an empty string if no item is current.
 */
QString getCurrentItem(TeViewStore* p_store);

/**
 * @brief Extracts the given archive files to a target directory.
 * @param p_store The application-wide TeViewStore.
 * @param list List of archive file paths to extract.
 * @param targetPath Destination directory for extracted files.
 * @param createArchiveFolder If true, creates a subfolder named after each archive.
 */
void extractArchives(TeViewStore* p_store, const QStringList & list, const QString & targetPath, bool createArchiveFolder);

/**
 * @brief Extracts the given archive entries to the active view's temp directory.
 *
 * Only valid when the active folder view is a TeArchiveFolderView with an open reader.
 * Each entry is extracted to targetPath.
 * @param p_store The application-wide TeViewStore.
 * @param basePath The base path to use for the extracted entries.
 * @param entries Virtual archive entry paths to extract.
 * @param targetPath Destination directory for extracted files.
 * @return Absolute paths of the extracted top-level items, or empty on failure.
 */
QStringList extractArchiveSelectionToPath(TeViewStore* p_store, const QString& basePath, const QStringList& entries, const QString& targetPath);

/**
 * @brief Extracts the given archive entries to the active view's temp directory.
 *
 * Only valid when the active folder view is a TeArchiveFolderView with an open
 * reader.  Each entry is extracted preserving its virtual path under the
 * temporary directory.
 * @param p_store The application-wide TeViewStore.
 * @param entries Virtual archive entry paths to extract.
 * @return Absolute paths of the extracted top-level items, or empty on failure.
 */
QStringList extractArchiveSelectionToTempPath(TeViewStore* p_store, const QStringList& entries);

/**
 * @brief Returns the directory path currently shown in the active list view.
 * @param p_store The application-wide TeViewStore.
 * @return Absolute directory path string.
 */
QString getCurrentFolder(TeViewStore* p_store);

/** @brief Returns the persisted list of favourite paths from QSettings. */
QStringList getFavorites();

/**
 * @brief Overwrites the persisted favourite-paths list.
 * @param list New list of absolute directory paths.
 */
void updateFavorites(const QStringList& list);

/**
 * @brief Returns true when the model index refers to a directory.
 * @param index A model index from QFileSystemModel or a compatible model.
 */
bool isDir(const QModelIndex& index);

/**
 * @brief Classifies a file by its extension.
 * @param path Absolute or relative file path.
 * @return A TeFileType value.
 */
TeFileType getFileType(const QString& path);

/**
 * @brief Detects the text encoding of a byte buffer.
 * @param data      Raw byte data from the file.
 * @param codecList Ordered list of codec names to probe.
 * @return The name of the first matching codec, or an empty string.
 */
QString detectTextCodec(const QByteArray& data, const QStringList& codecList);

/**
 * @brief Classifies a file from at most its first 1024 bytes.
 *
 * The detector recognizes UTF-8, UTF-16 (little/big endian), Shift-JIS,
 * and EUC-JP. This is a heuristic content check; it does not inspect the
 * file name or MIME type.
 * @param path File path to inspect.
 * @return true when the sample is likely text, otherwise false.
 */
bool isTextFile(const QString& path);

/** @brief No-op macro to suppress unused-variable warnings. */
#define NOT_USED(x) (void)x
