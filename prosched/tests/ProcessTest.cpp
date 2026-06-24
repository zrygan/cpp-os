#include "scheduler/process/Process.h"
#include <gtest/gtest.h>

// Helper: parse a raw instruction string and add each resulting Statement.
static void AddRaw(prosched::Process &p, const std::string &src) {
  prosched::Interpreter interp;
  auto stmts = interp.parse(src);
  for (auto &s : stmts)
    p.AddInstruction(s);
}

namespace ProcessAddInstruction {
prosched::Interpreter interpreter;

// AddInstruction returns the original instruction string on success
TEST(ProcessAddInstruction, ValidPrintReturnsInstruction) {
  prosched::Process p("add_1", 1, 0);

  std::vector<prosched::Statement> stmts =
      interpreter.parse("PRINT(\"hello\")");
  ASSERT_FALSE(stmts.empty());

  prosched::Statement stmt = stmts[0];
  prosched::Statement *result = p.AddInstruction(stmt);

  EXPECT_EQ(result->keyword, prosched::Keyword::PRINT);
  ASSERT_FALSE(result->args.empty());
  EXPECT_EQ(result->args[0], "\"hello\"");
}

// Works with non-PRINT instructions (DECLARE) too
TEST(ProcessAddInstruction, ValidDeclareReturnsInstruction) {
  prosched::Process p("add_2", 2, 0);

  std::vector<prosched::Statement> stmts = interpreter.parse("DECLARE(x, 10)");
  ASSERT_FALSE(stmts.empty());

  prosched::Statement stmt = stmts[0];
  prosched::Statement *result = p.AddInstruction(stmt);

  EXPECT_EQ(result->keyword, prosched::Keyword::DECLARE);
  ASSERT_FALSE(result->args.empty());
  ASSERT_EQ(result->args.size(), 2);
  EXPECT_EQ(result->args[0], "x");
  EXPECT_EQ(result->args[1], "10");
}

// Repeated AddInstruction calls all succeed — no internal cap
TEST(ProcessAddInstruction, MultipleInstructionsAllSucceed) {
  prosched::Process p("add_3", 3, 0);

  for (int i = 0; i < 10; i++) {
    auto stmts = interpreter.parse("PRINT(\"line\")");
    ASSERT_FALSE(stmts.empty()) << "Parse failed at index " << i;
    prosched::Statement *result = p.AddInstruction(stmts[0]);
    EXPECT_NE(result, nullptr) << "AddInstruction failed at index " << i;
  }
}

// Unknown keyword is absorbed by the interpreter — should not throw
TEST(ProcessAddInstruction, UnknownKeywordDoesNotCrash) {
  prosched::Process p("add_4", 4, 0);
  EXPECT_NO_THROW({
    auto stmts = interpreter.parse("UNKNOWNCMD(x)");
    for (auto &s : stmts)
      p.AddInstruction(s);
  });
}

// Empty string produces no statements — should not crash
TEST(ProcessAddInstruction, EmptyStringDoesNotCrash) {
  prosched::Process p("add_5", 5, 0);
  EXPECT_NO_THROW({
    auto stmts = interpreter.parse("");
    for (auto &s : stmts)
      p.AddInstruction(s);
  });
}

// Two separate Process instances must not share their statement vectors
TEST(ProcessAddInstruction, IndependentProcessesDoNotShareInstructions) {
  prosched::Process p1("add_6a", 6, 0);
  prosched::Process p2("add_6b", 7, 0);

  AddRaw(p1, "PRINT(\"from p1\")");

  // p2 has no instructions — executing should finish it immediately (empty
  // return)
  auto result = p2.ExecuteInstructions(1);
  EXPECT_TRUE(result.empty());
  EXPECT_TRUE(p2.IsFinished());
}

} // namespace ProcessAddInstruction

namespace ProcessExecuteInstructions {

// Each call should advance by exactly one statement
TEST(ProcessExecuteInstructions, ExecutesOneStatementPerCall) {
  prosched::Process p("exec_1", 1, 0);
  AddRaw(p, "PRINT(\"a\")");
  AddRaw(p, "PRINT(\"b\")");
  AddRaw(p, "PRINT(\"c\")");

  p.ExecuteInstructions(1);
  p.ExecuteInstructions(1);

  EXPECT_EQ(p.GetLogs().size(), 2u);
  EXPECT_FALSE(p.IsFinished());
}

// Should return a non-empty vector while there are still statements left
TEST(ProcessExecuteInstructions, ReturnsStatementsWhileRunning) {
  prosched::Process p("exec_2", 2, 0);
  AddRaw(p, "PRINT(\"a\")");
  AddRaw(p, "PRINT(\"b\")");

  auto result = p.ExecuteInstructions(1);
  EXPECT_FALSE(result.empty());
}

// Should return empty vector when called on an already-finished process
TEST(ProcessExecuteInstructions, CallingAfterFinishedReturnsEmpty) {
  prosched::Process p("exec_3", 3, 0);
  AddRaw(p, "PRINT(\"a\")");

  p.ExecuteInstructions(1);               // finishes the process
  auto result = p.ExecuteInstructions(1); // already finished

  EXPECT_TRUE(result.empty());
}

// A process with no instructions should finish on the first call
TEST(ProcessExecuteInstructions, NoInstructionsFinishesOnFirstCall) {
  prosched::Process p("exec_4", 4, 0);
  p.ExecuteInstructions(1);
  EXPECT_TRUE(p.IsFinished());
}

// Non-PRINT instructions execute without generating logs
TEST(ProcessExecuteInstructions, DeclareInstructionProducesNoLog) {
  prosched::Process p("exec_5", 5, 0);
  AddRaw(p, "DECLARE(x, 42)");
  p.ExecuteInstructions(1);

  EXPECT_TRUE(p.GetLogs().empty());
}

// Arithmetic result is visible when PRINTed after ADD
TEST(ProcessExecuteInstructions, AddResultVisibleInPrint) {
  prosched::Process p("exec_6", 6, 0);
  AddRaw(p, "DECLARE(x, 3)");
  AddRaw(p, "DECLARE(y, 7)");
  AddRaw(p, "ADD(z, x, y)");
  AddRaw(p, "PRINT(z)");

  p.ExecuteInstructions(1); // DECLARE x
  p.ExecuteInstructions(1); // DECLARE y
  p.ExecuteInstructions(1); // ADD z = 10
  p.ExecuteInstructions(1); // PRINT z → "10"

  auto logs = p.GetLogs();
  ASSERT_FALSE(logs.empty());
  EXPECT_NE(logs.back().find("10"), std::string::npos);
}

} // namespace ProcessExecuteInstructions

namespace ProcessIsFinished {

// Process starts not finished before any execution
TEST(ProcessIsFinished, FalseOnConstruction) {
  prosched::Process p("fin_1", 1, 0);
  EXPECT_FALSE(p.IsFinished());
}

// Not finished until the very last instruction runs
TEST(ProcessIsFinished, FalseAfterPartialExecution) {
  prosched::Process p("fin_2", 2, 0);
  AddRaw(p, "PRINT(\"a\")");
  AddRaw(p, "PRINT(\"b\")");

  p.ExecuteInstructions(1); // only first of two done
  EXPECT_FALSE(p.IsFinished());
}

// Finishes exactly on the call that runs the last instruction
TEST(ProcessIsFinished, TrueAfterLastInstructionRuns) {
  prosched::Process p("fin_3", 3, 0);
  AddRaw(p, "PRINT(\"a\")");

  p.ExecuteInstructions(1);
  EXPECT_TRUE(p.IsFinished());
}

// Finishes after all N instructions have each been executed once
TEST(ProcessIsFinished, TrueAfterAllOfManyInstructions) {
  prosched::Process p("fin_4", 4, 0);
  const int count = 5;
  for (int i = 0; i < count; i++)
    AddRaw(p, "PRINT(\"x\")");

  for (int i = 0; i < count; i++)
    p.ExecuteInstructions(1);

  EXPECT_TRUE(p.IsFinished());
}

// Calling again after finish must not flip back to not-finished
TEST(ProcessIsFinished, RemainsFinishedAfterExtraCall) {
  prosched::Process p("fin_5", 5, 0);
  AddRaw(p, "PRINT(\"a\")");

  p.ExecuteInstructions(1);
  p.ExecuteInstructions(1); // extra call on finished process
  EXPECT_TRUE(p.IsFinished());
}

} // namespace ProcessIsFinished

namespace ProcessLogs {

// Logs are empty before any execution
TEST(ProcessLogs, EmptyBeforeExecution) {
  prosched::Process p("log_1", 1, 0);
  AddRaw(p, "PRINT(\"hello\")");
  EXPECT_TRUE(p.GetLogs().empty());
}

// Each PRINT statement adds exactly one log entry
TEST(ProcessLogs, PrintAddsOneLogEntry) {
  prosched::Process p("log_2", 2, 0);
  AddRaw(p, "PRINT(\"hello\")");
  p.ExecuteInstructions(1);
  EXPECT_EQ(p.GetLogs().size(), 1u);
}

// Log entry must contain the string that was printed
TEST(ProcessLogs, LogContainsPrintedValue) {
  prosched::Process p("log_3", 3, 0);
  AddRaw(p, "PRINT(\"hello world\")");
  p.ExecuteInstructions(1);

  ASSERT_FALSE(p.GetLogs().empty());
  EXPECT_NE(p.GetLogs()[0].find("hello world"), std::string::npos);
}

// Log entry must contain the core number passed to ExecuteInstructions
TEST(ProcessLogs, LogContainsCoreNumber) {
  prosched::Process p("log_4", 4, 0);
  AddRaw(p, "PRINT(\"msg\")");
  p.ExecuteInstructions(3); // core 3

  ASSERT_FALSE(p.GetLogs().empty());
  EXPECT_NE(p.GetLogs()[0].find("Core:3"), std::string::npos);
}

// Non-PRINT instructions must not add log entries
TEST(ProcessLogs, NonPrintInstructionAddsNoLog) {
  prosched::Process p("log_5", 5, 0);
  AddRaw(p, "DECLARE(x, 10)");
  p.ExecuteInstructions(1);
  EXPECT_TRUE(p.GetLogs().empty());
}

// Logs accumulate across multiple executions
TEST(ProcessLogs, LogsAccumulateAcrossCalls) {
  prosched::Process p("log_6", 6, 0);
  AddRaw(p, "PRINT(\"a\")");
  AddRaw(p, "PRINT(\"b\")");
  AddRaw(p, "PRINT(\"c\")");

  p.ExecuteInstructions(1);
  p.ExecuteInstructions(1);
  p.ExecuteInstructions(1);

  EXPECT_EQ(p.GetLogs().size(), 3u);
}

// Two processes must not share log state
TEST(ProcessLogs, IndependentProcessesDoNotShareLogs) {
  prosched::Process p1("log_7a", 7, 0);
  prosched::Process p2("log_7b", 8, 0);

  AddRaw(p1, "PRINT(\"from p1\")");
  p1.ExecuteInstructions(1);

  EXPECT_TRUE(p2.GetLogs().empty());
}

} // namespace ProcessLogs

namespace ProcessAssignCore {

// Returns the assigned core number and updates GetCoreNum
TEST(ProcessAssignCore, PositiveValueReturnsAndSetsCore) {
  prosched::Process p("core_1", 1, 0);
  EXPECT_EQ(p.AssignCore(2), 2);
  EXPECT_EQ(p.GetCoreNum(), 2);
}

// Core 0 is valid after the coreNum >= 0 fix
TEST(ProcessAssignCore, ZeroIsValidReturnsAndSetsCore) {
  prosched::Process p("core_2", 2, 0);
  EXPECT_EQ(p.AssignCore(0), 0);
  EXPECT_EQ(p.GetCoreNum(), 0);
}

// Negative input is rejected; previously set core value is preserved
TEST(ProcessAssignCore, NegativeReturnsMinus1AndLeavesCorUnchanged) {
  prosched::Process p("core_3", 3, 0);
  p.AssignCore(5);
  EXPECT_EQ(p.AssignCore(-1), -1);
  EXPECT_EQ(p.GetCoreNum(), 5);
}

} // namespace ProcessAssignCore

namespace ProcessOwnership {

// A fresh Process is not scheduler-owned by default
TEST(ProcessOwnership, DefaultIsFalse) {
  prosched::Process p("own_1", 1, 0);
  EXPECT_FALSE(p.IsOwnedByScheduler());
}

// SetOwnedByScheduler(true) is immediately visible through the getter
TEST(ProcessOwnership, SetTrueReflectsInGetter) {
  prosched::Process p("own_2", 2, 0);
  p.SetOwnedByScheduler(true);
  EXPECT_TRUE(p.IsOwnedByScheduler());
}

// Ownership can be toggled back to false
TEST(ProcessOwnership, SetFalseAfterTrueReflectsInGetter) {
  prosched::Process p("own_3", 3, 0);
  p.SetOwnedByScheduler(true);
  p.SetOwnedByScheduler(false);
  EXPECT_FALSE(p.IsOwnedByScheduler());
}

} // namespace ProcessOwnership

namespace ProcessIdentity {

// GetPID returns exactly the value given to the constructor
TEST(ProcessIdentity, GetPIDReturnsCtrValue) {
  prosched::Process p("id_1", 42, 0);
  EXPECT_EQ(p.GetPID(), 42);
}

// GetName returns exactly the value given to the constructor
TEST(ProcessIdentity, GetNameReturnsCtrValue) {
  prosched::Process p("my_process", 1, 0);
  EXPECT_EQ(p.GetName(), "my_process");
}

} // namespace ProcessIdentity
