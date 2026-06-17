#include <gtest/gtest.h>
#include "scheduler/process/Process.h"
#include "commands/Command.h"
#include <memory>

namespace ProcessAddComandTest{
    // The returned pointer should reference the same Command object that was added
    TEST(ProcessAddCommand, ReturnedPointerMatchesInput) {
        Process p("proc", 1, 0);
        auto cmd = std::make_shared<Command>();

        std::shared_ptr<Command> *result = p.AddCommand(cmd);

        ASSERT_NE(result, nullptr);
        EXPECT_EQ(result->get(), cmd.get());
    }

    // Adding multiple commands should all succeed
    TEST(ProcessAddCommand, AddMultipleCommands) {
        Process p("proc", 1, 0);

        for (int i = 0; i < 5; i++) {
            auto cmd = std::make_shared<Command>();
            std::shared_ptr<Command> *result = p.AddCommand(cmd);
            EXPECT_NE(result, nullptr) << "Failed on command index " << i;
        }
    }

    // Adding a nullptr shared_ptr should return nullptr (nothing to register)
    TEST(ProcessAddCommand, NullCommandReturnsNull) {
        Process p("proc", 1, 0);

        std::shared_ptr<Command> *result = p.AddCommand(nullptr);

        EXPECT_EQ(result, nullptr);
    }

    // Adding a large number of commands should not crash (no artificial cap)
    TEST(ProcessAddCommand, ManyCommands) {
        Process p("proc", 1, 0);

        for (int i = 0; i < 1000; i++) {
            auto cmd = std::make_shared<Command>();
            std::shared_ptr<Command> *result = p.AddCommand(cmd);
            EXPECT_NE(result, nullptr) << "Failed on command index " << i;
        }
    }

    // Adding the same shared_ptr instance twice should succeed both times —
    // the same object can legitimately appear more than once in the list
    TEST(ProcessAddCommand, SameCommandAddedTwice) {
        Process p("proc", 1, 0);
        auto cmd = std::make_shared<Command>();

        std::shared_ptr<Command> *r1 = p.AddCommand(cmd);
        std::shared_ptr<Command> *r2 = p.AddCommand(cmd);

        EXPECT_NE(r1, nullptr);
        EXPECT_NE(r2, nullptr);
    }

    // Interleaving valid and null adds: valid ones must still succeed
    // and nulls must still return nullptr — neither should affect the other
    TEST(ProcessAddCommand, InterleavedValidAndNull) {
        Process p("proc", 1, 0);

        auto cmd1 = std::make_shared<Command>();
        auto cmd2 = std::make_shared<Command>();

        std::shared_ptr<Command> *r1   = p.AddCommand(cmd1);
        std::shared_ptr<Command> *null = p.AddCommand(nullptr);
        std::shared_ptr<Command> *r2   = p.AddCommand(cmd2);

        EXPECT_NE(r1,   nullptr);
        EXPECT_EQ(null, nullptr);
        EXPECT_NE(r2,   nullptr);
    }

    // Two separate Process instances must not share their command vectors —
    // adding to one must not affect the other
    TEST(ProcessAddCommand, IndependentProcessesDoNotShareCommands) {
        Process p1("proc1", 1, 0);
        Process p2("proc2", 2, 0);
        auto cmd = std::make_shared<Command>();

        std::shared_ptr<Command> *r1 = p1.AddCommand(cmd);

        // p2 has no commands added — executing index 0 must return nullptr
        EXPECT_NE(r1, nullptr);
        EXPECT_EQ(p2.ExecuteCurrentCommand(0), nullptr);
    }
}

namespace ProcessExecuteCommand{
    // Executing a valid index on a process that has commands should return non-null
    TEST(ProcessExecuteCurrentCommand, ValidIndexReturnsCommand) {
        Process p("proc", 1, 0);
        auto cmd = std::make_shared<Command>();

        p.AddCommand(cmd);

        Command *result = p.ExecuteCurrentCommand(0);

        ASSERT_NE(result, nullptr);
        EXPECT_EQ(result->get(), cmd.get());
    }

    // A process with no commands should return nullptr for any index
    TEST(ProcessExecuteCurrentCommand, NoCommandsReturnsNull) {
        Process p("proc", 1, 0);

        Command *result = p.ExecuteCurrentCommand(0);

        EXPECT_EQ(result, nullptr);
    }

    // A negative index is out of bounds — should return nullptr
    TEST(ProcessExecuteCurrentCommand, NegativeIndexReturnsNull) {
        Process p("proc", 1, 0);
        p.AddCommand(std::make_shared<Command>());

        Command *result = p.ExecuteCurrentCommand(-1);

        EXPECT_EQ(result, nullptr);
    }

    // An index past the end of the command list should return nullptr
    TEST(ProcessExecuteCurrentCommand, OutOfBoundsIndexReturnsNull) {
        Process p("proc", 1, 0);
        p.AddCommand(std::make_shared<Command>());

        // only index 0 is valid; 1 is one past the end
        Command *result = p.ExecuteCurrentCommand(1);

        EXPECT_EQ(result, nullptr);
    }

    // INT_MAX index should be handled safely without crashing
    TEST(ProcessExecuteCurrentCommand, LargeIndexReturnsNull) {
        Process p("proc", 1, 0);
        p.AddCommand(std::make_shared<Command>());

        Command *result = p.ExecuteCurrentCommand(INT_MAX);

        EXPECT_EQ(result, nullptr);
    }

    // Executing each command in sequence (index 0, 1, 2) should all return non-null
    TEST(ProcessExecuteCurrentCommand, SequentialExecution) {
        Process p("proc", 1, 0);

        auto cmd1 = std::make_shared<Command>()
        auto cmd2 = std::make_shared<Command>()
        auto cmd3 = std::make_shared<Command>()

        p.AddCommand(cmd1);
        p.AddCommand(cmd2);
        p.AddCommand(cmd3);

        Command *result = p.ExecuteCurrentCommand(0);
        ASSERT_NE(result, nullptr);
        ASSERT_EQ(result->get(), cmd.get());
        Command *result = p.ExecuteCurrentCommand(1);
        ASSERT_NE(result, nullptr);
        ASSERT_EQ(result->get(), cmd.get());
        Command *result = p.ExecuteCurrentCommand(2);
        ASSERT_NE(result, nullptr);
        EXPECT_EQ(result->get(), cmd.get());
    }

    // INT_MIN index (most negative value) should be handled safely — no crash
    TEST(ProcessExecuteCurrentCommand, MostNegativeIndexReturnsNull) {
        Process p("proc", 1, 0);
        p.AddCommand(std::make_shared<Command>());

        Command *result = p.ExecuteCurrentCommand(INT_MIN);

        EXPECT_EQ(result, nullptr);
    }

    // Executing the exact last valid index (size - 1) should return non-null
    TEST(ProcessExecuteCurrentCommand, LastValidIndexReturnsCommand) {
        Process p("proc", 1, 0);
        p.AddCommand(std::make_shared<Command>());
        p.AddCommand(std::make_shared<Command>());
        p.AddCommand(std::make_shared<Command>());

        // last valid index is 2
        Command *result = p.ExecuteCurrentCommand(2);

        EXPECT_NE(result, nullptr);
    }

    // Executing the same valid index twice should return non-null both times —
    // re-executing a command must not corrupt or remove it
    TEST(ProcessExecuteCurrentCommand, SameIndexExecutedTwice) {
        Process p("proc", 1, 0);
        p.AddCommand(std::make_shared<Command>());

        Command *r1 = p.ExecuteCurrentCommand(0);
        Command *r2 = p.ExecuteCurrentCommand(0);

        EXPECT_NE(r1, nullptr);
        EXPECT_NE(r2, nullptr);
    }

    // Executing a valid index right after a failed (out-of-bounds) call
    // should still succeed — a bad call must not poison the process state
    TEST(ProcessExecuteCurrentCommand, ValidAfterOutOfBoundsStillWorks) {
        Process p("proc", 1, 0);
        p.AddCommand(std::make_shared<Command>());

        p.ExecuteCurrentCommand(99);           // bad call
        Command *result = p.ExecuteCurrentCommand(0); // should still work

        EXPECT_NE(result, nullptr);
    }
}