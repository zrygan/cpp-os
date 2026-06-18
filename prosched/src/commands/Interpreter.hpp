#pragma once
#include <chrono>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

namespace prosched {

/** @enum Keyword
    @brief All the valid instruction types for the OS emulator.

    Defines the set of keywords that the interpreter recognizes and can execute.
*/
enum class Keyword {
  PRINT,     /*!< Output text or values to the screen buffer */
  DECLARE,   /*!< Create a variable and initialize it */
  ADD,       /*!< Add two operands and store result in a variable */
  SUBTRACT,  /*!< Subtract two operands and store result in a variable */
  SLEEP,     /*!< Pause execution for a number of ticks (each tick = 10ms) */
  FOR,       /*!< Loop: execute instructions multiple times */
  DBG,       /*!< Debug dump: print all declared variables 
                  Not required from the specs. This is my own helper
                  thingy. Used in after DECLARE, ADD, SUBTRACT */
  UNKNOWN    /*!< Unrecognized instruction (error case) */
};

/** @struct Statement
    @brief Represents a parsed instruction in the abstract syntax tree (AST).

    After parsing, a program becomes a tree of Statements where FOR loops
    can contain nested Statements. This tree structure makes it easy to traverse
    and execute the program recursively.
*/
struct Statement {
  Keyword keyword;                 /*!< What kind of instruction this is */
  std::vector<std::string> args;   /*!< Raw parsed arguments (not evaluated yet) */
  std::vector<Statement> nested;   /*!< Child statements (only set for FOR loops) */
};

/** @class Interpreter
    @brief A simple string-based language interpreter for the prosched runtime.

    Marq is a mini-programming language that executes from a single string.
    It supports variables, arithmetic operations, loops, and simple I/O.

    The interpreter operates in two distinct phases:
    - **Parsing**: converts the input string into an abstract syntax tree (AST)
    - **Execution**: walks the AST and executes instructions recursively
*/
class Interpreter {
private:
  std::unordered_map<std::string, uint16_t> memory;    /*!< Variable storage (all 16-bit unsigned) */
  std::vector<std::string> screen_buffer;              /*!< Output buffer (PRINT, DBG!, errors) */

  /** @brief All keyword strings (for reference, actual detection uses prefix matching) */
  static constexpr const char *KEYWORDS[] = {
      "PRINT",  "DECLARE", "ADD", "SUBTRACT", "SLEEP", "FOR", "DBG"};

  /** @brief Removes leading/trailing whitespace and newlines.
      @param s The string to trim
      @return Trimmed string, or empty string if the input is all whitespace
  */
  std::string trim(const std::string &s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    size_t end = s.find_last_not_of(" \t\r\n");
    return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
  }

  /** @brief Resolves an operand to a numeric value.

      Handles three cases:
      - Numeric literal: "42" → 42
      - Variable reference: "x" → memory[x] (auto-create with 0 if missing)
      - Empty string: "" → 0

      @param op The operand string to resolve (number, variable name, or empty)
      @return The resolved 16-bit unsigned integer value
  */
  uint16_t resolveOperand(const std::string &op) {
    std::string t = trim(op);
    if (t.empty())
      return 0;
    if (isdigit(t[0])) {
      return static_cast<uint16_t>(std::stoul(t));
    }
    if (memory.find(t) == memory.end())
      memory[t] = 0;
    return memory[t];
  }

  /** @brief Splits a single string of instructions to a list of
      strings. Each string is a "sub-instruction" in the whole string.

      Example:
      - Input: "FOO(x), FOR([PRINT(1), PRINT(2)], 3)"
      - Output: ["FOO(x)", "FOR([PRINT(1), PRINT(2)], 3)"]

      @param list The comma-separated instruction list to split
      @return Vector of trimmed instruction strings
  */
  std::vector<std::string> splitInstructions(const std::string &list) {
    std::vector<std::string> instrs;
    int depth = 0;
    std::string current;
    for (char c : list) {
      if (c == '[' || c == '(')
        depth++;
      else if (c == ']' || c == ')')
        depth--;
      else if (c == ',' && depth == 0) {
        if (!trim(current).empty())
          instrs.push_back(trim(current));
        current.clear();
        continue;
      }
      current += c;
    }
    if (!trim(current).empty())
      instrs.push_back(trim(current));
    return instrs;
  }

  /** @brief Figures out what kind of instruction this statement is.

      Uses prefix matching to determine the keyword type.
      For example, "PRINT(...)" will match the PRINT keyword.

      @param stmt The statement string to analyze
      @return The identified Keyword, or Keyword::UNKNOWN if not recognized
  */
  Keyword identifyKeyword(const std::string &stmt) {
    if (stmt.rfind("PRINT(", 0) == 0)
      return Keyword::PRINT;
    if (stmt.rfind("DECLARE(", 0) == 0)
      return Keyword::DECLARE;
    if (stmt.rfind("ADD(", 0) == 0)
      return Keyword::ADD;
    if (stmt.rfind("SUBTRACT(", 0) == 0)
      return Keyword::SUBTRACT;
    if (stmt.rfind("SLEEP(", 0) == 0)
      return Keyword::SLEEP;
    if (stmt.rfind("FOR([", 0) == 0)
      return Keyword::FOR;
    if (stmt == "DBG!")
      return Keyword::DBG;
    return Keyword::UNKNOWN;
  }

  /** @brief Extracts raw arguments from a statement string for the given keyword type.

      Parses the statement syntax to extract its arguments.
      Arguments remain as strings until execution (not evaluated yet).

      @param stmt The statement string to parse
      @param kw The keyword type identifying which statement format to expect
      @return Vector of argument strings (format varies by keyword)
  */
  std::vector<std::string> extractArgs(const std::string &stmt, Keyword kw) {
    std::vector<std::string> args;

    if (kw == Keyword::PRINT) {
      size_t end = stmt.find_last_of(')');
      if (end != std::string::npos) {
        args.push_back(trim(stmt.substr(6, end - 6)));
      }
    } else if (kw == Keyword::DECLARE) {
      size_t comma = stmt.find(',');
      size_t end = stmt.find(')');
      if (comma != std::string::npos && end != std::string::npos &&
          comma > 8) {
        args.push_back(trim(stmt.substr(8, comma - 8)));
        args.push_back(trim(stmt.substr(comma + 1, end - comma - 1)));
      }
    } else if (kw == Keyword::ADD || kw == Keyword::SUBTRACT) {
      size_t offset = (kw == Keyword::ADD) ? 4 : 9;
      size_t comma1 = stmt.find(',');
      size_t comma2 = stmt.find(',', comma1 + 1);
      size_t end = stmt.find(')');
      if (comma1 != std::string::npos && comma2 != std::string::npos &&
          end != std::string::npos) {
        args.push_back(trim(stmt.substr(offset, comma1 - offset)));
        args.push_back(trim(stmt.substr(comma1 + 1, comma2 - comma1 - 1)));
        args.push_back(trim(stmt.substr(comma2 + 1, end - comma2 - 1)));
      }
    } else if (kw == Keyword::SLEEP) {
      size_t end = stmt.find(')');
      if (end != std::string::npos) {
        args.push_back(trim(stmt.substr(6, end - 6)));
      }
    } else if (kw == Keyword::FOR) {
      int depth = 0;
      size_t closing_bracket = std::string::npos;
      for (size_t i = 4; i < stmt.size(); ++i) {
        if (stmt[i] == '[')
          depth++;
        else if (stmt[i] == ']') {
          depth--;
          if (depth == 0) {
            closing_bracket = i;
            break;
          }
        }
      }

      if (closing_bracket != std::string::npos) {
        size_t comma = stmt.find(',', closing_bracket);
        size_t end = stmt.rfind(')');
        if (comma != std::string::npos && end != std::string::npos) {
          args.push_back(stmt.substr(5, closing_bracket - 5));
          args.push_back(trim(stmt.substr(comma + 1, end - comma - 1)));
        }
      }
    }

    return args;
  }

  /** @brief Parse a single statement string into an AST node.

      Handles recursive parsing of FOR loop bodies, extracting arguments
      and recursively parsing nested instructions.

      @param stmt_str The statement string to parse
      @return A Statement AST node, ready for execution
  */
  // Statement parseStatement(const std::string &stmt_str);

  /** @brief Execute a single AST node.

      Dispatches to the appropriate execution handler based on the statement's keyword type.

      @param stmt The Statement AST node to execute
  */
  // void executeStatement(const Statement &stmt);

  /** @brief Parse a comma-separated block of instructions into a vector of AST nodes.

      The recursive entry point for parsing. Splits the input by commas
      (respecting bracket nesting) and parses each instruction.

      @param block_str The instruction block string to parse
      @return Vector of Statement AST nodes
  */
  std::vector<Statement> parseBlock(const std::string &block_str) {
    std::vector<Statement> stmts;
    auto instr_list = splitInstructions(block_str);
    for (const auto &instr : instr_list) {
      stmts.push_back(parseStatement(instr));
    }
    return stmts;
  }


  Statement parseStatement(const std::string &stmt_str) {
    std::string trimmed = trim(stmt_str);
    Statement stmt;
    stmt.keyword = identifyKeyword(trimmed);

    if (stmt.keyword == Keyword::UNKNOWN) {
      stmt.keyword = Keyword::UNKNOWN;
      return stmt;
    }

    stmt.args = extractArgs(trimmed, stmt.keyword);

    if (stmt.keyword == Keyword::FOR && stmt.args.size() >= 1) {
      stmt.nested = parseBlock(stmt.args[0]);
    }

    return stmt;
  }

  void executeStatement(const Statement &stmt) {
    try {
      switch (stmt.keyword) {
      case Keyword::PRINT:
        if (stmt.args.size() >= 1)
          executePrint(stmt);
        break;
      case Keyword::DECLARE:
        executeDeclare(stmt);
        break;
      case Keyword::ADD:
        executeAdd(stmt);
        break;
      case Keyword::SUBTRACT:
        executeSubtract(stmt);
        break;
      case Keyword::SLEEP:
        executeSleep(stmt);
        break;
      case Keyword::FOR:
        executeFor(stmt);
        break;
      case Keyword::DBG:
        executeDebug();
        break;
      case Keyword::UNKNOWN:
        screen_buffer.push_back("[!] Interpreter Warning: Unrecognized instruction");
        break;
      }
    } catch (const std::exception &e) {
      screen_buffer.push_back("[!] Interpreter Error: Failed to execute statement");
    }
  }

public:
  /** @brief Create a new interpreter instance with empty state.
  */
  Interpreter() = default;

  /** @brief Retrieve and clear the output buffer.

      Gets all output written by PRINT, DBG!, or error messages
      since the last flush, then clears the buffer.

      @return Vector of output strings
  */
  std::vector<std::string> flushBuffer() {
    auto copy = screen_buffer;
    screen_buffer.clear();
    return copy;
  }

  /** @brief Parse a program string into an abstract syntax tree (AST).

      Returns a vector of Statement nodes ready for execution.
      Performs no evaluation—just parsing and validation.

      @param program The program string to parse
      @return Vector of Statement AST nodes
  */
  std::vector<Statement> parse(const std::string &program) {
    return parseBlock(program);
  }

  /** @brief Execute a pre-parsed AST (vector of Statements).

      Useful if you want to parse once and execute multiple times.

      @param stmts Vector of Statement nodes to execute
  */
  void executeStatements(const std::vector<Statement> &stmts) {
    for (const auto &stmt : stmts) {
      executeStatement(stmt);
    }
  }

  /** @brief Parse and execute a program string in one call.

      Convenience method combining parsing and execution.
      Catches exceptions and adds error messages to the output buffer.

      @param program The program string to parse and execute
  */
  void executeString(std::string program) {
    program = trim(program);
    if (program.empty())
      return;

    try {
      std::vector<Statement>statements = parse(program);
      executeStatements(statements);
    } catch (const std::exception &e) {
      screen_buffer.push_back(
          "[!] Interpreter Error: Malformed syntax near -> " + program);
    }
  }

  /** @brief Execute PRINT: output text or numeric values.

      Handles string literals and variable substitution.
      Results are appended to the screen buffer.

      @param stmt The PRINT statement to execute

      @returns the value printed to the buffer

      @warning side effect on screen_buffer attribute
  */
  std::string executePrint(const Statement &stmt) {
    std::string arg = stmt.args[0];
    std::string output = "";

    size_t plus_idx = arg.find('+');
    if (plus_idx != std::string::npos) {
        std::string str_part = trim(arg.substr(0, plus_idx));
        std::string var_part = trim(arg.substr(plus_idx + 1));
        if (str_part.front() == '"' && str_part.back() == '"')
            output += str_part.substr(1, str_part.size() - 2);
        output += std::to_string(resolveOperand(var_part));
    } else if (arg.front() == '"' && arg.back() == '"') {
        output = arg.substr(1, arg.size() - 2);
    } else {
        output = std::to_string(resolveOperand(arg));
    }

    screen_buffer.push_back(output);  // side effect
    return output;
}

  /** @brief Execute DECLARE: create and initialize a variable.
   * 
   * @param stmt The DECLARE statement to execute, or none at failure
   * 
   * @return the value of the declaration
   * 
   * @warning side effect on memory attribute
  */
  std::optional<uint16_t> executeDeclare(const Statement &stmt) {
    if (stmt.args.size() < 2)
        return std::nullopt;
    memory[stmt.args[0]] = static_cast<uint16_t>(std::stoul(stmt.args[1]));
    return memory[stmt.args[0]];
}

  /** @brief Execute ADD: compute first_operand + second_operand, store in variable.
   * 
   * @param stmt The ADD statement to execute
   * 
   * @return the resulting value of the ADD, or none at failure
   * 
   * @warning side effect on memory attribute
  */
  std::optional<uint16_t> executeAdd(const Statement &stmt) {
    if (stmt.args.size() < 3)
        return std::nullopt;
    memory[stmt.args[0]] = resolveOperand(stmt.args[1]) + resolveOperand(stmt.args[2]);
    return memory[stmt.args[0]];
}

  /** @brief Execute SUBTRACT: compute first_operand - second_operand, store in variable.
   * 
   * @param stmt The SUBTRACT statement to execute
   * 
   * @return the resulting value of the SUBTRACT, or none at failure
   * 
   * @warning side effect on memory attribute
  */
  std::optional<uint16_t> executeSubtract(const Statement &stmt) {
    if (stmt.args.size() < 3)
        return std::nullopt;
    memory[stmt.args[0]] = resolveOperand(stmt.args[1]) - resolveOperand(stmt.args[2]);
    return memory[stmt.args[0]];
}

  /** @brief Execute SLEEP: pause for N ticks (1 tick = 10ms).
      @param stmt The SLEEP statement to execute
  */
  void executeSleep(const Statement &stmt) {
    if (stmt.args.size() >= 1) {
      uint8_t ticks = static_cast<uint8_t>(std::stoul(stmt.args[0]));
      std::this_thread::sleep_for(std::chrono::milliseconds(ticks * 10));
    }
  }

  /** @brief Execute FOR: run nested instructions repeatedly.

      The repeat count is resolved from the second argument.

      @param stmt The FOR statement to execute
  */
  void executeFor(const Statement &stmt) {
    if (stmt.args.size() >= 2) {
      uint16_t repeats = resolveOperand(stmt.args[1]);
      for (uint16_t i = 0; i < repeats; ++i) {
        for (const auto &nested : stmt.nested) {
          executeStatement(nested);
        }
      }
    }
  }

  /** @brief Execute DBG!: dump all variables and their values to the screen buffer.
   * 
   * @returns the memory of the class
  */
  std::unordered_map<std::string, uint16_t> executeDebug() {
    screen_buffer.push_back("====================================");
    if (memory.empty()) {
        screen_buffer.push_back("  [No variables declared]");
    } else {
        for (const auto &pair : memory) {
            screen_buffer.push_back("|" + pair.first + "|\t" + std::to_string(pair.second));
        }
    }
    screen_buffer.push_back("====================================");
    return memory;  
  }
};

} // namespace prosched