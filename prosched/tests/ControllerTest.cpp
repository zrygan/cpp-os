#include "controller/Controller.h"
#include <gtest/gtest.h>

namespace ControllerIdentifyCommand {

// "exit" token maps to CLI_EXIT
TEST(ControllerIdentifyCommand, ExitCommand) {
  Controller c;
  EXPECT_EQ(c.IdentifyCommand({"exit"}), CLI_COMMAND::CLI_EXIT);
}

// "scheduler-start" token maps to CLI_SCHEDULER_START
TEST(ControllerIdentifyCommand, SchedulerStart) {
  Controller c;
  EXPECT_EQ(c.IdentifyCommand({"scheduler-start"}),
            CLI_COMMAND::CLI_SCHEDULER_START);
}

// "scheduler-stop" token maps to CLI_SCHEDULER_STOP
TEST(ControllerIdentifyCommand, SchedulerStop) {
  Controller c;
  EXPECT_EQ(c.IdentifyCommand({"scheduler-stop"}),
            CLI_COMMAND::CLI_SCHEDULER_STOP);
}

// "report-util" token maps to CLI_REPORT_UTIL
TEST(ControllerIdentifyCommand, ReportUtil) {
  Controller c;
  EXPECT_EQ(c.IdentifyCommand({"report-util"}), CLI_COMMAND::CLI_REPORT_UTIL);
}

// {"screen", "-ls"} pair maps to CLI_SCREEN_LS
TEST(ControllerIdentifyCommand, ScreenLs) {
  Controller c;
  EXPECT_EQ(c.IdentifyCommand({"screen", "-ls"}), CLI_COMMAND::CLI_SCREEN_LS);
}

// {"screen", "-s"} pair maps to CLI_SCREEN_S
TEST(ControllerIdentifyCommand, ScreenS) {
  Controller c;
  EXPECT_EQ(c.IdentifyCommand({"screen", "-s"}), CLI_COMMAND::CLI_SCREEN_S);
}

// "screen" without a flag has no valid subcommand
TEST(ControllerIdentifyCommand, ScreenWithNoSubcommandIsUnknown) {
  Controller c;
  EXPECT_EQ(c.IdentifyCommand({"screen"}), CLI_COMMAND::UNKNOWN);
}

// Unrecognized flag after "screen" is treated as unknown
TEST(ControllerIdentifyCommand, ScreenWithBadSubcommandIsUnknown) {
  Controller c;
  EXPECT_EQ(c.IdentifyCommand({"screen", "-x"}), CLI_COMMAND::UNKNOWN);
}

// Completely unrecognized token maps to UNKNOWN
TEST(ControllerIdentifyCommand, UnrecognizedCommandIsUnknown) {
  Controller c;
  EXPECT_EQ(c.IdentifyCommand({"garbage"}), CLI_COMMAND::UNKNOWN);
}

} // namespace ControllerIdentifyCommand

namespace ControllerGetParsedInput {

// Empty input tokenizes to nothing — produces UNKNOWN
TEST(ControllerGetParsedInput, EmptyStringReturnsUnknown) {
  Controller c;
  EXPECT_EQ(c.GetParsedInput("").cliCommand, CLI_COMMAND::UNKNOWN);
}

// "exit" string tokenizes to CLI_EXIT
TEST(ControllerGetParsedInput, ExitCommand) {
  Controller c;
  EXPECT_EQ(c.GetParsedInput("exit").cliCommand, CLI_COMMAND::CLI_EXIT);
}

// "scheduler-start" string tokenizes to CLI_SCHEDULER_START
TEST(ControllerGetParsedInput, SchedulerStart) {
  Controller c;
  EXPECT_EQ(c.GetParsedInput("scheduler-start").cliCommand,
            CLI_COMMAND::CLI_SCHEDULER_START);
}

// "scheduler-stop" string tokenizes to CLI_SCHEDULER_STOP
TEST(ControllerGetParsedInput, SchedulerStop) {
  Controller c;
  EXPECT_EQ(c.GetParsedInput("scheduler-stop").cliCommand,
            CLI_COMMAND::CLI_SCHEDULER_STOP);
}

// "screen -ls" string tokenizes to CLI_SCREEN_LS
TEST(ControllerGetParsedInput, ScreenLs) {
  Controller c;
  EXPECT_EQ(c.GetParsedInput("screen -ls").cliCommand,
            CLI_COMMAND::CLI_SCREEN_LS);
}

// Third token "myproc" is stored in Command.processName
TEST(ControllerGetParsedInput, ScreenSExtractsProcessName) {
  Controller c;
  Command cmd = c.GetParsedInput("screen -s myproc");
  EXPECT_EQ(cmd.cliCommand, CLI_COMMAND::CLI_SCREEN_S);
  EXPECT_EQ(cmd.processName, "myproc");
}

// Unrecognized string tokenizes to UNKNOWN
TEST(ControllerGetParsedInput, GarbageInputReturnsUnknown) {
  Controller c;
  EXPECT_EQ(c.GetParsedInput("garbage").cliCommand, CLI_COMMAND::UNKNOWN);
}

} // namespace ControllerGetParsedInput

namespace ControllerHandlePreInit {

// "garbage" never reads isInitialized — safe even before initialize() is called
TEST(ControllerHandlePreInit, NonInitializeInputReturnsFalse) {
  Controller c;
  EXPECT_FALSE(c.HandlePreInit("garbage"));
}

// Empty string is not "initialize" — returns false
TEST(ControllerHandlePreInit, EmptyInputReturnsFalse) {
  Controller c;
  EXPECT_FALSE(c.HandlePreInit(""));
}

// Always returns true for "initialize" regardless of whether config was found
TEST(ControllerHandlePreInit, InitializeInputReturnsTrue) {
  Controller c;
  EXPECT_TRUE(c.HandlePreInit("initialize"));
}

} // namespace ControllerHandlePreInit

// Expected values from prosched/config.txt:
//   num-cpu 4  |  scheduler fcfs  |  quantum-cycles 5  |  batch-process-freq 1
//   min-ins 1000  |  max-ins 2000  |  delay-per-exec 0

namespace ControllerInitialize {

// "fcfs" in config.txt maps to SchedulerType::FCFS enum
TEST(ControllerInitialize, ReturnsCorrectSchedulerType) {
  Controller c;
  AlgoContext ctx = c.initialize();
  EXPECT_EQ(ctx.schedulerType, SchedulerType::FCFS);
}

// num-cpu 4 from config.txt is read into num_cpu
TEST(ControllerInitialize, ReturnsCorrectnum_cpu) {
  Controller c;
  AlgoContext ctx = c.initialize();
  EXPECT_EQ(ctx.num_cpu, 4);
}

// batch-process-freq 1 from config.txt is read into batch_process_frequency
TEST(ControllerInitialize, ReturnsCorrectbatch_process_frequency) {
  Controller c;
  AlgoContext ctx = c.initialize();
  EXPECT_EQ(ctx.batch_process_frequency, 1);
}

// min-ins and max-ins are read together as the instruction range
TEST(ControllerInitialize, ReturnsCorrectInstructionBounds) {
  Controller c;
  AlgoContext ctx = c.initialize();
  EXPECT_EQ(ctx.min_ins, 1000);
  EXPECT_EQ(ctx.max_ins, 2000);
}

// quantum-cycles 5 from config.txt is read into rr_quantum_cycles
TEST(ControllerInitialize, ReturnsCorrectrr_quantum_cycles) {
  Controller c;
  AlgoContext ctx = c.initialize();
  EXPECT_EQ(ctx.rr_quantum_cycles, 5);
}

// delay-per-exec 0 from config.txt is read into delay_per_execution
TEST(ControllerInitialize, ReturnsCorrectdelay_per_execution) {
  Controller c;
  AlgoContext ctx = c.initialize();
  EXPECT_EQ(ctx.delay_per_execution, 0);
}

// UNKNOWN means fromFile() failed (config.txt not found) or unrecognized
// scheduler string
TEST(ControllerInitialize, SchedulerTypeIsNotUnknownOnSuccess) {
  Controller c;
  AlgoContext ctx = c.initialize();
  EXPECT_NE(ctx.schedulerType, SchedulerType::UNKNOWN);
}

// initialize() is idempotent — no crash on repeated calls
TEST(ControllerInitialize, CalledTwiceDoesNotCrash) {
  Controller c;
  EXPECT_NO_THROW({
    c.initialize();
    c.initialize();
  });
}

} // namespace ControllerInitialize
