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
#include <QKeyEvent>
#include "commands/TeCommandBase.h"
#include "commands/TeCommandFactory.h"
#include "TeTypes.h"
#include "test_mock/mock_TeDispatcher.h"
#include "test_mock/mock_TeViewStore.h"

using namespace ::testing;

// ── Minimal concrete command for testing ──────────────────────────────────────
class SyncCommand : public TeCommandBase {
public:
    bool executed = false;
protected:
    bool execute(TeViewStore* /*p_store*/) override {
        executed = true;
        return true; // synchronous
    }
};

class AsyncCommand : public TeCommandBase {
public:
    bool executed = false;
protected:
    bool execute(TeViewStore* /*p_store*/) override {
        executed = true;
        return false; // async — caller must call finished() later
    }
public:
    void completeAsync() { finished(); }
};

// ── Fixture ───────────────────────────────────────────────────────────────────

class tst_TeCommandBase : public ::testing::Test {
protected:
    NiceMock<mock_TeDispatcher>  dispatcher;
    NiceMock<mock_TeViewStore>   store;
};

// ── setDispatcher / run (sync) ────────────────────────────────────────────────

TEST_F(tst_TeCommandBase, sync_run_calls_execute)
{
    SyncCommand cmd;
    cmd.setDispatcher(&dispatcher);
    cmd.run(&store);
    EXPECT_TRUE(cmd.executed);
}

TEST_F(tst_TeCommandBase, sync_run_calls_requestCommandFinalize)
{
    EXPECT_CALL(dispatcher, requestCommandFinalize(_)).Times(1);

    SyncCommand cmd;
    cmd.setDispatcher(&dispatcher);
    cmd.run(&store);
}

// ── async command lifecycle ───────────────────────────────────────────────────

TEST_F(tst_TeCommandBase, async_run_calls_execute_but_not_finalize_immediately)
{
    EXPECT_CALL(dispatcher, requestCommandFinalize(_)).Times(0);

    AsyncCommand cmd;
    cmd.setDispatcher(&dispatcher);
    cmd.run(&store);
    EXPECT_TRUE(cmd.executed);
}

TEST_F(tst_TeCommandBase, async_finished_triggers_finalize)
{
    EXPECT_CALL(dispatcher, requestCommandFinalize(_)).Times(1);

    AsyncCommand cmd;
    cmd.setDispatcher(&dispatcher);
    cmd.run(&store);
    cmd.completeAsync(); // calls finished() internally
}

// ── setSource / setDefaultParam ───────────────────────────────────────────────

TEST_F(tst_TeCommandBase, setSource_does_not_crash)
{
    SyncCommand cmd;
    QKeyEvent keyEv(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
    EXPECT_NO_THROW(cmd.setSource(TeTypes::WT_LISTVIEW, &keyEv, nullptr));
}

TEST_F(tst_TeCommandBase, setDefaultParam_does_not_crash)
{
    SyncCommand cmd;
    TeCmdParam param;
    EXPECT_NO_THROW(cmd.setDefaultParam(param));
}

// ── TeCommandFactory singleton ────────────────────────────────────────────────

TEST(tst_TeCommandFactory, factory_returns_non_null)
{
    EXPECT_NE(TeCommandFactory::factory(), nullptr);
}

TEST(tst_TeCommandFactory, factory_returns_same_instance)
{
    EXPECT_EQ(TeCommandFactory::factory(), TeCommandFactory::factory());
}
