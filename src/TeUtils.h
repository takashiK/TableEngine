#pragma once

class TeViewStore;

#include <QStringList>

enum TeFileType {
	TE_FILE_UNKNOWN,
	TE_FILE_FOLDER,
	TE_FILE_TEXT,
	TE_FILE_IMAGE,
	TE_FILE_ARCHIVE,
};

bool getSelectedItemList(TeViewStore* p_store, QStringList* p_paths);

TeFileType getFileType(const QString& path);
