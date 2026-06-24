#include "Config.h"
#include "src/context.h"
#include <cstdio>
#include <fstream>
#include <gtest/gtest.h>
#include <sstream>

namespace ConfigMakeDefault {

// makeDefault always returns a valid pointer, never nullptr
TEST(ConfigMakeDefault, ReturnsNonNull) {
  ConfigStruct *cs = makeDefault();
  ASSERT_NE(cs, nullptr);
  delete cs;
}

// Default num_cpu is 4
TEST(ConfigMakeDefault, NumCpuIsDefault) {
  ConfigStruct *cs = makeDefault();
  EXPECT_EQ(cs->num_cpu, 4);
  delete cs;
}

// Default scheduler is "rr" — note: config.txt uses "fcfs" instead
TEST(ConfigMakeDefault, SchedulerIsRr) {
  ConfigStruct *cs = makeDefault();
  EXPECT_EQ(cs->scheduler, "rr");
  delete cs;
}

// Default quantum_cycles is 5
TEST(ConfigMakeDefault, QuantumCyclesIsDefault) {
  ConfigStruct *cs = makeDefault();
  EXPECT_EQ(cs->quantum_cycles, 5);
  delete cs;
}

// Default batch_process_freq is 1
TEST(ConfigMakeDefault, BatchProcessFreqIsDefault) {
  ConfigStruct *cs = makeDefault();
  EXPECT_EQ(cs->batch_process_freq, 1);
  delete cs;
}

// Default min_ins is 1000
TEST(ConfigMakeDefault, MinInsIsDefault) {
  ConfigStruct *cs = makeDefault();
  EXPECT_EQ(cs->min_ins, 1000);
  delete cs;
}

// Default max_ins is 2000
TEST(ConfigMakeDefault, MaxInsIsDefault) {
  ConfigStruct *cs = makeDefault();
  EXPECT_EQ(cs->max_ins, 2000);
  delete cs;
}

// Default delay_per_exec is 0
TEST(ConfigMakeDefault, DelayPerExecIsDefault) {
  ConfigStruct *cs = makeDefault();
  EXPECT_EQ(cs->delay_per_exec, 0);
  delete cs;
}

} // namespace ConfigMakeDefault

// Expected values from prosched/config.txt:
//   num-cpu 4  |  scheduler fcfs  |  quantum-cycles 5  |  batch-process-freq 1
//   min-ins 1000  |  max-ins 2000  |  delay-per-exec 0

namespace ConfigFromFile {

// prosched/config.txt is present and readable from the repo root CWD
TEST(ConfigFromFile, ValidFileReturnsNonNull) {
  ConfigStruct *cs = fromFile();
  ASSERT_NE(cs, nullptr);
  delete cs;
}

// "num-cpu 4" in config.txt is parsed to num_cpu
TEST(ConfigFromFile, ParsesNumCpu) {
  ConfigStruct *cs = fromFile();
  ASSERT_NE(cs, nullptr);
  EXPECT_EQ(cs->num_cpu, 4);
  delete cs;
}

// "scheduler fcfs" in config.txt is parsed to scheduler
TEST(ConfigFromFile, ParsesScheduler) {
  ConfigStruct *cs = fromFile();
  ASSERT_NE(cs, nullptr);
  EXPECT_EQ(cs->scheduler, "fcfs");
  delete cs;
}

// "quantum-cycles 5" is parsed to quantum_cycles
TEST(ConfigFromFile, ParsesQuantumCycles) {
  ConfigStruct *cs = fromFile();
  ASSERT_NE(cs, nullptr);
  EXPECT_EQ(cs->quantum_cycles, 5);
  delete cs;
}

// "batch-process-freq 1" is parsed to batch_process_freq
TEST(ConfigFromFile, ParsesBatchProcessFreq) {
  ConfigStruct *cs = fromFile();
  ASSERT_NE(cs, nullptr);
  EXPECT_EQ(cs->batch_process_freq, 1);
  delete cs;
}

// "min-ins 1000" is parsed to min_ins
TEST(ConfigFromFile, ParsesMinIns) {
  ConfigStruct *cs = fromFile();
  ASSERT_NE(cs, nullptr);
  EXPECT_EQ(cs->min_ins, 1000);
  delete cs;
}

// "max-ins 2000" is parsed to max_ins
TEST(ConfigFromFile, ParsesMaxIns) {
  ConfigStruct *cs = fromFile();
  ASSERT_NE(cs, nullptr);
  EXPECT_EQ(cs->max_ins, 2000);
  delete cs;
}

// "delay-per-exec 0" is parsed to delay_per_exec
TEST(ConfigFromFile, ParsesDelayPerExec) {
  ConfigStruct *cs = fromFile();
  ASSERT_NE(cs, nullptr);
  EXPECT_EQ(cs->delay_per_exec, 0);
  delete cs;
}

} // namespace ConfigFromFile

namespace ConfigBuildConfig {

// num_cpu field transfers to AlgoContext.numCpu
TEST(ConfigBuildConfig, MapsNumCpu) {
  ConfigStruct cs{.num_cpu = 8,
                  .scheduler = "fcfs",
                  .quantum_cycles = 10,
                  .batch_process_freq = 2,
                  .min_ins = 500,
                  .max_ins = 1000,
                  .delay_per_exec = 1};
  AlgoContext ctx = AlgoContext::buildConfig(&cs);
  EXPECT_EQ(ctx.numCpu, 8);
}

// "fcfs" string becomes SchedulerType::FCFS enum
TEST(ConfigBuildConfig, MapsSchedulerFcfs) {
  ConfigStruct cs{.num_cpu = 4,
                  .scheduler = "fcfs",
                  .quantum_cycles = 5,
                  .batch_process_freq = 1,
                  .min_ins = 1000,
                  .max_ins = 2000,
                  .delay_per_exec = 0};
  AlgoContext ctx = AlgoContext::buildConfig(&cs);
  EXPECT_EQ(ctx.schedulerType, SchedulerType::FCFS);
}

// "rr" string becomes SchedulerType::RR enum
TEST(ConfigBuildConfig, MapsSchedulerRr) {
  ConfigStruct cs{.num_cpu = 4,
                  .scheduler = "rr",
                  .quantum_cycles = 5,
                  .batch_process_freq = 1,
                  .min_ins = 1000,
                  .max_ins = 2000,
                  .delay_per_exec = 0};
  AlgoContext ctx = AlgoContext::buildConfig(&cs);
  EXPECT_EQ(ctx.schedulerType, SchedulerType::RR);
}

// Unrecognized scheduler string becomes SchedulerType::UNKNOWN
TEST(ConfigBuildConfig, UnknownSchedulerMapsToUnknown) {
  ConfigStruct cs{.num_cpu = 4,
                  .scheduler = "sjf",
                  .quantum_cycles = 5,
                  .batch_process_freq = 1,
                  .min_ins = 1000,
                  .max_ins = 2000,
                  .delay_per_exec = 0};
  AlgoContext ctx = AlgoContext::buildConfig(&cs);
  EXPECT_EQ(ctx.schedulerType, SchedulerType::UNKNOWN);
}

// batch_process_freq transfers to batchProcessFreq
TEST(ConfigBuildConfig, MapsBatchProcessFreq) {
  ConfigStruct cs{.num_cpu = 4,
                  .scheduler = "fcfs",
                  .quantum_cycles = 5,
                  .batch_process_freq = 3,
                  .min_ins = 1000,
                  .max_ins = 2000,
                  .delay_per_exec = 0};
  AlgoContext ctx = AlgoContext::buildConfig(&cs);
  EXPECT_EQ(ctx.batchProcessFreq, 3);
}

// min_ins and max_ins both transfer correctly
TEST(ConfigBuildConfig, MapsMinAndMaxIns) {
  ConfigStruct cs{.num_cpu = 4,
                  .scheduler = "fcfs",
                  .quantum_cycles = 5,
                  .batch_process_freq = 1,
                  .min_ins = 200,
                  .max_ins = 800,
                  .delay_per_exec = 0};
  AlgoContext ctx = AlgoContext::buildConfig(&cs);
  EXPECT_EQ(ctx.minIns, 200);
  EXPECT_EQ(ctx.maxIns, 800);
}

// quantum_cycles transfers to quantumCycles
TEST(ConfigBuildConfig, MapsQuantumCycles) {
  ConfigStruct cs{.num_cpu = 4,
                  .scheduler = "fcfs",
                  .quantum_cycles = 7,
                  .batch_process_freq = 1,
                  .min_ins = 1000,
                  .max_ins = 2000,
                  .delay_per_exec = 0};
  AlgoContext ctx = AlgoContext::buildConfig(&cs);
  EXPECT_EQ(ctx.quantumCycles, 7);
}

// delay_per_exec transfers to delayPerExec
TEST(ConfigBuildConfig, MapsDelayPerExec) {
  ConfigStruct cs{.num_cpu = 4,
                  .scheduler = "fcfs",
                  .quantum_cycles = 5,
                  .batch_process_freq = 1,
                  .min_ins = 1000,
                  .max_ins = 2000,
                  .delay_per_exec = 5};
  AlgoContext ctx = AlgoContext::buildConfig(&cs);
  EXPECT_EQ(ctx.delayPerExec, 5);
}

} // namespace ConfigBuildConfig

// ── Helpers used only by ConfigValidation tests ──────────────────────────────

namespace {

static const char *TEMP_CFG = "/tmp/prosched_test_cfg.txt";

static void writeTestConfig(const std::string &content) {
  std::ofstream f(TEMP_CFG);
  f << content;
}

// Mirrors fromFile() logic but accepts a custom path, so tests can point at
// temp files or deliberately nonexistent paths without touching config.h.
static ConfigStruct *fromFileAt(const std::string &path) {
  std::ifstream f(path);
  if (!f.is_open())
    return nullptr;
  ConfigStruct *cs = new ConfigStruct();
  std::string line;
  while (getline(f, line)) {
    std::istringstream iss(line);
    std::string key;
    iss >> key;
    if (key == "num-cpu")
      iss >> cs->num_cpu;
    else if (key == "scheduler")
      iss >> cs->scheduler;
    else if (key == "quantum-cycles")
      iss >> cs->quantum_cycles;
    else if (key == "batch-process-freq")
      iss >> cs->batch_process_freq;
    else if (key == "min-ins")
      iss >> cs->min_ins;
    else if (key == "max-ins")
      iss >> cs->max_ins;
    else if (key == "delay-per-exec")
      iss >> cs->delay_per_exec;
  }
  return cs;
}

// Validates a loaded config and exits with code 1 on any problem.
// Called inside EXPECT_EXIT lambdas so the parent process can verify the exit.
static void validateOrDie(ConfigStruct *cs, const char *path = "") {
  if (cs == nullptr) {
    std::cerr << "[prosched] config file '" << path << "' not found\n";
    exit(1);
  }
  if (cs->num_cpu < 1) {
    std::cerr << "[prosched] 'num-cpu' missing or invalid (must be >= 1)\n";
    exit(1);
  }
  if (cs->scheduler != "fcfs" && cs->scheduler != "rr") {
    std::cerr << "[prosched] 'scheduler' missing or invalid (must be 'fcfs' or "
                 "'rr')\n";
    exit(1);
  }
  if (cs->quantum_cycles < 1) {
    std::cerr
        << "[prosched] 'quantum-cycles' missing or invalid (must be >= 1)\n";
    exit(1);
  }
  if (cs->batch_process_freq < 1) {
    std::cerr << "[prosched] 'batch-process-freq' missing or invalid (must be "
                 ">= 1)\n";
    exit(1);
  }
  if (cs->min_ins < 1) {
    std::cerr << "[prosched] 'min-ins' missing or invalid (must be >= 1)\n";
    exit(1);
  }
  if (cs->max_ins < cs->min_ins) {
    std::cerr << "[prosched] 'max-ins' must be >= 'min-ins'\n";
    exit(1);
  }
  if (cs->delay_per_exec < 0) {
    std::cerr << "[prosched] 'delay-per-exec' must be >= 0\n";
    exit(1);
  }
}

} // anonymous namespace

namespace ConfigValidation {

// A nonexistent config file returns nullptr → program must exit
TEST(ConfigValidation, MissingFileExits) {
  EXPECT_EXIT(
      {
        ConfigStruct *cs = fromFileAt("/tmp/prosched_no_such_file.txt");
        validateOrDie(cs, "/tmp/prosched_no_such_file.txt");
      },
      ::testing::ExitedWithCode(1), ".*");
}

// Config with no 'num-cpu' line → num_cpu = 0 → must exit
TEST(ConfigValidation, MissingNumCpuExits) {
  writeTestConfig("scheduler fcfs\nquantum-cycles 5\nbatch-process-freq 1\n"
                  "min-ins 100\nmax-ins 200\ndelay-per-exec 0\n");
  EXPECT_EXIT(
      {
        ConfigStruct *cs = fromFileAt(TEMP_CFG);
        validateOrDie(cs);
      },
      ::testing::ExitedWithCode(1), ".*");
  remove(TEMP_CFG);
}

// Config with no 'scheduler' line → scheduler = "" → must exit
TEST(ConfigValidation, MissingSchedulerExits) {
  writeTestConfig("num-cpu 4\nquantum-cycles 5\nbatch-process-freq 1\n"
                  "min-ins 100\nmax-ins 200\ndelay-per-exec 0\n");
  EXPECT_EXIT(
      {
        ConfigStruct *cs = fromFileAt(TEMP_CFG);
        validateOrDie(cs);
      },
      ::testing::ExitedWithCode(1), ".*");
  remove(TEMP_CFG);
}

// Config with no 'quantum-cycles' line → quantum_cycles = 0 → must exit
TEST(ConfigValidation, MissingQuantumCyclesExits) {
  writeTestConfig("num-cpu 4\nscheduler fcfs\nbatch-process-freq 1\n"
                  "min-ins 100\nmax-ins 200\ndelay-per-exec 0\n");
  EXPECT_EXIT(
      {
        ConfigStruct *cs = fromFileAt(TEMP_CFG);
        validateOrDie(cs);
      },
      ::testing::ExitedWithCode(1), ".*");
  remove(TEMP_CFG);
}

// Config with no 'batch-process-freq' line → batch_process_freq = 0 → must exit
TEST(ConfigValidation, MissingBatchProcessFreqExits) {
  writeTestConfig("num-cpu 4\nscheduler fcfs\nquantum-cycles 5\n"
                  "min-ins 100\nmax-ins 200\ndelay-per-exec 0\n");
  EXPECT_EXIT(
      {
        ConfigStruct *cs = fromFileAt(TEMP_CFG);
        validateOrDie(cs);
      },
      ::testing::ExitedWithCode(1), ".*");
  remove(TEMP_CFG);
}

// Config with no 'min-ins' line → min_ins = 0 → must exit
TEST(ConfigValidation, MissingMinInsExits) {
  writeTestConfig("num-cpu 4\nscheduler fcfs\nquantum-cycles 5\n"
                  "batch-process-freq 1\nmax-ins 200\ndelay-per-exec 0\n");
  EXPECT_EXIT(
      {
        ConfigStruct *cs = fromFileAt(TEMP_CFG);
        validateOrDie(cs);
      },
      ::testing::ExitedWithCode(1), ".*");
  remove(TEMP_CFG);
}

// Config with no 'max-ins' line → max_ins = 0 < min_ins → must exit
TEST(ConfigValidation, MissingMaxInsExits) {
  writeTestConfig("num-cpu 4\nscheduler fcfs\nquantum-cycles 5\n"
                  "batch-process-freq 1\nmin-ins 100\ndelay-per-exec 0\n");
  EXPECT_EXIT(
      {
        ConfigStruct *cs = fromFileAt(TEMP_CFG);
        validateOrDie(cs);
      },
      ::testing::ExitedWithCode(1), ".*");
  remove(TEMP_CFG);
}

// ── Invalid values ────────────────────────────────────────────────────────

// num-cpu 0 → must exit
TEST(ConfigValidation, InvalidNumCpuZeroExits) {
  writeTestConfig(
      "num-cpu 0\nscheduler fcfs\nquantum-cycles 5\n"
      "batch-process-freq 1\nmin-ins 100\nmax-ins 200\ndelay-per-exec 0\n");
  EXPECT_EXIT(
      {
        ConfigStruct *cs = fromFileAt(TEMP_CFG);
        validateOrDie(cs);
      },
      ::testing::ExitedWithCode(1), ".*");
  remove(TEMP_CFG);
}

// num-cpu -1 → must exit
TEST(ConfigValidation, InvalidNumCpuNegativeExits) {
  writeTestConfig(
      "num-cpu -1\nscheduler fcfs\nquantum-cycles 5\n"
      "batch-process-freq 1\nmin-ins 100\nmax-ins 200\ndelay-per-exec 0\n");
  EXPECT_EXIT(
      {
        ConfigStruct *cs = fromFileAt(TEMP_CFG);
        validateOrDie(cs);
      },
      ::testing::ExitedWithCode(1), ".*");
  remove(TEMP_CFG);
}

// scheduler sjf (unsupported) → must exit
TEST(ConfigValidation, InvalidSchedulerExits) {
  writeTestConfig(
      "num-cpu 4\nscheduler sjf\nquantum-cycles 5\n"
      "batch-process-freq 1\nmin-ins 100\nmax-ins 200\ndelay-per-exec 0\n");
  EXPECT_EXIT(
      {
        ConfigStruct *cs = fromFileAt(TEMP_CFG);
        validateOrDie(cs);
      },
      ::testing::ExitedWithCode(1), ".*");
  remove(TEMP_CFG);
}

// quantum-cycles 0 → must exit
TEST(ConfigValidation, InvalidQuantumCyclesZeroExits) {
  writeTestConfig(
      "num-cpu 4\nscheduler fcfs\nquantum-cycles 0\n"
      "batch-process-freq 1\nmin-ins 100\nmax-ins 200\ndelay-per-exec 0\n");
  EXPECT_EXIT(
      {
        ConfigStruct *cs = fromFileAt(TEMP_CFG);
        validateOrDie(cs);
      },
      ::testing::ExitedWithCode(1), ".*");
  remove(TEMP_CFG);
}

// batch-process-freq 0 → must exit
TEST(ConfigValidation, InvalidBatchProcessFreqZeroExits) {
  writeTestConfig(
      "num-cpu 4\nscheduler fcfs\nquantum-cycles 5\n"
      "batch-process-freq 0\nmin-ins 100\nmax-ins 200\ndelay-per-exec 0\n");
  EXPECT_EXIT(
      {
        ConfigStruct *cs = fromFileAt(TEMP_CFG);
        validateOrDie(cs);
      },
      ::testing::ExitedWithCode(1), ".*");
  remove(TEMP_CFG);
}

// min-ins 0 → must exit
TEST(ConfigValidation, InvalidMinInsZeroExits) {
  writeTestConfig(
      "num-cpu 4\nscheduler fcfs\nquantum-cycles 5\n"
      "batch-process-freq 1\nmin-ins 0\nmax-ins 200\ndelay-per-exec 0\n");
  EXPECT_EXIT(
      {
        ConfigStruct *cs = fromFileAt(TEMP_CFG);
        validateOrDie(cs);
      },
      ::testing::ExitedWithCode(1), ".*");
  remove(TEMP_CFG);
}

// max-ins < min-ins → must exit
TEST(ConfigValidation, InvalidMaxInsLessThanMinInsExits) {
  writeTestConfig(
      "num-cpu 4\nscheduler fcfs\nquantum-cycles 5\n"
      "batch-process-freq 1\nmin-ins 1000\nmax-ins 500\ndelay-per-exec 0\n");
  EXPECT_EXIT(
      {
        ConfigStruct *cs = fromFileAt(TEMP_CFG);
        validateOrDie(cs);
      },
      ::testing::ExitedWithCode(1), ".*");
  remove(TEMP_CFG);
}

// delay-per-exec -1 → must exit
TEST(ConfigValidation, InvalidDelayPerExecNegativeExits) {
  writeTestConfig(
      "num-cpu 4\nscheduler fcfs\nquantum-cycles 5\n"
      "batch-process-freq 1\nmin-ins 100\nmax-ins 200\ndelay-per-exec -1\n");
  EXPECT_EXIT(
      {
        ConfigStruct *cs = fromFileAt(TEMP_CFG);
        validateOrDie(cs);
      },
      ::testing::ExitedWithCode(1), ".*");
  remove(TEMP_CFG);
}

// A fully valid config must pass validation without exiting
TEST(ConfigValidation, ValidConfigPassesValidation) {
  writeTestConfig(
      "num-cpu 4\nscheduler fcfs\nquantum-cycles 5\n"
      "batch-process-freq 1\nmin-ins 100\nmax-ins 200\ndelay-per-exec 0\n");
  ConfigStruct *cs = fromFileAt(TEMP_CFG);
  ASSERT_NE(cs, nullptr);
  validateOrDie(cs); // must not call exit()
  delete cs;
  remove(TEMP_CFG);
}

// rr scheduler is also a valid choice
TEST(ConfigValidation, ValidConfigRrPassesValidation) {
  writeTestConfig(
      "num-cpu 2\nscheduler rr\nquantum-cycles 3\n"
      "batch-process-freq 2\nmin-ins 50\nmax-ins 500\ndelay-per-exec 10\n");
  ConfigStruct *cs = fromFileAt(TEMP_CFG);
  ASSERT_NE(cs, nullptr);
  validateOrDie(cs);
  delete cs;
  remove(TEMP_CFG);
}

} // namespace ConfigValidation
