#pragma once

#include <QObject>
#include <QList>
#include <QEvent>

/**
 * @file TeEventEmitter.h
 * @brief Event-forwarding filter that re-emits widget events as Qt signals.
 * @ingroup main
 */

class QWidget;

/**
 * @class TeEventEmitter
 * @brief Event filter that re-emits selected widget events as a Qt signal.
 * @ingroup main
 *
 * @details Install this filter on one or more widgets to receive a unified
 * emitEvent() signal whenever any of the registered event types occur on
 * those widgets.  Used by TeViewStore to detect when a folder view receives
 * focus.
 *
 * Two registration modes are supported:
 * - **Persistent** (addEmitter()): receives every matching event.
 * - **One-shot** (addOneShotEmiter()): receives only the first matching event
 *   and then removes itself automatically.
 */
class TeEventEmitter  : public QObject
{
	Q_OBJECT

public:
	TeEventEmitter(QObject *parent=nullptr);
	~TeEventEmitter();

	/**
	 * @brief Registers an event type to listen for.
	 * @param eventType The Qt event type to monitor.
	 */
	void addEventType(QEvent::Type eventType);

	/**
	 * @brief Registers a widget for one-shot event emission.
	 *
	 * The filter removes itself from @p obj after the first matching event.
	 * @param obj The widget to monitor.
	 */
	void addOneShotEmiter(QWidget* obj);

	/**
	 * @brief Registers a widget for persistent event emission.
	 * @param obj The widget to monitor.
	 */
	void addEmitter(QWidget* obj);

protected:
	/** @brief Qt event filter entry point; emits emitEvent() on match. */
	virtual bool eventFilter(QObject* obj, QEvent* event);

signals:
	/**
	 * @brief Emitted when a monitored event occurs on a registered widget.
	 * @param obj   The widget that produced the event.
	 * @param event The matched event (still owned by Qt).
	 */
	void emitEvent(QWidget* obj, QEvent* event);

private:
	QList<const QWidget*> m_oneShotList;
	QList<const QWidget*> m_objList;
	QList<QEvent::Type>   m_eventTypes;
};
