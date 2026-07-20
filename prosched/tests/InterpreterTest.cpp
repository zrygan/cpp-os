#include "commands/Interpreter.h"
#include <chrono>
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

// ─── FlushBuffer
// ──────────────────────────────────────────────────────────────

namespace InterpreterFlushBuffer {

// No output produced yet — flush returns an empty vector
TEST(InterpreterFlushBuffer, EmptyBufferReturnsEmptyVector) {
  prosched::Interpreter interp;
  EXPECT_TRUE(interp.FlushBuffer().empty());
}

// A PRINT call adds one entry; flush returns it
TEST(InterpreterFlushBuffer, ReturnsAccumulatedOutput) {
  prosched::Interpreter interp;
  interp.ExecuteString(R"(PRINT("hello"))");
  auto buf = interp.FlushBuffer();
  ASSERT_EQ(buf.size(), 1u);
  EXPECT_EQ(buf[0], "hello");
}

// Second flush after the first returns empty — buffer was cleared
TEST(InterpreterFlushBuffer, ClearedAfterFlush) {
  prosched::Interpreter interp;
  interp.ExecuteString(R"(PRINT("hello"))");
  interp.FlushBuffer();
  EXPECT_TRUE(interp.FlushBuffer().empty());
}

// All three PRINT outputs accumulate and are returned in order before flush
TEST(InterpreterFlushBuffer, MultiplePrintsAllAppearBeforeFlush) {
  prosched::Interpreter interp;
  interp.ExecuteString(R"(PRINT("a"), PRINT("b"), PRINT("c"))");
  auto buf = interp.FlushBuffer();
  ASSERT_EQ(buf.size(), 3u);
  EXPECT_EQ(buf[0], "a");
  EXPECT_EQ(buf[1], "b");
  EXPECT_EQ(buf[2], "c");
}

} // namespace InterpreterFlushBuffer

// ─── ExecuteString
// ────────────────────────────────────────────────────────────

namespace InterpreterExecuteString {

// Parsing empty input must not throw or produce output
TEST(InterpreterExecuteString, EmptyStringNocrash) {
  prosched::Interpreter interp;
  EXPECT_NO_THROW(interp.ExecuteString(""));
  EXPECT_TRUE(interp.FlushBuffer().empty());
}

// Whitespace-only input is treated the same as empty
TEST(InterpreterExecuteString, WhitespaceOnlyNocrash) {
  prosched::Interpreter interp;
  EXPECT_NO_THROW(interp.ExecuteString("   "));
  EXPECT_TRUE(interp.FlushBuffer().empty());
}

// PRINT("hello world") buffers the literal string value
TEST(InterpreterExecuteString, ValidPrintProducesOutput) {
  prosched::Interpreter interp;
  interp.ExecuteString(R"(PRINT("hello world"))");
  auto buf = interp.FlushBuffer();
  ASSERT_EQ(buf.size(), 1u);
  EXPECT_EQ(buf[0], "hello world");
}

// Unrecognized keywords push a "[!]" warning entry to the buffer
TEST(InterpreterExecuteString, UnknownKeywordPushesWarning) {
  prosched::Interpreter interp;
  interp.ExecuteString("UNKNOWNCMD(x)");
  auto buf = interp.FlushBuffer();
  ASSERT_FALSE(buf.empty());
  EXPECT_NE(buf[0].find("[!]"), std::string::npos);
}

// DECLARE stores a value; the subsequent PRINT reads it correctly
TEST(InterpreterExecuteString, DeclareAndPrintWorkTogether) {
  prosched::Interpreter interp;
  interp.ExecuteString("DECLARE(x, 42), PRINT(x)");
  auto buf = interp.FlushBuffer();
  ASSERT_EQ(buf.size(), 1u);
  EXPECT_EQ(buf[0], "42");
}

// DECLARE+ADD pipeline: PRINT shows the computed sum
TEST(InterpreterExecuteString, AddAndPrintWorkTogether) {
  prosched::Interpreter interp;
  interp.ExecuteString("DECLARE(a, 3), DECLARE(b, 4), ADD(c, a, b), PRINT(c)");
  auto buf = interp.FlushBuffer();
  ASSERT_EQ(buf.size(), 1u);
  EXPECT_EQ(buf[0], "7");
}

} // namespace InterpreterExecuteString

// ─── ExecutePrint
// ─────────────────────────────────────────────────────────────

namespace InterpreterExecutePrint {

// Quoted arg strips the quotes; inner text is returned and buffered
TEST(InterpreterExecutePrint, StringLiteralReturnsCorrectly) {
  prosched::Interpreter interp;
  auto stmt = makeStmt(prosched::Keyword::kPrint, {R"("hello")"});
  EXPECT_EQ(interp.ExecutePrint(stmt), "hello");
  auto buf = interp.FlushBuffer();
  ASSERT_EQ(buf.size(), 1u);
  EXPECT_EQ(buf[0], "hello");
}

// Empty quoted string prints and buffers an empty-string entry
TEST(InterpreterExecutePrint, EmptyStringLiteralReturnsEmpty) {
  prosched::Interpreter interp;
  auto stmt = makeStmt(prosched::Keyword::kPrint, {R"("")"});
  EXPECT_EQ(interp.ExecutePrint(stmt), "");
  auto buf = interp.FlushBuffer();
  ASSERT_EQ(buf.size(), 1u);
  EXPECT_EQ(buf[0], "");
}

// Undeclared variable resolves to "0" — the default value
TEST(InterpreterExecutePrint, UndeclaredVariablePrintsZero) {
  prosched::Interpreter interp;
  auto stmt = makeStmt(prosched::Keyword::kPrint, {"notdeclared"});
  EXPECT_EQ(interp.ExecutePrint(stmt), "0");
}

// "str=" + var concatenates a string literal with a variable's value
TEST(InterpreterExecutePrint, ConcatStringAndVariable) {
  prosched::Interpreter interp;
  interp.ExecuteDeclare(makeStmt(prosched::Keyword::kDeclare, {"x", "5"}));
  interp.FlushBuffer();

  auto stmt = makeStmt(prosched::Keyword::kPrint, {R"("val=" + x)"});
  EXPECT_EQ(interp.ExecutePrint(stmt), "val=5");
}

} // namespace InterpreterExecutePrint

// ─── ExecuteDeclare
// ───────────────────────────────────────────────────────────

namespace InterpreterExecuteDeclare {

// Returns the stored uint16 value on success
TEST(InterpreterExecuteDeclare, BasicDeclarationReturnsValue) {
  prosched::Interpreter interp;
  auto stmt = makeStmt(prosched::Keyword::kDeclare, {"x", "10"});
  auto result = interp.ExecuteDeclare(stmt);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result.value(), 10);
}

// Zero is a valid uint16 value
TEST(InterpreterExecuteDeclare, ZeroValueReturnsZero) {
  prosched::Interpreter interp;
  auto stmt = makeStmt(prosched::Keyword::kDeclare, {"x", "0"});
  auto result = interp.ExecuteDeclare(stmt);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result.value(), 0);
}

// 65535 is the uint16 max — must not overflow or truncate
TEST(InterpreterExecuteDeclare, MaxUint16ReturnsCorrectly) {
  prosched::Interpreter interp;
  auto stmt = makeStmt(prosched::Keyword::kDeclare, {"x", "65535"});
  auto result = interp.ExecuteDeclare(stmt);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result.value(), 65535);
}

// Single-arg DECLARE is malformed — returns nullopt without crashing
TEST(InterpreterExecuteDeclare, MissingSecondArgReturnsNullopt) {
  prosched::Interpreter interp;
  auto stmt = makeStmt(prosched::Keyword::kDeclare, {"x"});
  EXPECT_FALSE(interp.ExecuteDeclare(stmt).has_value());
}

// Declared variable is accessible in subsequent PRINT calls
TEST(InterpreterExecuteDeclare, VariableIsAvailableForSubsequentPrint) {
  prosched::Interpreter interp;
  interp.ExecuteDeclare(makeStmt(prosched::Keyword::kDeclare, {"myvar", "42"}));
  interp.ExecutePrint(makeStmt(prosched::Keyword::kPrint, {"myvar"}));
  auto buf = interp.FlushBuffer();
  ASSERT_EQ(buf.size(), 1u);
  EXPECT_EQ(buf[0], "42");
}

} // namespace InterpreterExecuteDeclare

// ─── ExecuteAdd
// ───────────────────────────────────────────────────────────────

namespace InterpreterExecuteAdd {

// Reads two declared vars, computes their sum, stores in a new var
TEST(InterpreterExecuteAdd, TwoVariablesAdded) {
  prosched::Interpreter interp;
  interp.ExecuteDeclare(makeStmt(prosched::Keyword::kDeclare, {"a", "3"}));
  interp.ExecuteDeclare(makeStmt(prosched::Keyword::kDeclare, {"b", "7"}));
  auto result =
      interp.ExecuteAdd(makeStmt(prosched::Keyword::kAdd, {"c", "a", "b"}));
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result.value(), 10);
}

// Numeric string literals are parsed inline — no prior DECLARE needed
TEST(InterpreterExecuteAdd, LiteralNumericOperands) {
  prosched::Interpreter interp;
  auto result =
      interp.ExecuteAdd(makeStmt(prosched::Keyword::kAdd, {"z", "5", "3"}));
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result.value(), 8);
}

// dest can be one of the source operands — overwrites correctly
TEST(InterpreterExecuteAdd, OverwritesExistingVariable) {
  prosched::Interpreter interp;
  interp.ExecuteDeclare(makeStmt(prosched::Keyword::kDeclare, {"x", "5"}));
  auto result =
      interp.ExecuteAdd(makeStmt(prosched::Keyword::kAdd, {"x", "x", "3"}));
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result.value(), 8);
}

// Fewer than 3 args is malformed — returns nullopt
TEST(InterpreterExecuteAdd, MissingArgsReturnsNullopt) {
  prosched::Interpreter interp;
  auto result = interp.ExecuteAdd(makeStmt(prosched::Keyword::kAdd, {"x", "5"}));
  EXPECT_FALSE(result.has_value());
}

} // namespace InterpreterExecuteAdd

// ─── ExecuteSubtract
// ──────────────────────────────────────────────────────────

namespace InterpreterExecuteSubtract {

// Subtracts two declared variables and stores the result in a new var
TEST(InterpreterExecuteSubtract, BasicSubtractionCorrect) {
  prosched::Interpreter interp;
  interp.ExecuteDeclare(makeStmt(prosched::Keyword::kDeclare, {"a", "10"}));
  interp.ExecuteDeclare(makeStmt(prosched::Keyword::kDeclare, {"b", "3"}));
  auto result = interp.ExecuteSubtract(
      makeStmt(prosched::Keyword::kSubtract, {"c", "a", "b"}));
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result.value(), 7);
}

// uint16 underflow wraps around — no exception thrown
TEST(InterpreterExecuteSubtract, Uint16UnderflowWrapsWithoutThrow) {
  prosched::Interpreter interp;
  interp.ExecuteDeclare(makeStmt(prosched::Keyword::kDeclare, {"a", "3"}));
  interp.ExecuteDeclare(makeStmt(prosched::Keyword::kDeclare, {"b", "10"}));
  auto result = interp.ExecuteSubtract(
      makeStmt(prosched::Keyword::kSubtract, {"c", "a", "b"}));
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result.value(), static_cast<uint16_t>(3 - 10));
}

// Fewer than 3 args is malformed — returns nullopt
TEST(InterpreterExecuteSubtract, MissingArgsReturnsNullopt) {
  prosched::Interpreter interp;
  auto result =
      interp.ExecuteSubtract(makeStmt(prosched::Keyword::kSubtract, {"x", "5"}));
  EXPECT_FALSE(result.has_value());
}

} // namespace InterpreterExecuteSubtract

// ─── ExecuteSleep
// ─────────────────────────────────────────────────────────────

namespace InterpreterExecuteSleep {

// SLEEP(0) is a no-op and must not block the thread
TEST(InterpreterExecuteSleep, ZeroTicksCompletesWithoutHang) {
  prosched::Interpreter interp;
  EXPECT_NO_THROW(
      interp.ExecuteSleep(makeStmt(prosched::Keyword::kSleep, {"0"})));
}

// SLEEP(1) completes in finite time
TEST(InterpreterExecuteSleep, OneTickCompletes) {
  prosched::Interpreter interp;
  EXPECT_NO_THROW(
      interp.ExecuteSleep(makeStmt(prosched::Keyword::kSleep, {"1"})));
}

} // namespace InterpreterExecuteSleep

// ─── executeFor
// ───────────────────────────────────────────────────────────────

namespace InterpreterExecuteFor {

// FOR body runs exactly N times, one PRINT per iteration
TEST(InterpreterExecuteFor, BasicThreeIterations) {
  prosched::Interpreter interp;
  interp.ExecuteString(R"(FOR([PRINT("x")], 3))");
  auto buf = interp.FlushBuffer();
  ASSERT_EQ(buf.size(), 3u);
  for (const auto &line : buf)
    EXPECT_EQ(line, "x");
}

// FOR with count 0 skips the body entirely
TEST(InterpreterExecuteFor, ZeroIterationsProducesNothing) {
  prosched::Interpreter interp;
  interp.ExecuteString(R"(FOR([PRINT("x")], 0))");
  EXPECT_TRUE(interp.FlushBuffer().empty());
}

// Inner loop (2) × outer loop (3) = 6 total PRINT calls
TEST(InterpreterExecuteFor, NestedForMultipliesIterationCount) {
  prosched::Interpreter interp;
  interp.ExecuteString(R"(FOR([FOR([PRINT("n")], 2)], 3))");
  auto buf = interp.FlushBuffer();
  EXPECT_EQ(buf.size(), 6u);
}

} // namespace InterpreterExecuteFor

// ─── ExecuteDebug
// ─────────────────────────────────────────────────────────────

namespace InterpreterExecuteDebug {

// No variables declared — buffer contains the "[No variables declared]" message
TEST(InterpreterExecuteDebug, EmptyMemoryShowsNoVarsMessage) {
  prosched::Interpreter interp;
  interp.ExecuteDebug();
  auto buf = interp.FlushBuffer();
  bool found = false;
  for (const auto &line : buf)
    if (line.find("[No variables declared]") != std::string::npos)
      found = true;
  EXPECT_TRUE(found);
}

// Declared variable name appears somewhere in the debug output lines
TEST(InterpreterExecuteDebug, DeclaredVariableAppearsInDump) {
  prosched::Interpreter interp;
  interp.ExecuteDeclare(makeStmt(prosched::Keyword::kDeclare, {"myvar", "99"}));
  interp.FlushBuffer();
  interp.ExecuteDebug();
  auto buf = interp.FlushBuffer();
  bool found = false;
  for (const auto &line : buf)
    if (line.find("myvar") != std::string::npos)
      found = true;
  EXPECT_TRUE(found);
}

// Return value is the live memory map containing all declared variables
TEST(InterpreterExecuteDebug, ReturnsMemoryMapWithAllDeclaredVars) {
  prosched::Interpreter interp;
  interp.ExecuteDeclare(makeStmt(prosched::Keyword::kDeclare, {"a", "1"}));
  interp.ExecuteDeclare(makeStmt(prosched::Keyword::kDeclare, {"b", "2"}));
  auto mem = interp.ExecuteDebug();
  EXPECT_EQ(mem.at("a"), 1);
  EXPECT_EQ(mem.at("b"), 2);
}

} // namespace InterpreterExecuteDebug

// ─── parse ─────────────────────────────────────────────────────────────────

namespace InterpreterParse {

// Empty string produces no AST nodes
TEST(InterpreterParse, EmptyStringReturnsEmptyVector) {
  prosched::Interpreter interp;
  auto stmts = interp.Parse("");
  EXPECT_TRUE(stmts.empty());
}

// Single valid statement produces one node with the correct keyword
TEST(InterpreterParse, SingleStatementReturnsOneNodeWithCorrectKeyword) {
  prosched::Interpreter interp;
  auto stmts = interp.Parse(R"(PRINT("hi"))");
  ASSERT_EQ(stmts.size(), 1u);
  EXPECT_EQ(stmts[0].keyword, prosched::Keyword::kPrint);
}

// Multiple comma-separated statements produce the correct node count
TEST(InterpreterParse, MultipleStatementsReturnCorrectCount) {
  prosched::Interpreter interp;
  auto stmts = interp.Parse(R"(PRINT("a"), DECLARE(x, 1), ADD(y, x, x))");
  EXPECT_EQ(stmts.size(), 3u);
  EXPECT_EQ(stmts[0].keyword, prosched::Keyword::kPrint);
  EXPECT_EQ(stmts[1].keyword, prosched::Keyword::kDeclare);
  EXPECT_EQ(stmts[2].keyword, prosched::Keyword::kAdd);
}

// FOR node stores its body in the nested field, not flattened into args
TEST(InterpreterParse, ForStatementPopulatesNestedField) {
  prosched::Interpreter interp;
  auto stmts = interp.Parse(R"(FOR([PRINT("x")], 2))");
  ASSERT_EQ(stmts.size(), 1u);
  ASSERT_EQ(stmts[0].keyword, prosched::Keyword::kFor);
  EXPECT_FALSE(stmts[0].nested.empty());
}

// Unknown keyword is parsed without crashing and tagged UNKNOWN
TEST(InterpreterParse, UnknownKeywordTaggedAsUnknown) {
  prosched::Interpreter interp;
  auto stmts = interp.Parse("BADCMD(x)");
  ASSERT_EQ(stmts.size(), 1u);
  EXPECT_EQ(stmts[0].keyword, prosched::Keyword::kUnknown);
}

// The top-level splitter respects bracket depth: commas inside a FOR body are
// NOT treated as statement separators. A FOR (whose body has its own commas)
// followed by a PRINT must yield exactly 2 top-level statements.
TEST(InterpreterParse, CommasInsideForBodyDoNotSplitTopLevel) {
  prosched::Interpreter interp;
  auto stmts =
      interp.Parse(R"(FOR([PRINT("a"), PRINT("b")], 2), PRINT("after"))");
  ASSERT_EQ(stmts.size(), 2u);
  EXPECT_EQ(stmts[0].keyword, prosched::Keyword::kFor);
  EXPECT_EQ(stmts[1].keyword, prosched::Keyword::kPrint);
  // The FOR's body keeps both of its inner statements
  EXPECT_EQ(stmts[0].nested.size(), 2u);
}

// Deeply nested FOR([FOR([...])]) parses into a tree of the right shape —
// the inner FOR is preserved as a nested child, not flattened or mis-split.
TEST(InterpreterParse, NestedForParsesIntoNestedTree) {
  prosched::Interpreter interp;
  auto stmts = interp.Parse(R"(FOR([FOR([PRINT("x")], 2)], 3))");
  ASSERT_EQ(stmts.size(), 1u);
  ASSERT_EQ(stmts[0].keyword, prosched::Keyword::kFor);
  ASSERT_EQ(stmts[0].nested.size(), 1u);
  ASSERT_EQ(stmts[0].nested[0].keyword, prosched::Keyword::kFor);
  ASSERT_EQ(stmts[0].nested[0].nested.size(), 1u);
  EXPECT_EQ(stmts[0].nested[0].nested[0].keyword, prosched::Keyword::kPrint);
}

} // namespace InterpreterParse

// ─── ExecuteStatements ─────────────────────────────────────────────────────

namespace InterpreterExecuteStatements {

// ExecuteStatements on a pre-parsed AST produces the same output as
// ExecuteString
TEST(InterpreterExecuteStatements, MatchesExecuteStringOutput) {
  prosched::Interpreter interp_a;
  interp_a.ExecuteString(R"(DECLARE(x, 5), PRINT(x))");
  auto expected = interp_a.FlushBuffer();

  prosched::Interpreter interp_b;
  auto stmts = interp_b.Parse(R"(DECLARE(x, 5), PRINT(x))");
  interp_b.ExecuteStatements(stmts);
  auto actual = interp_b.FlushBuffer();

  EXPECT_EQ(actual, expected);
}

// ExecuteStatements works when called with a manually constructed statement
// list
TEST(InterpreterExecuteStatements, WorksWithPreBuiltStatementList) {
  prosched::Interpreter interp;
  std::vector<prosched::Statement> stmts = {
      makeStmt(prosched::Keyword::kDeclare, {"n", "7"}),
      makeStmt(prosched::Keyword::kPrint, {"n"})};
  interp.ExecuteStatements(stmts);
  auto buf = interp.FlushBuffer();
  ASSERT_EQ(buf.size(), 1u);
  EXPECT_EQ(buf[0], "7");
}

// An UNKNOWN statement pushes a "[!]" warning to the buffer and does not throw
TEST(InterpreterExecuteStatements, UnknownStatementPushesWarning) {
  prosched::Interpreter interp;
  std::vector<prosched::Statement> stmts = {
      makeStmt(prosched::Keyword::kUnknown)};
  EXPECT_NO_THROW(interp.ExecuteStatements(stmts));
  auto buf = interp.FlushBuffer();
  ASSERT_FALSE(buf.empty());
  EXPECT_NE(buf[0].find("[!]"), std::string::npos);
}

} // namespace InterpreterExecuteStatements

// ─── executeFor (additional) ────────────────────────────────────────────────

namespace InterpreterExecuteFor {

// Outer(3) × middle(2) × inner(4) = 24 total PRINT calls — tests max nesting
// depth
TEST(InterpreterExecuteFor, TripleNestedForMultipliesIterationCount) {
  prosched::Interpreter interp;
  interp.ExecuteString(R"(FOR([FOR([FOR([PRINT("x")], 4)], 2)], 3))");
  auto buf = interp.FlushBuffer();
  EXPECT_EQ(buf.size(), 24u);
}

} // namespace InterpreterExecuteFor

// ─── ExecuteSleep (additional) ─────────────────────────────────────────────

namespace InterpreterExecuteSleep {

// SLEEP(0) resolves to 0 ms and finishes in negligible time
TEST(InterpreterExecuteSleep, ZeroTicksTakesNegligibleTime) {
  prosched::Interpreter interp;
  auto start = std::chrono::steady_clock::now();
  interp.ExecuteSleep(makeStmt(prosched::Keyword::kSleep, {"0"}));
  auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::steady_clock::now() - start)
                        .count();
  EXPECT_LT(elapsed_ms, 5);
}

// SLEEP(1) resolves to 10 ms (1 tick × 10 ms per tick); measurably longer than
// SLEEP(0)
TEST(InterpreterExecuteSleep, OneTickTakesAtLeastOneTick) {
  prosched::Interpreter interp;
  auto start = std::chrono::steady_clock::now();
  interp.ExecuteSleep(makeStmt(prosched::Keyword::kSleep, {"1"}));
  auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::steady_clock::now() - start)
                        .count();
  EXPECT_GE(elapsed_ms,
            8); // 10 ms nominal; 8 ms lower bound absorbs OS scheduling jitter
}

} // namespace InterpreterExecuteSleep

// ─── GetRandomStatement ────────────────────────────────────────────────────

namespace InterpreterGetRandomStatement {

// Over many calls the result is never UNKNOWN or DBG — both are not spec
// language keywords
TEST(InterpreterGetRandomStatement, AlwaysReturnsValidKeyword) {
  for (int i = 0; i < 100; i++) {
    auto stmt = prosched::GetRandomStatement("proc", 0);
    EXPECT_NE(stmt.keyword, prosched::Keyword::kUnknown)
        << "Got UNKNOWN on iteration " << i;
    EXPECT_NE(stmt.keyword, prosched::Keyword::kDebug)
        << "Got DBG on iteration " << i;
  }
}

// When maxDepth >= 3, FOR must never be selected
TEST(InterpreterGetRandomStatement, NeverReturnsForAtMaxDepth) {
  for (int i = 0; i < 100; i++) {
    auto stmt = prosched::GetRandomStatement("proc", 3);
    EXPECT_NE(stmt.keyword, prosched::Keyword::kFor)
        << "Got FOR at maxDepth=3 on iteration " << i;
  }
}

// Each keyword type's statement carries arguments in the correct shape for its
// keyword
TEST(InterpreterGetRandomStatement, ArgShapeMatchesKeyword) {
  for (int i = 0; i < 200; i++) {
    auto stmt = prosched::GetRandomStatement("proc", 0);
    switch (stmt.keyword) {
    case prosched::Keyword::kPrint:
      ASSERT_EQ(stmt.args.size(), 1u) << "PRINT must have 1 arg";
      EXPECT_EQ(stmt.args[0].front(), '"')
          << "PRINT arg must be a quoted string";
      EXPECT_EQ(stmt.args[0].back(), '"')
          << "PRINT arg must be a quoted string";
      break;
    case prosched::Keyword::kDeclare:
      EXPECT_EQ(stmt.args.size(), 2u) << "DECLARE must have 2 args";
      break;
    case prosched::Keyword::kAdd:
    case prosched::Keyword::kSubtract:
      EXPECT_EQ(stmt.args.size(), 3u) << "ADD/SUBTRACT must have 3 args";
      break;
    case prosched::Keyword::kSleep:
      ASSERT_EQ(stmt.args.size(), 1u) << "SLEEP must have 1 arg";
      EXPECT_NO_THROW(std::stoul(stmt.args[0])) << "SLEEP arg must be numeric";
      break;
    case prosched::Keyword::kFor:
      ASSERT_GE(stmt.args.size(), 2u) << "FOR must have at least 2 args";
      EXPECT_FALSE(stmt.nested.empty()) << "FOR must have nested statements";
      break;
    default:
      break;
    }
  }
}

} // namespace InterpreterGetRandomStatement
