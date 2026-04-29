#pragma once

#include "TeFileInfoAcsr.h"

/**
 * @file TeSystemFileInfoAcsr.h
 * @brief TeFileInfoAcsr implementation for local file-system entries.
 * @ingroup utility
 */

/**
 * @class TeSystemFileInfoAcsr
 * @brief TeFileInfoAcsr concrete class providing context-menu actions and
 *        activation for local file-system items.
 * @ingroup utility
 *
 * @details Supplies the standard "Open", "Open With", and "Properties" actions
 * for files and directories that reside on the local file system.  activate()
 * opens the item using the OS default handler.
 *
 * @see TeFileInfoAcsr, TeArchiveFileInfoAcsr
 */
class TeSystemFileInfoAcsr  : public TeFileInfoAcsr
{
	Q_OBJECT

public:
	TeSystemFileInfoAcsr(QObject *parent);
	~TeSystemFileInfoAcsr();

	/** @brief Returns the local file-system context-menu actions. */
	virtual QList<QPair<QString, ActionFunc>> actions() const;

public slots:
	/**
	 * @brief Opens the file or directory using the OS default application.
	 * @param infos Metadata of the activated file.
	 */
	virtual void activate(const TeFileInfo& infos);
};
