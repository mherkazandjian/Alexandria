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
 * @file src/lib/AsciiReaderHelper.cpp
 * @date April 15, 2014
 * @author Nikolaos Apostolakos
 */

#include <set>
#include <sstream>
#include <boost/regex.hpp>
using boost::regex;
using boost::regex_match;
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>
#include "ElementsKernel/Exception.h"
#include "ElementsKernel/Logging.h"
#include "AsciiReaderHelper.h"
#include "NdArray/NdArray.h"

namespace Euclid {
namespace Table {

using NdArray::NdArray;

static Elements::Logging logger = Elements::Logging::getLogger("AsciiReader");

size_t countColumns(std::istream& in, const std::string& comment) {
  StreamRewinder rewinder {in};
  size_t count = 0;
  regex column_separator {"\\s+"};
  while (in) {
    std::string line;
    getline(in, line);
    // Remove any comments
    size_t comment_pos = line.find(comment);
    if (comment_pos != std::string::npos) {
      line = line.substr(0, comment_pos);
    }
    boost::trim(line);
    if (!line.empty()) {
      boost::sregex_token_iterator i (line.begin(), line.end(), column_separator, -1);
      boost::sregex_token_iterator j;
      while (i != j) {
        ++i;
        ++count;
      }
      break;
    }
  }
  if (count == 0) {
    throw Elements::Exception() << "No data lines found";
  }
  return count;
}

std::type_index keywordToType(const std::string& keyword) {
  if (keyword == "bool" || keyword == "boolean") {
    return typeid(bool);
  } else if (keyword == "int" || keyword == "int32") {
    return typeid(int32_t);
  } else if (keyword == "long" || keyword == "int64") {
    return typeid(int64_t);
  } else if (keyword == "float") {
    return typeid(float);
  } else if (keyword == "double") {
    return typeid(double);
  } else if (keyword == "string") {
    return typeid(std::string);
  } else if (keyword == "[bool]" || keyword == "[boolean]") {
    return typeid(std::vector<bool>);
  } else if (keyword == "[int]" || keyword == "[int32]") {
    return typeid(std::vector<int32_t>);
  } else if (keyword == "[long]" || keyword == "[int64]") {
    return typeid(std::vector<int64_t>);
  } else if (keyword == "[float]") {
    return typeid(std::vector<float>);
  } else if (keyword == "[double]") {
    return typeid(std::vector<double>);
  } else if (keyword == "[bool+]" || keyword == "[boolean+]") {
    return typeid(NdArray<bool>);
  } else if (keyword == "[int+]" || keyword == "[int32+]") {
    return typeid(NdArray<int32_t>);
  } else if (keyword == "[long+]" || keyword == "[int64+]") {
    return typeid(NdArray<int64_t>);
  } else if (keyword == "[float+]") {
    return typeid(NdArray<float>);
  } else if (keyword == "[double+]") {
    return typeid(NdArray<double>);
  }
  throw Elements::Exception() << "Unknown column type keyword " << keyword;
}

std::map<std::string, ColumnDescription> autoDetectColumnDescriptions(
                                      std::istream& in, const std::string& comment) {
  StreamRewinder rewinder {in};
  std::map<std::string, ColumnDescription> descriptions;
  while (in) {
    std::string line;
    getline(in, line);
    boost::trim(line);
    if (line.empty()) {
      continue; // We skip empty lines
    }
    if (boost::starts_with(line, comment)) {
      // If we have a comment we remove all comment characters and check if we have
      // a column description
      boost::replace_all(line, comment, "");
      boost::trim(line);
      if (boost::starts_with(line, "Column:")) {
        line.erase(0, 7);
        boost::trim(line);
        if (!line.empty()) {
          boost::sregex_token_iterator token (line.begin(), line.end(), regex{"\\s+"}, -1);
          boost::sregex_token_iterator end;
          std::string name = *token;
          if (descriptions.count(name) != 0) {
            throw Elements::Exception() << "Duplicate column name " << name;
          }
          ++token;
          std::type_index type = typeid(std::string);
          if (token != end) {
            std::string token_str =*token;
            if (!boost::starts_with(token_str, "(") && token_str != "-") {
              type = keywordToType(token_str);
              ++token;
            }
          }
          std::string unit = "";
          if (token != end) {
            std::string token_str = *token;
            if (boost::starts_with(token_str, "(")) {
              unit = token_str;
              unit.erase(unit.begin());
              unit.erase(unit.end()-1);
              ++token;
            }
          }
          if (token != end && *token == "-") {
            ++token;
          }
          std::stringstream desc;
          while (token != end) {
            desc << *token << ' ';
            ++token;
          }
          std::string desc_str = desc.str();
          boost::trim(desc_str);
          descriptions.emplace(std::piecewise_construct,
                               std::forward_as_tuple(name),
                               std::forward_as_tuple(name, type, unit, desc_str));
        }
      }
    } else {
      break; // here we reached the first data line
    }
  }
  return descriptions;
}

std::vector<std::string> autoDetectColumnNames(std::istream& in,
                                               const std::string& comment,
                                               size_t columns_number) {
  StreamRewinder rewinder {in};
  std::vector<std::string> names {};
  
  // Find the last comment line and at the same time read the names of the
  // column info description comments
  std::string last_comment {};
  std::vector<std::string> desc_names {};
  while (in) {
    std::string line;
    getline(in, line);
    boost::trim(line);
    if (line.empty()) {
      continue; // We skip empty lines
    }
    if (boost::starts_with(line, comment)) {
      // If we have a comment we remove all comment characters and check if we have
      // the correct number of tokens
      boost::replace_all(line, comment, "");
      boost::trim(line);
      if (!line.empty()) {
        last_comment = line;
      }
      if (boost::starts_with(line, "Column:")) {
        std::string temp = line;
        temp.erase(0, 7);
        boost::trim(temp);
        auto space_i = temp.find(' ');
        if (space_i > 0) {
          temp = temp.substr(0, space_i);
        }
        desc_names.emplace_back(std::move(temp));
      }
    } else {
      break; // here we reached the first data line
    }
  }
  
  // Check if the last comment line contains the names of the columns
  if (!last_comment.empty()){
    boost::sregex_token_iterator i (last_comment.begin(), last_comment.end(), regex{"\\s+"}, -1);
    boost::sregex_token_iterator j;
    while (i != j) {
      names.push_back(*i);
      ++i;
    }
    if (names.size() != columns_number) {
      names.clear();
    }
  }
  
  // If the names are empty we fill them with the column descriprion ones
  if (names.empty()) {
    if (desc_names.size() != 0 && desc_names.size() != columns_number) {
      logger.warn() << "Number of column descriptions does not matches the number"
              << " of the columns";
    }
    names = desc_names;
  }
  
  if (names.size() < columns_number) {
    for (size_t i=names.size()+1; i<=columns_number; ++i) {
      names.push_back("col" + std::to_string(i));
    }
  }
  // Check for duplicate names
  std::set<std::string> set {};
  for (auto name : names) {
    if (!set.insert(name).second) {
      throw Elements::Exception() << "Duplicate column name " << name;
    }
  }
  return names;
}

namespace {

template <typename T>
std::vector<T> convertStringToVector(const std::string& str) {
  std::vector<T> result {};
  boost::char_separator<char> sep {","};
  boost::tokenizer< boost::char_separator<char> > tok {str, sep};
  for (auto& s : tok) {
    result.push_back(boost::get<T>(convertToCellType(s, typeid(T))));
  }
  return result;
}

template <typename T>
NdArray<T> convertStringToNdArray(const std::string& str) {
  if (str.empty()) {
    throw Elements::Exception() << "Cannot convert an empty string to a NdArray";
  } else if (str[0] != '<') {
    throw Elements::Exception() << "Unexpected initial character for a NdArray: " << str[0];
  }

  auto closing_char = str.find('>');
  if (closing_char == std::string::npos) {
    throw Elements::Exception() << "Could not find '>'";
  }

  auto shape_str = str.substr(1, closing_char - 1);
  auto shape_i = convertStringToVector<int32_t>(shape_str);
  auto data = convertStringToVector<T>(str.substr(closing_char + 1));

  std::vector<size_t> shape_u;
  std::copy(shape_i.begin(), shape_i.end(), std::back_inserter(shape_u));
  return NdArray<T>(shape_u, data);
}

}

Row::cell_type convertToCellType(const std::string& value, std::type_index type) {
  try {
    if (type == typeid(bool)) {
      if (value == "true" || value == "t" || value == "yes" || value == "y" || value == "1") {
        return Row::cell_type {true};
      }
      if (value == "false" || value == "f" || value == "no" || value == "n" || value == "0") {
        return Row::cell_type {false};
      }
    } else if (type == typeid(int32_t)) {
      return Row::cell_type {boost::lexical_cast<int32_t>(value)};
    } else if (type == typeid(int64_t)) {
      return Row::cell_type {boost::lexical_cast<int64_t>(value)};
    } else if (type == typeid(float)) {
      return Row::cell_type {boost::lexical_cast<float>(value)};
    } else if (type == typeid(double)) {
      return Row::cell_type {boost::lexical_cast<double>(value)};
    } else if (type == typeid(std::string)) {
      return Row::cell_type {boost::lexical_cast<std::string>(value)};
    } else if (type == typeid(std::vector<bool>)) {
      return Row::cell_type {convertStringToVector<bool>(value)};
    } else if (type == typeid(std::vector<int32_t>)) {
      return Row::cell_type {convertStringToVector<int32_t>(value)};
    } else if (type == typeid(std::vector<int64_t>)) {
      return Row::cell_type {convertStringToVector<int64_t>(value)};
    } else if (type == typeid(std::vector<float>)) {
      return Row::cell_type {convertStringToVector<float>(value)};
    } else if (type == typeid(std::vector<double>)) {
      return Row::cell_type {convertStringToVector<double>(value)};
    } else if (type == typeid(NdArray<bool>)) {
      return Row::cell_type {convertStringToNdArray<bool>(value)};
    } else if (type == typeid(NdArray<int32_t>)) {
      return Row::cell_type {convertStringToNdArray<int32_t>(value)};
    } else if (type == typeid(NdArray<int64_t>)) {
      return Row::cell_type {convertStringToNdArray<int64_t>(value)};
    } else if (type == typeid(NdArray<float>)) {
      return Row::cell_type {convertStringToNdArray<float>(value)};
    } else if (type == typeid(NdArray<double>)) {
      return Row::cell_type {convertStringToNdArray<double>(value)};
    }
  } catch( boost::bad_lexical_cast const& ) {
    throw Elements::Exception() << "Cannot convert " << value << " to " << type.name();
  }
  throw Elements::Exception() << "Unknown type name " << type.name();
}

bool hasNextRow(std::istream& in, const std::string& comment) {
  StreamRewinder rewinder {in};
  while(in) {
    std::string line;
    getline(in, line);
    size_t comment_pos = line.find(comment);
    if (comment_pos != std::string::npos) {
      line = line.substr(0, comment_pos);
    }
    boost::trim(line);
    if (!line.empty()) {
      return true;
    }
  }
  return false;
}

std::size_t countRemainingRows(std::istream& in, const std::string& comment) {
  StreamRewinder rewinder {in};
  std::size_t count = 0;
  while(in) {
    std::string line;
    getline(in, line);
    size_t comment_pos = line.find(comment);
    if (comment_pos != std::string::npos) {
      line = line.substr(0, comment_pos);
    }
    boost::trim(line);
    if (!line.empty()) {
      ++count;
    }
  }
  return count;
}

}
} // end of namespace Euclid
