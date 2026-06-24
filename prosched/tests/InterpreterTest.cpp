#include "commands/Interpreter.hpp"
#include <gtest/gtest.h>

static prosched::Statement
makeStmt(prosched::Keyword kw, std::vector<std::string> args = {},
         std::vector<prosched::Statement> nested = {}) {
  prosched::Statement s;
  s.keyword = kw;
  s.args = std::move(args);
  s.nested = std::move(nested);
  return s;
}

// ─── flushBuffer
// ──────────────────────────────────────────────────────────────

namespace InterpreterFlushBuffer {

// No output produced yet — flush returns an empty vector
TEST(InterpreterFlushBuffer, EmptyBufferReturnsEmptyVector) {
  prosched::Interpreter interp;
  EXPECT_TRUE(interp.flushBuffer().empty());
}

// A PRINT call adds one entry; flush returns it
TEST(InterpreterFlushBuffer, ReturnsAccumulatedOutput) {
  prosched::Interpreter interp;
  interp.executeString(R"(PRINT("hello"))");
  auto buf = interp.flushBuffer();
  ASSERT_EQ(buf.size(), 1u);
  EXPECT_EQ(buf[0], "hello");
}

// Second flush after the first returns empty — buffer was cleared
TEST(InterpreterFlushBuffer, ClearedAfterFlush) {
  prosched::Interpreter interp;
  interp.executeString(R"(PRINT("hello"))");
  interp.flushBuffer();
  EXPECT_TRUE(interp.flushBuffer().empty());
}

// All three PRINT outputs accumulate and are returned in order before flush
TEST(InterpreterFlushBuffer, MultiplePrintsAllAppearBeforeFlush) {
  prosched::Interpreter interp;
  interp.executeString(R"(PRINT("a"), PRINT("b"), PRINT("c"))");
  auto buf = interp.flushBuffer();
  ASSERT_EQ(buf.size(), 3u);
  EXPECT_EQ(buf[0], "a");
  EXPECT_EQ(buf[1], "b");
  EXPECT_EQ(buf[2], "c");
}

} // namespace InterpreterFlushBuffer

// ─── executeString
// ────────────────────────────────────────────────────────────

namespace InterpreterExecuteString {

// Parsing empty input must not throw or produce output
TEST(InterpreterExecuteString, EmptyStringNocrash) {
  prosched::Interpreter interp;
  EXPECT_NO_THROW(interp.executeString(""));
  EXPECT_TRUE(interp.flushBuffer().empty());
}

// Whitespace-only input is treated the same as empty
TEST(InterpreterExecuteString, WhitespaceOnlyNocrash) {
  prosched::Interpreter interp;
  EXPECT_NO_THROW(interp.executeString("   "));
  EXPECT_TRUE(interp.flushBuffer().empty());
}

// PRINT("hello world") buffers the literal string value
TEST(InterpreterExecuteString, ValidPrintProducesOutput) {
  prosched::Interpreter interp;
  interp.executeString(R"(PRINT("hello world"))");
  auto buf = interp.flushBuffer();
  ASSERT_EQ(buf.size(), 1u);
  EXPECT_EQ(buf[0], "hello world");
}

// Unrecognized keywords push a "[!]" warning entry to the buffer
TEST(InterpreterExecuteString, UnknownKeywordPushesWarning) {
  prosched::Interpreter interp;
  interp.executeString("UNKNOWNCMD(x)");
  auto buf = interp.flushBuffer();
  ASSERT_FALSE(buf.empty());
  EXPECT_NE(buf[0].find("[!]"), std::string::npos);
}

// DECLARE stores a value; the subsequent PRINT reads it correctly
TEST(InterpreterExecuteString, DeclareAndPrintWorkTogether) {
  prosched::Interpreter interp;
  interp.executeString("DECLARE(x, 42), PRINT(x)");
  auto buf = interp.flushBuffer();
  ASSERT_EQ(buf.size(), 1u);
  EXPECT_EQ(buf[0], "42");
}

// DECLARE+ADD pipeline: PRINT shows the computed sum
TEST(InterpreterExecuteString, AddAndPrintWorkTogether) {
  prosched::Interpreter interp;
  interp.executeString("DECLARE(a, 3), DECLARE(b, 4), ADD(c, a, b), PRINT(c)");
  auto buf = interp.flushBuffer();
  ASSERT_EQ(buf.size(), 1u);
  EXPECT_EQ(buf[0], "7");
}

} // namespace InterpreterExecuteString

// ─── executePrint
// ─────────────────────────────────────────────────────────────

namespace InterpreterExecutePrint {

// Quoted arg strips the quotes; inner text is returned and buffered
TEST(InterpreterExecutePrint, StringLiteralReturnsCorrectly) {
  prosched::Interpreter interp;
  auto stmt = makeStmt(prosched::Keyword::PRINT, {R"("hello")"});
  EXPECT_EQ(interp.executePrint(stmt), "hello");
  auto buf = interp.flushBuffer();
  ASSERT_EQ(buf.size(), 1u);
  EXPECT_EQ(buf[0], "hello");
}

// Empty quoted string prints and buffers an empty-string entry
TEST(InterpreterExecutePrint, EmptyStringLiteralReturnsEmpty) {
  prosched::Interpreter interp;
  auto stmt = makeStmt(prosched::Keyword::PRINT, {R"("")"});
  EXPECT_EQ(interp.executePrint(stmt), "");
  auto buf = interp.flushBuffer();
  ASSERT_EQ(buf.size(), 1u);
  EXPECT_EQ(buf[0], "");
}

// Undeclared variable resolves to "0" — the default value
TEST(InterpreterExecutePrint, UndeclaredVariablePrintsZero) {
  prosched::Interpreter interp;
  auto stmt = makeStmt(prosched::Keyword::PRINT, {"notdeclared"});
  EXPECT_EQ(interp.executePrint(stmt), "0");
}

// "str=" + var concatenates a string literal with a variable's value
TEST(InterpreterExecutePrint, ConcatStringAndVariable) {
  prosched::Interpreter interp;
  interp.executeDeclare(makeStmt(prosched::Keyword::DECLARE, {"x", "5"}));
  interp.flushBuffer();

  auto stmt = makeStmt(prosched::Keyword::PRINT, {R"("val=" + x)"});
  EXPECT_EQ(interp.executePrint(stmt), "val=5");
}

} // namespace InterpreterExecutePrint

// ─── executeDeclare
// ───────────────────────────────────────────────────────────

namespace InterpreterExecuteDeclare {

// Returns the stored uint16 value on success
TEST(InterpreterExecuteDeclare, BasicDeclarationReturnsValue) {
  prosched::Interpreter interp;
  auto stmt = makeStmt(prosched::Keyword::DECLARE, {"x", "10"});
  auto result = interp.executeDeclare(stmt);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result.value(), 10);
}

// Zero is a valid uint16 value
TEST(InterpreterExecuteDeclare, ZeroValueReturnsZero) {
  prosched::Interpreter interp;
  auto stmt = makeStmt(prosched::Keyword::DECLARE, {"x", "0"});
  auto result = interp.executeDeclare(stmt);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result.value(), 0);
}

// 65535 is the uint16 max — must not overflow or truncate
TEST(InterpreterExecuteDeclare, MaxUint16ReturnsCorrectly) {
  prosched::Interpreter interp;
  auto stmt = makeStmt(prosched::Keyword::DECLARE, {"x", "65535"});
  auto result = interp.executeDeclare(stmt);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result.value(), 65535);
}

// Single-arg DECLARE is malformed — returns nullopt without crashing
TEST(InterpreterExecuteDeclare, MissingSecondArgReturnsNullopt) {
  prosched::Interpreter interp;
  auto stmt = makeStmt(prosched::Keyword::DECLARE, {"x"});
  EXPECT_FALSE(interp.executeDeclare(stmt).has_value());
}

// Declared variable is accessible in subsequent PRINT calls
TEST(InterpreterExecuteDeclare, VariableIsAvailableForSubsequentPrint) {
  prosched::Interpreter interp;
  interp.executeDeclare(makeStmt(prosched::Keyword::DECLARE, {"myvar", "42"}));
  interp.executePrint(makeStmt(prosched::Keyword::PRINT, {"myvar"}));
  auto buf = interp.flushBuffer();
  ASSERT_EQ(buf.size(), 1u);
  EXPECT_EQ(buf[0], "42");
}

} // namespace InterpreterExecuteDeclare

// ─── executeAdd
// ───────────────────────────────────────────────────────────────

namespace InterpreterExecuteAdd {

// Reads two declared vars, computes their sum, stores in a new var
TEST(InterpreterExecuteAdd, TwoVariablesAdded) {
  prosched::Interpreter interp;
  interp.executeDeclare(makeStmt(prosched::Keyword::DECLARE, {"a", "3"}));
  interp.executeDeclare(makeStmt(prosched::Keyword::DECLARE, {"b", "7"}));
  auto result =
      interp.executeAdd(makeStmt(prosched::Keyword::ADD, {"c", "a", "b"}));
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result.value(), 10);
}

// Numeric string literals are parsed inline — no prior DECLARE needed
TEST(InterpreterExecuteAdd, LiteralNumericOperands) {
  prosched::Interpreter interp;
  auto result =
      interp.executeAdd(makeStmt(prosched::Keyword::ADD, {"z", "5", "3"}));
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result.value(), 8);
}

// dest can be one of the source operands — overwrites correctly
TEST(InterpreterExecuteAdd, OverwritesExistingVariable) {
  prosched::Interpreter interp;
  interp.executeDeclare(makeStmt(prosched::Keyword::DECLARE, {"x", "5"}));
  auto result =
      interp.executeAdd(makeStmt(prosched::Keyword::ADD, {"x", "x", "3"}));
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result.value(), 8);
}

// Fewer than 3 args is malformed — returns nullopt
TEST(InterpreterExecuteAdd, MissingArgsReturnsNullopt) {
  prosched::Interpreter interp;
  auto result = interp.executeAdd(makeStmt(prosched::Keyword::ADD, {"x", "5"}));
  EXPECT_FALSE(result.has_value());
}

} // namespace InterpreterExecuteAdd

// ─── executeSubtract
// ──────────────────────────────────────────────────────────

namespace InterpreterExecuteSubtract {

// Subtracts two declared variables and stores the result in a new var
TEST(InterpreterExecuteSubtract, BasicSubtractionCorrect) {
  prosched::Interpreter interp;
  interp.executeDeclare(makeStmt(prosched::Keyword::DECLARE, {"a", "10"}));
  interp.executeDeclare(makeStmt(prosched::Keyword::DECLARE, {"b", "3"}));
  auto result = interp.executeSubtract(
      makeStmt(prosched::Keyword::SUBTRACT, {"c", "a", "b"}));
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result.value(), 7);
}

// uint16 underflow wraps around — no exception thrown
TEST(InterpreterExecuteSubtract, Uint16UnderflowWrapsWithoutThrow) {
  prosched::Interpreter interp;
  interp.executeDeclare(makeStmt(prosched::Keyword::DECLARE, {"a", "3"}));
  interp.executeDeclare(makeStmt(prosched::Keyword::DECLARE, {"b", "10"}));
  auto result = interp.executeSubtract(
      makeStmt(prosched::Keyword::SUBTRACT, {"c", "a", "b"}));
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result.value(), static_cast<uint16_t>(3 - 10));
}

// Fewer than 3 args is malformed — returns nullopt
TEST(InterpreterExecuteSubtract, MissingArgsReturnsNullopt) {
  prosched::Interpreter interp;
  auto result =
      interp.executeSubtract(makeStmt(prosched::Keyword::SUBTRACT, {"x", "5"}));
  EXPECT_FALSE(result.has_value());
}

} // namespace InterpreterExecuteSubtract

// ─── executeSleep
// ─────────────────────────────────────────────────────────────

namespace InterpreterExecuteSleep {

// SLEEP(0) is a no-op and must not block the thread
TEST(InterpreterExecuteSleep, ZeroTicksCompletesWithoutHang) {
  prosched::Interpreter interp;
  EXPECT_NO_THROW(
      interp.executeSleep(makeStmt(prosched::Keyword::SLEEP, {"0"})));
}

// SLEEP(1) completes in finite time
TEST(InterpreterExecuteSleep, OneTickCompletes) {
  prosched::Interpreter interp;
  EXPECT_NO_THROW(
      interp.executeSleep(makeStmt(prosched::Keyword::SLEEP, {"1"})));
}

} // namespace InterpreterExecuteSleep

// ─── executeFor
// ───────────────────────────────────────────────────────────────

namespace InterpreterExecuteFor {

// FOR body runs exactly N times, one PRINT per iteration
TEST(InterpreterExecuteFor, BasicThreeIterations) {
  prosched::Interpreter interp;
  interp.executeString(R"(FOR([PRINT("x")], 3))");
  auto buf = interp.flushBuffer();
  ASSERT_EQ(buf.size(), 3u);
  for (const auto &line : buf)
    EXPECT_EQ(line, "x");
}

// FOR with count 0 skips the body entirely
TEST(InterpreterExecuteFor, ZeroIterationsProducesNothing) {
  prosched::Interpreter interp;
  interp.executeString(R"(FOR([PRINT("x")], 0))");
  EXPECT_TRUE(interp.flushBuffer().empty());
}

// Inner loop (2) × outer loop (3) = 6 total PRINT calls
TEST(InterpreterExecuteFor, NestedForMultipliesIterationCount) {
  prosched::Interpreter interp;
  interp.executeString(R"(FOR([FOR([PRINT("n")], 2)], 3))");
  auto buf = interp.flushBuffer();
  EXPECT_EQ(buf.size(), 6u);
}

} // namespace InterpreterExecuteFor

// ─── executeDebug
// ─────────────────────────────────────────────────────────────

namespace InterpreterExecuteDebug {

// No variables declared — buffer contains the "[No variables declared]" message
TEST(InterpreterExecuteDebug, EmptyMemoryShowsNoVarsMessage) {
  prosched::Interpreter interp;
  interp.executeDebug();
  auto buf = interp.flushBuffer();
  bool found = false;
  for (const auto &line : buf)
    if (line.find("[No variables declared]") != std::string::npos)
      found = true;
  EXPECT_TRUE(found);
}

// Declared variable name appears somewhere in the debug output lines
TEST(InterpreterExecuteDebug, DeclaredVariableAppearsInDump) {
  prosched::Interpreter interp;
  interp.executeDeclare(makeStmt(prosched::Keyword::DECLARE, {"myvar", "99"}));
  interp.flushBuffer();
  interp.executeDebug();
  auto buf = interp.flushBuffer();
  bool found = false;
  for (const auto &line : buf)
    if (line.find("myvar") != std::string::npos)
      found = true;
  EXPECT_TRUE(found);
}

// Return value is the live memory map containing all declared variables
TEST(InterpreterExecuteDebug, ReturnsMemoryMapWithAllDeclaredVars) {
  prosched::Interpreter interp;
  interp.executeDeclare(makeStmt(prosched::Keyword::DECLARE, {"a", "1"}));
  interp.executeDeclare(makeStmt(prosched::Keyword::DECLARE, {"b", "2"}));
  auto mem = interp.executeDebug();
  EXPECT_EQ(mem.at("a"), 1);
  EXPECT_EQ(mem.at("b"), 2);
}

} // namespace InterpreterExecuteDebug
