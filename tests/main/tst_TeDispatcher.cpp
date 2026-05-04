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
#include <QKeyEvent>
#include <QSettings>
#include "TeDispatcher.h"
#include "commands/TeCommandFactory.h"
#include "TeTypes.h"
#include "test_mock/mock_TeViewStore.h"

using namespace ::testing;

class tst_TeDispatcher : public ::testing::Test {
protected:
    void SetUp() override {
        dispatcher.setFactory(TeCommandFactory::factory());
        dispatcher.setViewStore(&store);
    }

    TeDispatcher dispatcher;
    NiceMock<mock_TeViewStore> store;
};

// ── getType ───────────────────────────────────────────────────────────────────

TEST_F(tst_TeDispatcher, getType_returns_WT_NONE)
{
    EXPECT_EQ(dispatcher.getType(), TeTypes::WT_NONE);
}

// ── dispatch with no key bindings ─────────────────────────────────────────────

TEST_F(tst_TeDispatcher, dispatch_non_keypress_event_returns_true)
{
    // dispatch() should return true regardless of event type.
    QEvent mouseEvent(QEvent::MouseButtonPress);
    bool result = dispatcher.dispatch(TeTypes::WT_LISTVIEW, &mouseEvent);
    EXPECT_TRUE(result);
}

TEST_F(tst_TeDispatcher, dispatch_keypress_with_no_binding_returns_true)
{
    QKeyEvent keyEv(QEvent::KeyPress, Qt::Key_F12, Qt::NoModifier);
    bool result = dispatcher.dispatch(TeTypes::WT_LISTVIEW, &keyEv);
    EXPECT_TRUE(result);
}

// ── requestCommandFinalize ────────────────────────────────────────────────────

TEST_F(tst_TeDispatcher, requestCommandFinalize_emits_commandFinalize)
{
    QSignalSpy spy(&dispatcher, &TeDispatcher::commandFinalize);
    // Pass nullptr — the signal just forwards the pointer.
    dispatcher.requestCommandFinalize(nullptr);
    EXPECT_EQ(spy.count(), 1);
}

// ── commandFinalize signal carries pointer ────────────────────────────────────

TEST_F(tst_TeDispatcher, commandFinalize_signal_fires_once_with_null)
{
    // TeCommandBase* is not a registered meta-type; just verify the signal fires.
    QSignalSpy spy(&dispatcher, &TeDispatcher::commandFinalize);
    dispatcher.requestCommandFinalize(nullptr);
    EXPECT_EQ(spy.count(), 1);
}

// ── loadKeySetting does not crash ────────────────────────────────────────────

TEST_F(tst_TeDispatcher, loadKeySetting_does_not_crash)
{
    EXPECT_NO_THROW(dispatcher.loadKeySetting());
}
