/* Copyright (c) 2024 Otto Link. Distributed under the terms of the GNU General
 * Public License. The full license is in the file LICENSE, distributed with
 * this software. */
#include <sstream>
#include <string>
#include <vector>

namespace gngui
{

std::vector<std::string> split_string(const std::string &string, char delimiter)
{
  std::vector<std::string> result;
  std::stringstream        ss(string);
  std::string              word;

  while (std::getline(ss, word, delimiter))
    result.push_back(word);

  return result;
}

} // namespace gngui
