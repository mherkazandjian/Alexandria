/*
 * Copyright (C) 2012-2020 Euclid Science Ground Segment
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 3.0 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * @file src/lib/AsciiReader.cpp
 * @date 12/02/16
 * @author nikoapos
 */

#include <fstream>
#include <set>
// The std regex library is not fully implemented in GCC 4.8. The following lines
// make use of the BOOST library and can be modified if GCC 4.9 will be used in
// the future.
// #include <regex>
#include <boost/regex.hpp>
using boost::regex;
using boost::regex_match;
#include <boost/algorithm/string.hpp>

#include "ElementsKernel/Exception.h"
#include "Table/AsciiReader.h"

#include "ReaderHelper.h"
#include "AsciiReaderHelper.h"

namespace Euclid {
namespace Table {

AsciiReader::AsciiReader(std::istream& stream) : AsciiReader(InstOrRefHolder<std::istream>::create(stream)) {
}

AsciiReader::AsciiReader(const std::string& filename) : AsciiReader(create<std::ifstream>(filename)) {
}

AsciiReader::AsciiReader(std::unique_ptr<InstOrRefHolder<std::istream>> stream_holder)
        : m_stream_holder(std::move(stream_holder)) {
}

AsciiReader& AsciiReader::setCommentIndicator(const std::string& indicator) {
  if (m_reading_started) {
    throw Elements::Exception() << "Changing comment indicator after reading "
            << "has started is not allowed";
  }
  if (indicator.empty()) {
    throw Elements::Exception() << "Empty string as comment indicator";
  }
  m_comment = indicator;
  return *this;
}

AsciiReader& AsciiReader::fixColumnNames(std::vector<std::string> column_names) {
  if (m_reading_started) {
    throw Elements::Exception() << "Fixing the column names after reading "
            << "has started is not allowed";
  }
  
  m_column_names = std::move(column_names);
  
  std::set<std::string> set {};
  regex whitespace {".*\\s.*"}; // Checks if input contains any whitespace characters
  for (const auto& name : m_column_names) {
    if (name.empty()) {
      throw Elements::Exception() << "Empty string column names are not allowed";
    }
    if (regex_match(name, whitespace)) {
      throw Elements::Exception() << "Column name '" << name << "' contains "
                                << "whitespace characters";
    }
    if (!set.insert(name).second) {  // Check for duplicate names
      throw Elements::Exception() << "Duplicate column name " << name;
    }
  }
  if (!m_column_names.empty() && !m_column_types.empty()
      && m_column_names.size() != m_column_types.size()) {
    throw Elements::Exception() << "Different number of column names and types";
  }
  
  return *this;
}

AsciiReader& AsciiReader::fixColumnTypes(std::vector<std::type_index> column_types) {
  if (m_reading_started) {
    throw Elements::Exception() << "Fixing the column types after reading "
            << "has started is not allowed";
  }

  m_column_types = std::move(column_types);
  
  if (!m_column_names.empty() && !m_column_types.empty()
      && m_column_names.size() != m_column_types.size()) {
    throw Elements::Exception() << "Different number of column names and types";
  }
  
  return *this;
}

void AsciiReader::readColumnInfo() {
  if (m_column_info != nullptr) {
    return;
  }
  m_reading_started = true;
  
  auto& in = m_stream_holder->ref();
  
  size_t columns_number = countColumns(in, m_comment);
  if (!m_column_names.empty() && m_column_names.size() != columns_number) {
    throw Elements::Exception() << "Columns number in stream (" << columns_number
                                << ") does not match the column names number ("
                                << m_column_names.size() << ")";
  }
  if (!m_column_types.empty() && m_column_types.size() != columns_number) {
    throw Elements::Exception() << "Columns number in stream (" << columns_number
                                << ") does not match the column types number ("
                                << m_column_types.size() << ")";
  }
  
  auto auto_names = autoDetectColumnNames(in, m_comment, columns_number);
  auto auto_desc = autoDetectColumnDescriptions(in, m_comment);
  
  std::vector<std::string> names {};
  std::vector<std::type_index> types {};
  std::vector<std::string> units {};
  std::vector<std::string> descriptions {};
  for (size_t i=0; i<columns_number; ++i) {
    if (m_column_names.empty()) {
      names.emplace_back(auto_names[i]);
    } else {
      names.emplace_back(m_column_names[i]);
    }
    auto info = auto_desc.find(auto_names[i]);
    if (info != auto_desc.end()) {
      if (m_column_types.empty()) {
        types.emplace_back(info->second.type);
      } else {
        types.emplace_back(m_column_types[i]);
      }
      units.emplace_back(info->second.unit);
      descriptions.emplace_back(info->second.description);
    } else {
      if (m_column_types.empty()) {
        types.emplace_back(typeid(std::string));
      } else {
        types.emplace_back(m_column_types[i]);
      }
      units.emplace_back("");
      descriptions.emplace_back("");
    }
  }
  m_column_info = createColumnInfo(names, types, units, descriptions);
  
}


const ColumnInfo& AsciiReader::getInfo() {
  readColumnInfo();
  return *m_column_info;
}

Table AsciiReader::read(long rows) {
  readColumnInfo();
  auto& in = m_stream_holder->ref();
  
  std::vector<Row> row_list;
  regex column_separator {"\\s+"};
  while(in && rows != 0) {
    std::string line;
    getline(in, line);
    size_t comment_pos = line.find(m_comment);
    if (comment_pos != std::string::npos) {
      line = line.substr(0, comment_pos);
    }
    boost::trim(line);
    if (!line.empty()) {
      --rows;
      boost::sregex_token_iterator i (line.begin(), line.end(), column_separator, -1);
      boost::sregex_token_iterator j;
      size_t count {0};
      std::vector<Row::cell_type> values {};
      while (i != j) {
        if (count >= m_column_info->size()) {
          throw Elements::Exception() << "Line with wrong number of cells: " << line;
        }
        values.push_back(convertToCellType(*i, m_column_info->getDescription(count).type));
        ++count;
        ++i;
      }
      row_list.push_back(Row{std::move(values), m_column_info});
    }
  }
  
  if (row_list.empty()) {
    throw Elements::Exception() << "No more table rows left";
  }
  return Table{std::move(row_list)};
}

void AsciiReader::skip(long rows) {
  readColumnInfo();
  auto& in = m_stream_holder->ref();
  
  while(in && rows != 0) {
    std::string line;
    getline(in, line);
    size_t comment_pos = line.find(m_comment);
    if (comment_pos != std::string::npos) {
      line = line.substr(0, comment_pos);
    }
    boost::trim(line);
    if (!line.empty()) {
      --rows;
    }
  }
}

bool AsciiReader::hasMoreRows() {
  return hasNextRow(m_stream_holder->ref(), m_comment);
}

} // Table namespace
} // Euclid namespace



