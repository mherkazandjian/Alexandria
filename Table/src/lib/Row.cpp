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
 * @file src/lib/Row.cpp
 * @date April 8, 2014
 * @author Nikolaos Apostolakos
 */

#include <algorithm>
// The std regex library is not fully implemented in GCC 4.8. The following lines
// make use of the BOOST library and can be modified if GCC 4.9 will be used in
// the future.
// #include <regex>
#include <boost/regex.hpp>
using boost::regex;
using boost::regex_match;
#include <boost/algorithm/string/join.hpp>
#include "ElementsKernel/Exception.h"
#include "Table/Row.h"

namespace std {

template <typename T>
std::ostream& operator<< (std::ostream& s, const std::vector<T>& v) {
  auto it = v.begin();
  if (it != v.end()) {
    s << *it;
  }
  while (++it != v.end()) {
    s << ',' << *it;
  }
  return s;
}

template std::ostream& operator<< <double> (std::ostream& s, const std::vector<double>& v);
template std::ostream& operator<< <float> (std::ostream& s, const std::vector<float>& v);
template std::ostream& operator<< <std::int64_t> (std::ostream& s, const std::vector<std::int64_t>& v);
template std::ostream& operator<< <std::int32_t> (std::ostream& s, const std::vector<std::int32_t>& v);
template std::ostream& operator<< <bool> (std::ostream& s, const std::vector<bool>& v);

}

namespace Euclid {
namespace Table {

Row::Row(std::vector<cell_type> values, std::shared_ptr<ColumnInfo> column_info)
        : m_values(std::move(values)), m_column_info{column_info} {
  if (!m_column_info) {
    throw Elements::Exception() << "Row construction with nullptr column_info";
  }
  if (m_values.size() != m_column_info->size()) {
    throw Elements::Exception() << "Wrong number of row values (" << m_values.size()
                              << " instead of " << m_column_info->size();
  }
  for (std::size_t i=0; i<m_values.size(); ++i) {
    if (std::type_index{m_values[i].type()} != column_info->getDescription(i).type) {
      throw Elements::Exception() << "Incompatible cell type";
    }
  }
  regex whitespace {".*\\s.*"}; // Checks if input contains any whitespace characters
  for (auto cell : m_values) {
    if (cell.type() == typeid(std::string)) {
      std::string value = boost::get<std::string>(cell);
      if (value.empty()) {
        throw Elements::Exception() << "Empty string cell values are not allowed";
      }
      if (regex_match(value, whitespace)) {
        throw Elements::Exception() << "Cell value '" << value << "' contains "
                                  << "whitespace characters";
      }
    }
  }
}

std::shared_ptr<ColumnInfo> Row::getColumnInfo() const {
  return m_column_info;
}

size_t Row::size() const {
  return m_values.size();
}

const Row::cell_type& Row::operator [](const size_t index) const {
  if (index >= m_values.size()) {
    throw Elements::Exception("Index out of bounds");
  }
  return m_values[index];
}

const Row::cell_type& Row::operator [](const std::string& column) const {
  auto index = m_column_info->find(column);
  if (!index) {
    throw Elements::Exception() << "Row does not contain column with name " << column;
  }
  return m_values[*index];
}

Row::const_iterator Row::begin() const {
  return m_values.cbegin();
}

Row::const_iterator Row::end() const {
  return m_values.cend();
}

}
} // end of namespace Euclid