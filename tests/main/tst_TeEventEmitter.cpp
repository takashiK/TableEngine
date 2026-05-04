/****************************************************************************
**
** Copyright (C) 2021 Takashi Kuwabara.
** Contact: laffile@gmail.com
**
**  This program is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 2 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program.  If not, see <https://www.gnu.org/licenses/>.
**
****************************************************************************/

#include <gmock/gmock.h>
#include <QTest>
#include <QSignalSpy>
#include <QWidget>
#include <QEvent>
#include <QFocusEvent>
#include "TeEventEmitter.h"

using namespace ::testing;

class tst_TeEventEmitter : public ::testing::Test {
protected:
    TeEventEmitter emitter;
};

// ── addEventType / addEmitter / emitEvent signal ──────────────────────────────

TEST_F(tst_TeEventEmitter, persistent_emitter_fires_on_registered_event)
{
    emitter.addEventType(QEvent::FocusIn);

    QWidget widget;
    emitter.addEmitter(&widget);

    QSignalSpy spy(&emitter, &TeEventEmitter::emitEvent);

    QFocusEvent focusIn(QEvent::FocusIn);
    QCoreApplication::sendEvent(&widget, &focusIn);

    EXPECT_EQ(spy.count(), 1);
}

TEST_F(tst_TeEventEmitter, persistent_emitter_fires_multiple_times)
{
    emitter.addEventType(QEvent::FocusIn);

    QWidget widget;
    emitter.addEmitter(&widget);

    QSignalSpy spy(&emitter, &TeEventEmitter::emitEvent);

    for (int i = 0; i < 3; ++i) {
        QFocusEvent e(QEvent::FocusIn);
        QCoreApplication::sendEvent(&widget, &e);
    }
    EXPECT_EQ(spy.count(), 3);
}

TEST_F(tst_TeEventEmitter, unregistered_event_type_does_not_fire)
{
    emitter.addEventType(QEvent::FocusIn);  // only FocusIn registered

    QWidget widget;
    emitter.addEmitter(&widget);

    QSignalSpy spy(&emitter, &TeEventEmitter::emitEvent);

    QEvent hideEvent(QEvent::Hide);
    QCoreApplication::sendEvent(&widget, &hideEvent);

    EXPECT_EQ(spy.count(), 0);
}

// ── addOneShotEmiter ──────────────────────────────────────────────────────────

TEST_F(tst_TeEventEmitter, one_shot_fires_only_once)
{
    emitter.addEventType(QEvent::FocusIn);

    QWidget widget;
    emitter.addOneShotEmiter(&widget);

    QSignalSpy spy(&emitter, &TeEventEmitter::emitEvent);

    // First event — should fire
    QFocusEvent e1(QEvent::FocusIn);
    QCoreApplication::sendEvent(&widget, &e1);
    EXPECT_EQ(spy.count(), 1);

    // Second event — one-shot was removed; should NOT fire again
    QFocusEvent e2(QEvent::FocusIn);
    QCoreApplication::sendEvent(&widget, &e2);
    EXPECT_EQ(spy.count(), 1);
}

// ── signal payload ────────────────────────────────────────────────────────────

TEST_F(tst_TeEventEmitter, signal_carries_source_widget)
{
    emitter.addEventType(QEvent::FocusIn);

    QWidget widget;
    emitter.addEmitter(&widget);

    QWidget* reportedWidget = nullptr;
    int signalCount = 0;
    QObject::connect(&emitter, &TeEventEmitter::emitEvent,
                     [&](QWidget* obj, QEvent*) {
                         reportedWidget = obj;
                         ++signalCount;
                     });

    QFocusEvent e(QEvent::FocusIn);
    QCoreApplication::sendEvent(&widget, &e);

    EXPECT_EQ(signalCount, 1);
    EXPECT_EQ(reportedWidget, &widget);
}
