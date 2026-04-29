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
 * @brief Returns the path of the focused (current) item in the active list view.
 * @param p_store The application-wide TeViewStore.
 * @return Absolute path string, or an empty string if no item is current.
 */
QString getCurrentItem(TeViewStore* p_store);

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

/** @brief No-op macro to suppress unused-variable warnings. */
#define NOT_USED(x) (void)x
