#pragma once

class TeViewStore;
class QAbstractItemModel;
class QModelIndex;

#include <QStringList>

enum TeFileType {
	TE_FILE_UNKNOWN,
	TE_FILE_FOLDER,
	TE_FILE_TEXT,
	TE_FILE_IMAGE,
	TE_FILE_ARCHIVE,
};

bool getSelectedItemList(TeViewStore* p_store, QStringList* p_paths);
QString getCurrentItem(TeViewStore* p_store);
QString getCurrentFolder(TeViewStore* p_store);


QStringList getFavorites();
void updateFavorites(const QStringList& list);

bool isDir(const QAbstractItemModel* p_model, const QModelIndex& index);

TeFileType getFileType(const QString& path);

QString detectTextCodec(const QByteArray& data, const QStringList& codecList);

#define NOT_USED(x) (void)x
