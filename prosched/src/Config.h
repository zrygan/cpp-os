#ifndef CONFIG_H
#define CONFIG_H
#include <cassert>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

/**
 * @brief This struct contains the values from a config file.
 *  This should never be constructed by itself. That is, call
 *  makeDefault() or fromFile() to construct a ConfigStruct.
 */
struct ConfigStruct {
  int num_cpu;
  std::string scheduler;
  int rr_quantum_cycles;
  int batch_process_freq;
  int min_ins;
  int max_ins;
  int delay_per_exec;
};

/**
 * @brief This constant stores the fixed filename of the config file.
 */
const std::string CONFIG_FILENAME = "prosched/config.txt";

/**
 * @brief This function is for DEBUGGING PURPOSES only. This will
 *  create a ConfigStruct with the default values.
 *
 * @return Always returns a valid pointer to a ConfigStruct.
 */
inline ConfigStruct *makeDefault() {
  ConfigStruct *cs = new ConfigStruct{.num_cpu = 4,
                                      .scheduler = "rr",
                                      .rr_quantum_cycles = 5,
                                      .batch_process_freq = 1,
                                      .min_ins = 1000,
                                      .max_ins = 2000,
                                      .delay_per_exec = 0};
  return cs;
}

/**
 * @brief This function is the main procedure for making a
 *  ConfigStruct. This requires the constant `CONFIG_FILENAME` to be
 *  correct. This function parses and constructs a ConfigStruct from
 *  a file.
 *
 * @return The ConfigStruct defined by the file. If an error occurs
 *  anywhere, returns a `nullptr`.
 */
inline ConfigStruct *fromFile() {
  // Zhean: i swear cpp is so horrific.
  // what the hell is an ifstream (input file steam) but
  // why "if"steam ???
  std::ifstream configFile(CONFIG_FILENAME);
  if (!configFile.is_open()) {
    return nullptr;
  }

  ConfigStruct *cs = new ConfigStruct();
  std::string line;
  while (getline(configFile, line)) {
    std::istringstream iss(line);
    std::string key;
    iss >> key;

    if (key == "num-cpu")
      iss >> cs->num_cpu;
    else if (key == "scheduler")
      iss >> cs->scheduler;
    else if (key == "quantum-cycles")
      iss >> cs->rr_quantum_cycles;
    else if (key == "batch-process-freq")
      iss >> cs->batch_process_freq;
    else if (key == "min-ins")
      iss >> cs->min_ins;
    else if (key == "max-ins")
      iss >> cs->max_ins;
    else if (key == "delay-per-exec")
      iss >> cs->delay_per_exec;
  }

  configFile.close();
  return cs;
}

#endif // CONFIG_H