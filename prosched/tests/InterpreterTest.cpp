#include "commands/Interpreter.hpp"
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

// ─── parse ─────────────────────────────────────────────────────────────────

namespace InterpreterParse {

  // Empty string produces no AST nodes
  TEST(InterpreterParse, EmptyStringReturnsEmptyVector) {
    prosched::Interpreter interp;
    auto stmts = interp.parse("");
    EXPECT_TRUE(stmts.empty());
  }

  // Single valid statement produces one node with the correct keyword
  TEST(InterpreterParse, SingleStatementReturnsOneNodeWithCorrectKeyword) {
    prosched::Interpreter interp;
    auto stmts = interp.parse(R"(PRINT("hi"))");
    ASSERT_EQ(stmts.size(), 1u);
    EXPECT_EQ(stmts[0].keyword, prosched::Keyword::PRINT);
  }

  // Multiple comma-separated statements produce the correct node count
  TEST(InterpreterParse, MultipleStatementsReturnCorrectCount) {
    prosched::Interpreter interp;
    auto stmts = interp.parse(R"(PRINT("a"), DECLARE(x, 1), ADD(y, x, x))");
    EXPECT_EQ(stmts.size(), 3u);
    EXPECT_EQ(stmts[0].keyword, prosched::Keyword::PRINT);
    EXPECT_EQ(stmts[1].keyword, prosched::Keyword::DECLARE);
    EXPECT_EQ(stmts[2].keyword, prosched::Keyword::ADD);
  }

  // FOR node stores its body in the nested field, not flattened into args
  TEST(InterpreterParse, ForStatementPopulatesNestedField) {
    prosched::Interpreter interp;
    auto stmts = interp.parse(R"(FOR([PRINT("x")], 2))");
    ASSERT_EQ(stmts.size(), 1u);
    ASSERT_EQ(stmts[0].keyword, prosched::Keyword::FOR);
    EXPECT_FALSE(stmts[0].nested.empty());
  }

  // Unknown keyword is parsed without crashing and tagged UNKNOWN
  TEST(InterpreterParse, UnknownKeywordTaggedAsUnknown) {
    prosched::Interpreter interp;
    auto stmts = interp.parse("BADCMD(x)");
    ASSERT_EQ(stmts.size(), 1u);
    EXPECT_EQ(stmts[0].keyword, prosched::Keyword::UNKNOWN);
}

  // The top-level splitter respects bracket depth: commas inside a FOR body are
  // NOT treated as statement separators. A FOR (whose body has its own commas)
  // followed by a PRINT must yield exactly 2 top-level statements.
  TEST(InterpreterParse, CommasInsideForBodyDoNotSplitTopLevel) {
    prosched::Interpreter interp;
    auto stmts = interp.parse(R"(FOR([PRINT("a"), PRINT("b")], 2), PRINT("after"))");
    ASSERT_EQ(stmts.size(), 2u);
    EXPECT_EQ(stmts[0].keyword, prosched::Keyword::FOR);
    EXPECT_EQ(stmts[1].keyword, prosched::Keyword::PRINT);
    // The FOR's body keeps both of its inner statements
    EXPECT_EQ(stmts[0].nested.size(), 2u);
  }

  // Deeply nested FOR([FOR([...])]) parses into a tree of the right shape —
  // the inner FOR is preserved as a nested child, not flattened or mis-split.
  TEST(InterpreterParse, NestedForParsesIntoNestedTree) {
    prosched::Interpreter interp;
    auto stmts = interp.parse(R"(FOR([FOR([PRINT("x")], 2)], 3))");
    ASSERT_EQ(stmts.size(), 1u);
    ASSERT_EQ(stmts[0].keyword, prosched::Keyword::FOR);
    ASSERT_EQ(stmts[0].nested.size(), 1u);
    ASSERT_EQ(stmts[0].nested[0].keyword, prosched::Keyword::FOR);
    ASSERT_EQ(stmts[0].nested[0].nested.size(), 1u);
    EXPECT_EQ(stmts[0].nested[0].nested[0].keyword, prosched::Keyword::PRINT);
  }

} // namespace InterpreterParse

// ─── executeStatements ─────────────────────────────────────────────────────

namespace InterpreterExecuteStatements {

  // executeStatements on a pre-parsed AST produces the same output as executeString
  TEST(InterpreterExecuteStatements, MatchesExecuteStringOutput) {
    prosched::Interpreter interp_a;
    interp_a.executeString(R"(DECLARE(x, 5), PRINT(x))");
    auto expected = interp_a.flushBuffer();

    prosched::Interpreter interp_b;
    auto stmts = interp_b.parse(R"(DECLARE(x, 5), PRINT(x))");
    interp_b.executeStatements(stmts);
    auto actual = interp_b.flushBuffer();

    EXPECT_EQ(actual, expected);
  }

  // executeStatements works when called with a manually constructed statement list
  TEST(InterpreterExecuteStatements, WorksWithPreBuiltStatementList) {
    prosched::Interpreter interp;
    std::vector<prosched::Statement> stmts = {
        makeStmt(prosched::Keyword::DECLARE, {"n", "7"}),
        makeStmt(prosched::Keyword::PRINT, {"n"})};
    interp.executeStatements(stmts);
    auto buf = interp.flushBuffer();
    ASSERT_EQ(buf.size(), 1u);
    EXPECT_EQ(buf[0], "7");
  }

  // An UNKNOWN statement pushes a "[!]" warning to the buffer and does not throw
  TEST(InterpreterExecuteStatements, UnknownStatementPushesWarning) {
    prosched::Interpreter interp;
    std::vector<prosched::Statement> stmts = {makeStmt(prosched::Keyword::UNKNOWN)};
    EXPECT_NO_THROW(interp.executeStatements(stmts));
    auto buf = interp.flushBuffer();
    ASSERT_FALSE(buf.empty());
    EXPECT_NE(buf[0].find("[!]"), std::string::npos);
  }

} // namespace InterpreterExecuteStatements

// ─── executeFor (additional) ────────────────────────────────────────────────

namespace InterpreterExecuteFor {

  // Outer(3) × middle(2) × inner(4) = 24 total PRINT calls — tests max nesting depth
  TEST(InterpreterExecuteFor, TripleNestedForMultipliesIterationCount) {
    prosched::Interpreter interp;
    interp.executeString(R"(FOR([FOR([FOR([PRINT("x")], 4)], 2)], 3))");
    auto buf = interp.flushBuffer();
    EXPECT_EQ(buf.size(), 24u);
  }

  } // namespace InterpreterExecuteFor

  // ─── executeSleep (additional) ─────────────────────────────────────────────

  namespace InterpreterExecuteSleep {

  // SLEEP(0) resolves to 0 ms and finishes in negligible time
  TEST(InterpreterExecuteSleep, ZeroTicksTakesNegligibleTime) {
    prosched::Interpreter interp;
    auto start = std::chrono::steady_clock::now();
    interp.executeSleep(makeStmt(prosched::Keyword::SLEEP, {"0"}));
    auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                          std::chrono::steady_clock::now() - start)
                          .count();
    EXPECT_LT(elapsed_ms, 5);
  }

  // SLEEP(1) resolves to 10 ms (1 tick × 10 ms per tick); measurably longer than SLEEP(0)
  TEST(InterpreterExecuteSleep, OneTickTakesAtLeastOneTick) {
    prosched::Interpreter interp;
    auto start = std::chrono::steady_clock::now();
    interp.executeSleep(makeStmt(prosched::Keyword::SLEEP, {"1"}));
    auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                          std::chrono::steady_clock::now() - start)
                          .count();
    EXPECT_GE(elapsed_ms, 8); // 10 ms nominal; 8 ms lower bound absorbs OS scheduling jitter
  }

} // namespace InterpreterExecuteSleep

// ─── GetRandomStatement ────────────────────────────────────────────────────

namespace InterpreterGetRandomStatement {

  // Over many calls the result is never UNKNOWN or DBG — both are not spec language keywords
  TEST(InterpreterGetRandomStatement, AlwaysReturnsValidKeyword) {
    for (int i = 0; i < 100; i++) {
      auto stmt = prosched::GetRandomStatement("proc", 0);
      EXPECT_NE(stmt.keyword, prosched::Keyword::UNKNOWN)
          << "Got UNKNOWN on iteration " << i;
      EXPECT_NE(stmt.keyword, prosched::Keyword::DBG)
          << "Got DBG on iteration " << i;
    }
  }

  // When maxDepth >= 3, FOR must never be selected
  TEST(InterpreterGetRandomStatement, NeverReturnsForAtMaxDepth) {
    for (int i = 0; i < 100; i++) {
      auto stmt = prosched::GetRandomStatement("proc", 3);
      EXPECT_NE(stmt.keyword, prosched::Keyword::FOR)
          << "Got FOR at maxDepth=3 on iteration " << i;
    }
  }

  // Each keyword type's statement carries arguments in the correct shape for its keyword
  TEST(InterpreterGetRandomStatement, ArgShapeMatchesKeyword) {
    for (int i = 0; i < 200; i++) {
      auto stmt = prosched::GetRandomStatement("proc", 0);
      switch (stmt.keyword) {
      case prosched::Keyword::PRINT:
        ASSERT_EQ(stmt.args.size(), 1u) << "PRINT must have 1 arg";
        EXPECT_EQ(stmt.args[0].front(), '"') << "PRINT arg must be a quoted string";
        EXPECT_EQ(stmt.args[0].back(), '"') << "PRINT arg must be a quoted string";
        break;
      case prosched::Keyword::DECLARE:
        EXPECT_EQ(stmt.args.size(), 2u) << "DECLARE must have 2 args";
        break;
      case prosched::Keyword::ADD:
      case prosched::Keyword::SUBTRACT:
        EXPECT_EQ(stmt.args.size(), 3u) << "ADD/SUBTRACT must have 3 args";
        break;
      case prosched::Keyword::SLEEP:
        ASSERT_EQ(stmt.args.size(), 1u) << "SLEEP must have 1 arg";
        EXPECT_NO_THROW(std::stoul(stmt.args[0])) << "SLEEP arg must be numeric";
        break;
      case prosched::Keyword::FOR:
        ASSERT_GE(stmt.args.size(), 2u) << "FOR must have at least 2 args";
        EXPECT_FALSE(stmt.nested.empty()) << "FOR must have nested statements";
        break;
      default:
        break;
      }
    }
  }

} // namespace InterpreterGetRandomStatement
