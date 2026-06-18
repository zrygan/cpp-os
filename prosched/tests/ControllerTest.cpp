#include <gtest/gtest.h>
#include "controller/Controller.h"


namespace ControllerIdentifyCommand {

    TEST(ControllerIdentifyCommand, ExitCommand) {
        Controller c;
        EXPECT_EQ(c.IdentifyCommand({"exit"}), CLI_COMMAND::CLI_EXIT);
    }

    TEST(ControllerIdentifyCommand, SchedulerStart) {
        Controller c;
        EXPECT_EQ(c.IdentifyCommand({"scheduler-start"}), CLI_COMMAND::CLI_SCHEDULER_START);
    }

    TEST(ControllerIdentifyCommand, SchedulerStop) {
        Controller c;
        EXPECT_EQ(c.IdentifyCommand({"scheduler-stop"}), CLI_COMMAND::CLI_SCHEDULER_STOP);
    }

    TEST(ControllerIdentifyCommand, ReportUtil) {
        Controller c;
        EXPECT_EQ(c.IdentifyCommand({"report-util"}), CLI_COMMAND::CLI_REPORT_UTIL);
    }

    TEST(ControllerIdentifyCommand, ScreenLs) {
        Controller c;
        EXPECT_EQ(c.IdentifyCommand({"screen", "-ls"}), CLI_COMMAND::CLI_SCREEN_LS);
    }

    TEST(ControllerIdentifyCommand, ScreenS) {
        Controller c;
        EXPECT_EQ(c.IdentifyCommand({"screen", "-s"}), CLI_COMMAND::CLI_SCREEN_S);
    }

    TEST(ControllerIdentifyCommand, ScreenWithNoSubcommandIsUnknown) {
        Controller c;
        EXPECT_EQ(c.IdentifyCommand({"screen"}), CLI_COMMAND::UNKNOWN);
    }

    TEST(ControllerIdentifyCommand, ScreenWithBadSubcommandIsUnknown) {
        Controller c;
        EXPECT_EQ(c.IdentifyCommand({"screen", "-x"}), CLI_COMMAND::UNKNOWN);
    }

    TEST(ControllerIdentifyCommand, UnrecognizedCommandIsUnknown) {
        Controller c;
        EXPECT_EQ(c.IdentifyCommand({"garbage"}), CLI_COMMAND::UNKNOWN);
    }

} // namespace ControllerIdentifyCommand


namespace ControllerGetParsedInput {

    TEST(ControllerGetParsedInput, EmptyStringReturnsUnknown) {
        Controller c;
        EXPECT_EQ(c.GetParsedInput("").cliCommand, CLI_COMMAND::UNKNOWN);
    }

    TEST(ControllerGetParsedInput, ExitCommand) {
        Controller c;
        EXPECT_EQ(c.GetParsedInput("exit").cliCommand, CLI_COMMAND::CLI_EXIT);
    }

    TEST(ControllerGetParsedInput, SchedulerStart) {
        Controller c;
        EXPECT_EQ(c.GetParsedInput("scheduler-start").cliCommand, CLI_COMMAND::CLI_SCHEDULER_START);
    }

    TEST(ControllerGetParsedInput, SchedulerStop) {
        Controller c;
        EXPECT_EQ(c.GetParsedInput("scheduler-stop").cliCommand, CLI_COMMAND::CLI_SCHEDULER_STOP);
    }

    TEST(ControllerGetParsedInput, ScreenLs) {
        Controller c;
        EXPECT_EQ(c.GetParsedInput("screen -ls").cliCommand, CLI_COMMAND::CLI_SCREEN_LS);
    }

    TEST(ControllerGetParsedInput, ScreenSExtractsProcessName) {
        Controller c;
        Command cmd = c.GetParsedInput("screen -s myproc");
        EXPECT_EQ(cmd.cliCommand, CLI_COMMAND::CLI_SCREEN_S);
        EXPECT_EQ(cmd.processName, "myproc");
    }

    TEST(ControllerGetParsedInput, GarbageInputReturnsUnknown) {
        Controller c;
        EXPECT_EQ(c.GetParsedInput("garbage").cliCommand, CLI_COMMAND::UNKNOWN);
    }

} // namespace ControllerGetParsedInput

namespace ControllerHandlePreInit {

    TEST(ControllerHandlePreInit, NonInitializeInputReturnsFalse) {
        Controller c;
        // "garbage" never reads isInitialized — safe even before initialize() is called
        EXPECT_FALSE(c.HandlePreInit("garbage"));
    }

    TEST(ControllerHandlePreInit, EmptyInputReturnsFalse) {
        Controller c;
        EXPECT_FALSE(c.HandlePreInit(""));
    }

    TEST(ControllerHandlePreInit, InitializeInputReturnsTrue) {
        Controller c;
        // Always returns true for "initialize" regardless of whether config was found
        EXPECT_TRUE(c.HandlePreInit("initialize"));
    }

} // namespace ControllerHandlePreInit


// ─── initialize ───────────────────────────────────────────────────────────────
// Reads "prosched/config.txt" relative to CWD. CMakeLists.txt copies the file to
// ${CMAKE_CURRENT_BINARY_DIR}/prosched/config.txt so tests find it at runtime.
//
// Expected values from prosched/config.txt:
//   num-cpu 4  |  scheduler fcfs  |  quantum-cycles 5  |  batch-process-freq 1
//   min-ins 1000  |  max-ins 2000  |  delay-per-exec 0

namespace ControllerInitialize {

    TEST(ControllerInitialize, ReturnsCorrectSchedulerType) {
        Controller c;
        AlgoContext ctx = c.initialize();
        EXPECT_EQ(ctx.schedulerType, SchedulerType::FCFS);
    }

    TEST(ControllerInitialize, ReturnsCorrectNumCpu) {
        Controller c;
        AlgoContext ctx = c.initialize();
        EXPECT_EQ(ctx.numCpu, 4);
    }

    TEST(ControllerInitialize, ReturnsCorrectBatchProcessFreq) {
        Controller c;
        AlgoContext ctx = c.initialize();
        EXPECT_EQ(ctx.batchProcessFreq, 1);
    }

    TEST(ControllerInitialize, ReturnsCorrectInstructionBounds) {
        Controller c;
        AlgoContext ctx = c.initialize();
        EXPECT_EQ(ctx.minIns, 1000);
        EXPECT_EQ(ctx.maxIns, 2000);
    }

    TEST(ControllerInitialize, ReturnsCorrectQuantumCycles) {
        Controller c;
        AlgoContext ctx = c.initialize();
        EXPECT_EQ(ctx.quantumCycles, 5);
    }

    TEST(ControllerInitialize, ReturnsCorrectDelayPerExec) {
        Controller c;
        AlgoContext ctx = c.initialize();
        EXPECT_EQ(ctx.delayPerExec, 0);
    }

    TEST(ControllerInitialize, SchedulerTypeIsNotUnknownOnSuccess) {
        Controller c;
        AlgoContext ctx = c.initialize();
        // UNKNOWN means fromFile() failed (config.txt not found) or scheduler string unrecognized
        EXPECT_NE(ctx.schedulerType, SchedulerType::UNKNOWN);
    }

    TEST(ControllerInitialize, CalledTwiceDoesNotCrash) {
        Controller c;
        EXPECT_NO_THROW({
            c.initialize();
            c.initialize();
        });
    }

} // namespace ControllerInitialize
