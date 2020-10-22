/* matching_statistics - Computes the matching statistics from BWT and Thresholds
    Copyright (C) 2020 Massimiliano Rossi
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see http://www.gnu.org/licenses/ .
*/
/*!
   \file matching_statistics.cpp
   \brief matching_statistics.cpp Computes the matching statistics from BWT and Thresholds.
   \author Massimiliano Rossi
   \date 13/07/2020
*/

#include <iostream>

#define VERBOSE

#include <common.hpp>

#include <sdsl/io.hpp>

#include <ms_pointers.hpp>

#include <malloc_count.h>

typedef std::pair<std::string, std::vector<uint8_t>> pattern_t;

std::vector<pattern_t> read_patterns(std::string filename)
{
  // Open File
  FILE *fd;
  if ((fd = fopen(filename.c_str(), "r")) == nullptr)
    error("open() file " + filename + " failed");

  std::vector<pattern_t> patterns;

  pattern_t pattern;

  char c;
  while (fread(&c, sizeof(char), 1, fd) == 1)
  {
    if (c == '>')
    {
      if (pattern.second.size() > 0)
        patterns.push_back(pattern);

      pattern.first.clear();
      pattern.second.clear();

      pattern.first.append(1, c);
      while (fread(&c, sizeof(char), 1, fd) == 1 && c != '\n')
        pattern.first.append(1, c);
    }
    else
    {
      pattern.second.push_back(c);
      while (fread(&c, sizeof(char), 1, fd) == 1 && c != '\n')
        pattern.second.push_back(c);
    }
  }

  if (pattern.second.size() > 0)
    patterns.push_back(pattern);

  fclose(fd);

  return patterns;
}

int main(int argc, char *const argv[]) {
  Args args;
  parseArgs(argc, argv, args);

  verbose("Building the matching statistics index");
  std::chrono::high_resolution_clock::time_point t_insert_start = std::chrono::high_resolution_clock::now();

  ms_pointers<> ms(args.filename);

  std::chrono::high_resolution_clock::time_point t_insert_end = std::chrono::high_resolution_clock::now();

  verbose("Matching statistics index construction complete");
  verbose("Memory peak: ", malloc_count_peak());
  verbose("Elapsed time (s): ", std::chrono::duration<double, std::ratio<1>>(t_insert_end - t_insert_start).count());

  verbose("Reading patterns");
  t_insert_start = std::chrono::high_resolution_clock::now();

  std::vector<pattern_t> patterns = read_patterns(args.patterns);

  t_insert_end = std::chrono::high_resolution_clock::now();

  verbose("Memory peak: ", malloc_count_peak());
  verbose("Elapsed time (s): ", std::chrono::duration<double, std::ratio<1>>(t_insert_end - t_insert_start).count());

  verbose("Processing patterns");
  t_insert_start = std::chrono::high_resolution_clock::now();

  std::ofstream f_pointers(args.patterns + ".pointers");
  std::ofstream f_lengths(args.patterns + ".lengths");

  if (!f_pointers.is_open())
    error("open() file " + std::string(args.filename) + ".pointers failed");

  if (!f_lengths.is_open())
    error("open() file " + std::string(args.filename) + ".lengths failed");

  for (auto pattern : patterns) {
   f_lengths << pattern.first << endl;
    auto [lengths,refs] = ms.query(pattern.second);
    for(const auto& length : lengths) {
      f_lengths << length << " ";
    }
    f_lengths << endl;

    f_pointers << pattern.first << endl;
    for (auto elem : refs)
      f_pointers << elem << " ";
    f_pointers << endl;
  }

  f_pointers.close();
  f_lengths.close();

  t_insert_end = std::chrono::high_resolution_clock::now();

  verbose("Memory peak: ", malloc_count_peak());
  verbose("Elapsed time (s): ", std::chrono::duration<double, std::ratio<1>>(t_insert_end - t_insert_start).count());

  auto mem_peak = malloc_count_peak();
  verbose("Memory peak: ", malloc_count_peak());

  size_t space = 0;
  if (args.memo)
  {
    verbose("Thresholds size (bytes): ", space);
  }

  if (args.store)
  {
  }

  if (args.csv)
    std::cerr << csv(args.filename.c_str(), time, space, mem_peak) << std::endl;

  return 0;
}
