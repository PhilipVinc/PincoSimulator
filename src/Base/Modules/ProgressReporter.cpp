//
// Created by Filippo Vicentini on 20/04/2018.
//

#include "ProgressReporter.hpp"

#include "Base/FileFormats/DataStore.hpp"
#include "Base/Settings.hpp"
#include "Base/TaskProcessor.hpp"
#include "Base/TaskResults.hpp"
#include "Base/Utils/StringFormatter.hpp"

#include <chrono>
#include <iostream>

#include "easylogging++.h"

ProgressReporter::ProgressReporter(const Settings *settings,
                                   const std::shared_ptr<TaskProcessor> _proc,
                                   bool forceFile)
    : processor(_proc), terminate(false) {
  auto term = settings->terminal;
  if (term == Settings::Terminal::dumb) {
    logging = ProgressReporter::dumb;
  } else if (term == Settings::Terminal::none) {
    logging = logType::file;
  } else {
    logging = logType::smart;
  }

  if (forceFile) { logging = logType::file; }

  IOThread = std::thread(&ProgressReporter::IOThreadUpdate, this);
}

ProgressReporter::~ProgressReporter() {
  terminate = true;
  IOThread.join();
  LOG(INFO) << "Deleted ProgressReporter.";
}

void ProgressReporter::Update() {}

void ProgressReporter::IOThreadUpdate() {
  while (nTasks == 0 && !terminate) { std::this_thread::sleep_for(std::chrono::seconds(1)); }

  std::chrono::system_clock::time_point startTime =
      std::chrono::system_clock::now();
  std::chrono::system_clock::time_point lastPrintTime = startTime;
  std::chrono::system_clock::duration deltaTPrint     = std::chrono::seconds(1);
  std::chrono::system_clock::duration deltaTCompute   = std::chrono::seconds(1);
  size_t lastMsgLength                                = 0;

  while (!terminate) {
    auto now                               = std::chrono::system_clock::now();
    std::chrono::system_clock::duration dt = now - lastPrintTime;
    if (dt > deltaTPrint && (nTasks != 0)) {
      std::chrono::system_clock::duration elapsed = now - startTime;
      int deltaTc =
          std::chrono::duration_cast<std::chrono::seconds>(elapsed).count();

      auto nCompletedTasks = processor->NumberOfCompletedTasks();
      auto progress        = processor->Progress() / float(nTasks) * 100;

      if (nCompletedTasks == nTasks) { terminate = true; }

      std::string tmptmp =
          progress != 0 ? std::to_string(int(deltaTc / progress * 100)) : "***";
      std::string msgString =
          string_format("(%.2f%%) Completed %i/%i.   Time: (%i/%s). ", progress,
                        nCompletedTasks, nTasks, deltaTc, tmptmp.c_str());

      if (logging == logType::file) {
        LOG(INFO) << msgString;
      } else if (logging == logType::dumb) {
        std::string tmp = std::string(lastMsgLength, '\b');
        lastMsgLength   = msgString.length() - 1;
        msgString       = tmp + msgString;
        cout << msgString;
        fflush(stdout);
      } else if (logging == logType::smart) {
        msgString = "\r" + msgString;
        cout << msgString;
        fflush(stdout);
      }
      lastPrintTime = now;
    }
    std::this_thread::sleep_for(deltaTCompute);
  }

  auto now                                    = chrono::system_clock::now();
  std::chrono::system_clock::duration elapsed = now - startTime;
  int deltaTc =
      std::chrono::duration_cast<std::chrono::seconds>(elapsed).count();
  std::string msgString =
      string_format("Completed %i tasks in %i seconds. ", nTasks, deltaTc);
  LOG(INFO) << msgString << endl;
}
