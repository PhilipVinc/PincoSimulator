//
// Created by Filippo Vicentini on 16/05/2018.
//

#ifndef SIMULATOR_FILENOTEXISTSEXCEPTION_HPP
#define SIMULATOR_FILENOTEXISTSEXCEPTION_HPP

#include <stdexcept>
#include <string>

class FileNotExists : public std::runtime_error {
 public:
  FileNotExists(std::string& path)
      : std::runtime_error("Inexistant file to be loaded: " + path){};
};
#endif  // SIMULATOR_FILENOTEXISTSEXCEPTION_HPP
