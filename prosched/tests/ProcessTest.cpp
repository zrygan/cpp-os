#include <gtest/gtest.h>
#include "scheduler/process/Process.h"
#include "commands/Command.h"
#include <memory>

namespace ProcessAddComandTest {

// A valid command returns the same Command* that was passed in
TEST(ProcessAddCommand, ReturnedPointerMatchesInput) {
    Process p("proc", 1, 0);
    std::shared_ptr<Command> cmd = std::make_shared<Command>();

    std::shared_ptr<Command> *result = p.AddCommand(cmd);

    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->get(), cmd.get());
}

// Adding multiple commands should all succeed
TEST(ProcessAddCommand, AddMultipleCommands) {
    Process p("proc", 1, 0);

    for (int i = 0; i < 5; i++) {
        std::shared_ptr<Command> cmd = std::make_shared<Command>();
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
        std::shared_ptr<Command> cmd = std::make_shared<Command>();
        std::shared_ptr<Command> *result = p.AddCommand(cmd);
        EXPECT_NE(result, nullptr) << "Failed on command index " << i;
    }
}

// Adding the same shared_ptr instance twice should succeed both times
TEST(ProcessAddCommand, SameCommandAddedTwice) {
    Process p("proc", 1, 0);
    std::shared_ptr<Command> cmd = std::make_shared<Command>();

    std::shared_ptr<Command> *r1 = p.AddCommand(cmd);
    std::shared_ptr<Command> *r2 = p.AddCommand(cmd);

    EXPECT_NE(r1, nullptr);
    EXPECT_NE(r2, nullptr);
}

// Interleaving valid and null adds: valid ones must still succeed
// and nulls must still return nullptr
TEST(ProcessAddCommand, InterleavedValidAndNull) {
    Process p("proc", 1, 0);
    std::shared_ptr<Command> cmd1 = std::make_shared<Command>();
    std::shared_ptr<Command> cmd2 = std::make_shared<Command>();

    std::shared_ptr<Command> *r1   = p.AddCommand(cmd1);
    std::shared_ptr<Command> *null = p.AddCommand(nullptr);
    std::shared_ptr<Command> *r2   = p.AddCommand(cmd2);

    EXPECT_NE(r1,   nullptr);
    EXPECT_EQ(null, nullptr);
    EXPECT_NE(r2,   nullptr);
}

// Two separate Process instances must not share their command vectors
TEST(ProcessAddCommand, IndependentProcessesDoNotShareCommands) {
    Process p1("proc1", 1, 0);
    Process p2("proc2", 2, 0);
    std::shared_ptr<Command> cmd = std::make_shared<Command>();

    std::shared_ptr<Command> *r1 = p1.AddCommand(cmd);

    EXPECT_NE(r1, nullptr);
    EXPECT_EQ(p2.ExecuteCurrentCommand(0), nullptr);
}

} // namespace ProcessAddComandTest

namespace ProcessExecuteCommand {

// Executing a valid index should return the Command that lives at that slot
TEST(ProcessExecuteCurrentCommand, ValidIndexReturnsCommand) {
    Process p("proc", 1, 0);
    std::shared_ptr<Command> cmd = std::make_shared<Command>();
    p.AddCommand(cmd);

    Command *result = p.ExecuteCurrentCommand(0);

    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result, cmd.get());
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

    Command *result = p.ExecuteCurrentCommand(1); // only index 0 is valid

    EXPECT_EQ(result, nullptr);
}

// INT_MAX index should be handled safely without crashing
TEST(ProcessExecuteCurrentCommand, LargeIndexReturnsNull) {
    Process p("proc", 1, 0);
    p.AddCommand(std::make_shared<Command>());

    Command *result = p.ExecuteCurrentCommand(INT_MAX);

    EXPECT_EQ(result, nullptr);
}

// Each index in sequence should return its own Command
TEST(ProcessExecuteCurrentCommand, SequentialExecution) {
    Process p("proc", 1, 0);
    std::shared_ptr<Command> cmd1 = std::make_shared<Command>();
    std::shared_ptr<Command> cmd2 = std::make_shared<Command>();
    std::shared_ptr<Command> cmd3 = std::make_shared<Command>();
    p.AddCommand(cmd1);
    p.AddCommand(cmd2);
    p.AddCommand(cmd3);

    Command *r0 = p.ExecuteCurrentCommand(0);
    Command *r1 = p.ExecuteCurrentCommand(1);
    Command *r2 = p.ExecuteCurrentCommand(2);

    EXPECT_EQ(r0, cmd1.get());
    EXPECT_EQ(r1, cmd2.get());
    EXPECT_EQ(r2, cmd3.get());
}

// INT_MIN index should be handled safely — no crash
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
    std::shared_ptr<Command> last = std::make_shared<Command>();
    p.AddCommand(last);

    Command *result = p.ExecuteCurrentCommand(2);

    EXPECT_EQ(result, last.get());
}

// Re-executing the same index twice must not remove or corrupt the command
TEST(ProcessExecuteCurrentCommand, SameIndexExecutedTwice) {
    Process p("proc", 1, 0);
    std::shared_ptr<Command> cmd = std::make_shared<Command>();
    p.AddCommand(cmd);

    Command *r1 = p.ExecuteCurrentCommand(0);
    Command *r2 = p.ExecuteCurrentCommand(0);

    EXPECT_EQ(r1, cmd.get());
    EXPECT_EQ(r2, cmd.get());
}

// A bad call must not poison state — valid call after OOB must still succeed
TEST(ProcessExecuteCurrentCommand, ValidAfterOutOfBoundsStillWorks) {
    Process p("proc", 1, 0);
    std::shared_ptr<Command> cmd = std::make_shared<Command>();
    p.AddCommand(cmd);

    p.ExecuteCurrentCommand(99);
    Command *result = p.ExecuteCurrentCommand(0);

    EXPECT_EQ(result, cmd.get());
}

} // namespace ProcessExecuteCommand
