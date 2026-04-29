#pragma once

#include <QObject>
#include <QList>
#include <QAction>
#include <QStandardItem>

#include "TeFileInfo.h"
#include "TeDispatchable.h"

/**
 * @file TeFileInfoAcsr.h
 * @brief Abstract base class for context-menu action providers on file items.
 * @ingroup utility
 *
 * @see doc/markdown/utils/TeArchiveFileInfoAcsr.md
 */

/**
 * @class TeFileInfoAcsr
 * @brief Abstract interface that attaches a set of user actions to a
 *        QStandardItem representing a file.
 * @ingroup utility
 *
 * @details Concrete subclasses (TeSystemFileInfoAcsr, TeArchiveFileInfoAcsr)
 * supply a list of labelled action callbacks via actions() and handle item
 * activation via activate().  An instance is attached to a QStandardItem
 * using assignToStandardItem() and retrieved via fromStandardItem().
 *
 * @see TeSystemFileInfoAcsr, TeArchiveFileInfoAcsr
 * @see doc/markdown/utils/TeArchiveFileInfoAcsr.md
 */
class TeFileInfoAcsr  : public QObject
{
	Q_OBJECT
public:
	/** @brief Function pointer type for context-menu actions. */
	typedef void(*ActionFunc)(TeDispatchable* dispatcher, TeTypes::WidgetType type, const QList<TeFileInfo>& infos);

public:
	TeFileInfoAcsr(QObject *parent);
	virtual ~TeFileInfoAcsr();

	/**
	 * @brief Returns the list of available context-menu actions for this item.
	 *
	 * Each entry pairs a human-readable action name with a function pointer
	 * that will be called when the user selects that action.
	 * @return List of (name, callback) pairs.
	 */
	virtual QList<QPair<QString, ActionFunc>> actions() const = 0;

	/**
	 * @brief Attaches this accessor to @p item as custom item data.
	 *
	 * The item takes ownership of this object.  After this call, the
	 * accessor can be retrieved via fromStandardItem().
	 * @param item The QStandardItem to bind to.
	 */
	void assignToStandardItem( QStandardItem* item);

	/**
	 * @brief Retrieves the TeFileInfoAcsr attached to @p item.
	 * @param item The item to query.
	 * @return The attached accessor, or nullptr if none is attached.
	 */
	static TeFileInfoAcsr* fromStandardItem(const QStandardItem* item);

public slots:
	/**
	 * @brief Invoked when the user double-clicks or opens an item.
	 * @param info Metadata of the activated file.
	 */
	virtual void activate(const TeFileInfo& info) = 0;
};
