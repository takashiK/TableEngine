#pragma once

#include "TeFileInfoAcsr.h"

/**
 * @file TeArchiveFileInfoAcsr.h
 * @brief TeFileInfoAcsr specialisation for archive-entry items.
 * @ingroup utility
 *
 * @see doc/markdown/utils/TeArchiveFileInfoAcsr.md
 */

/**
 * @class TeArchiveFileInfoAcsr
 * @brief Intermediate abstract class for archive-entry context-menu actions.
 * @ingroup utility
 *
 * @details Extends TeFileInfoAcsr with a default activate() implementation
 * that opens the archive entry in TeArchiveFolderView.  Concrete subclasses
 * must still implement actions() to supply the specific context-menu entries.
 *
 * @see TeFileInfoAcsr, TeSystemFileInfoAcsr
 * @see doc/markdown/utils/TeArchiveFileInfoAcsr.md
 */
class TeArchiveFileInfoAcsr  : public TeFileInfoAcsr
{
	Q_OBJECT

public:
	TeArchiveFileInfoAcsr(QObject *parent);
	~TeArchiveFileInfoAcsr();

	/** @brief Returns context-menu actions specific to the archive-entry type. */
	virtual QList<QPair<QString, ActionFunc>> actions() const = 0;

public slots:
	/**
	 * @brief Opens the archive that contains the activated entry.
	 * @param info Metadata of the activated archive entry.
	 */
	virtual void activate(const TeFileInfo& info);
};
