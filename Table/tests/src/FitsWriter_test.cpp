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
 * @file tests/src/FitsWriter_test.cpp
 * @date 12/01/16
 * @author nikoapos
 */

#include <boost/test/unit_test.hpp>
#include <CCfits/CCfits>
#include "ElementsKernel/Temporary.h"
#include "Table/FitsWriter.h"

using namespace Euclid::Table;

struct FitsWriter_Fixture {
  std::vector<ColumnInfo::info_type> info_list {
      ColumnInfo::info_type("Boolean", typeid(bool), "deg", "Desc1"),
      ColumnInfo::info_type("Integer", typeid(int32_t), "mag", "Desc2"),
      ColumnInfo::info_type("Long", typeid(int64_t), "", "Desc3"),
      ColumnInfo::info_type("Float", typeid(float), "ph", "Desc4"),
      ColumnInfo::info_type("Double", typeid(double), "s", "Desc5"),
      ColumnInfo::info_type("String", typeid(std::string), "m", "Desc6")
  };
  std::shared_ptr<ColumnInfo> column_info {new ColumnInfo {info_list}};
  std::vector<Row::cell_type> values0 {true, 1, int64_t{123}, 0.F, 0., std::string{"first"}};
  Row row0 {values0, column_info};
  std::vector<Row::cell_type> values1 {false, 12345, int64_t{123456789}, 2.3e-2F, 1.12345e-18, std::string{"second"}};
  Row row1 {values1, column_info};
  std::vector<Row> row_list {row0, row1};
  Table table {row_list};
  Elements::TempDir temp_dir;
  std::string fits_file_path = (temp_dir.path()/"FitsWriter_test.fits").native();
};

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE (FitsWriter_test)

//-----------------------------------------------------------------------------
// Test the write Binary
//-----------------------------------------------------------------------------

BOOST_FIXTURE_TEST_CASE(writeBinary, FitsWriter_Fixture) {

  // Given
  FitsWriter writer {fits_file_path};
  writer.setFormat(FitsWriter::Format::BINARY);
  writer.setHduName("BinaryTable");

  // When
  writer.addData(table);
  CCfits::FITS fits {fits_file_path, CCfits::RWmode::Read};
  auto& result = fits.extension("BinaryTable");
  result.readAllKeys();

  // Then
  BOOST_CHECK_EQUAL(result.rows(), 2);
  BOOST_CHECK_EQUAL(result.numCols(), 6);

  BOOST_CHECK_EQUAL(result.column(1).name(), "Boolean");
  BOOST_CHECK_EQUAL(result.column(2).name(), "Integer");
  BOOST_CHECK_EQUAL(result.column(3).name(), "Long");
  BOOST_CHECK_EQUAL(result.column(4).name(), "Float");
  BOOST_CHECK_EQUAL(result.column(5).name(), "Double");
  BOOST_CHECK_EQUAL(result.column(6).name(), "String");

  BOOST_CHECK_EQUAL(result.column(1).format(), "L");
  BOOST_CHECK_EQUAL(result.column(2).format(), "J");
  BOOST_CHECK_EQUAL(result.column(3).format(), "K");
  BOOST_CHECK_EQUAL(result.column(4).format(), "E");
  BOOST_CHECK_EQUAL(result.column(5).format(), "D");
  BOOST_CHECK_EQUAL(result.column(6).format(), "6A");

  BOOST_CHECK_EQUAL(result.column(1).unit(), "deg");
  BOOST_CHECK_EQUAL(result.column(2).unit(), "mag");
  BOOST_CHECK_EQUAL(result.column(3).unit(), "");
  BOOST_CHECK_EQUAL(result.column(4).unit(), "ph");
  BOOST_CHECK_EQUAL(result.column(5).unit(), "s");
  BOOST_CHECK_EQUAL(result.column(6).unit(), "m");

  std::string tmp;
  BOOST_CHECK_EQUAL(result.keyWord("TDESC1").value(tmp), "Desc1");
  BOOST_CHECK_EQUAL(result.keyWord("TDESC2").value(tmp), "Desc2");
  BOOST_CHECK_EQUAL(result.keyWord("TDESC3").value(tmp), "Desc3");
  BOOST_CHECK_EQUAL(result.keyWord("TDESC4").value(tmp), "Desc4");
  BOOST_CHECK_EQUAL(result.keyWord("TDESC5").value(tmp), "Desc5");
  BOOST_CHECK_EQUAL(result.keyWord("TDESC6").value(tmp), "Desc6");

  // When
  std::vector<bool> bool_data {};
  result.column(1).read(bool_data, 1, 2);

  // Then
  BOOST_CHECK_EQUAL(bool_data[0], true);
  BOOST_CHECK_EQUAL(bool_data[1], false);

  // When
  std::vector<int32_t> int_data {};
  result.column(2).read(int_data, 1, 2);

  // Then
  BOOST_CHECK_EQUAL(int_data[0], 1);
  BOOST_CHECK_EQUAL(int_data[1], 12345);

  // When
  std::vector<int64_t> long_data {};
  result.column(3).read(long_data, 1, 2);

  // Then
  BOOST_CHECK_EQUAL(long_data[0], 123);
  BOOST_CHECK_EQUAL(long_data[1], 123456789);

  // When
  std::vector<float> float_data {};
  result.column(4).read(float_data, 1, 2);

  // Then
  BOOST_CHECK_EQUAL(float_data[0], 0.F);
  BOOST_CHECK_EQUAL(float_data[1], 2.3e-2F);

  // When
  std::vector<double> double_data {};
  result.column(5).read(double_data, 1, 2);

  // Then
  BOOST_CHECK_EQUAL(double_data[0], 0.F);
  BOOST_CHECK_EQUAL(double_data[1], 1.12345e-18);

  // When
  std::vector<std::string> string_data {};
  result.column(6).read(string_data, 1, 2);

  // Then
  BOOST_CHECK_EQUAL(string_data[0], "first");
  BOOST_CHECK_EQUAL(string_data[1], "second");

}

//-----------------------------------------------------------------------------
// Test the write ASCII
//-----------------------------------------------------------------------------

BOOST_FIXTURE_TEST_CASE(writeAscii, FitsWriter_Fixture) {

  // Given
  FitsWriter writer {fits_file_path};
  writer.setFormat(FitsWriter::Format::ASCII);
  writer.setHduName("AsciiTable");

  // When
  writer.addData(table);
  CCfits::FITS fits {fits_file_path, CCfits::RWmode::Read};
  auto& result = fits.extension("AsciiTable");
  result.readAllKeys();

  // Then
  BOOST_CHECK_EQUAL(result.rows(), 2);
  BOOST_CHECK_EQUAL(result.numCols(), 6);

  BOOST_CHECK_EQUAL(result.column(1).name(), "Boolean");
  BOOST_CHECK_EQUAL(result.column(2).name(), "Integer");
  BOOST_CHECK_EQUAL(result.column(3).name(), "Long");
  BOOST_CHECK_EQUAL(result.column(4).name(), "Float");
  BOOST_CHECK_EQUAL(result.column(5).name(), "Double");
  BOOST_CHECK_EQUAL(result.column(6).name(), "String");

  BOOST_CHECK_EQUAL(result.column(1).format(), "I1");
  BOOST_CHECK_EQUAL(result.column(2).format(), "I5");
  BOOST_CHECK_EQUAL(result.column(3).format(), "I9");
  BOOST_CHECK_EQUAL(result.column(4).format(), "E12");
  BOOST_CHECK_EQUAL(result.column(5).format(), "E12");
  BOOST_CHECK_EQUAL(result.column(6).format(), "A6");

  BOOST_CHECK_EQUAL(result.column(1).unit(), "deg");
  BOOST_CHECK_EQUAL(result.column(2).unit(), "mag");
  BOOST_CHECK_EQUAL(result.column(3).unit(), "");
  BOOST_CHECK_EQUAL(result.column(4).unit(), "ph");
  BOOST_CHECK_EQUAL(result.column(5).unit(), "s");
  BOOST_CHECK_EQUAL(result.column(6).unit(), "m");

  std::string tmp;
  BOOST_CHECK_EQUAL(result.keyWord("TDESC1").value(tmp), "Desc1");
  BOOST_CHECK_EQUAL(result.keyWord("TDESC2").value(tmp), "Desc2");
  BOOST_CHECK_EQUAL(result.keyWord("TDESC3").value(tmp), "Desc3");
  BOOST_CHECK_EQUAL(result.keyWord("TDESC4").value(tmp), "Desc4");
  BOOST_CHECK_EQUAL(result.keyWord("TDESC5").value(tmp), "Desc5");
  BOOST_CHECK_EQUAL(result.keyWord("TDESC6").value(tmp), "Desc6");


  // When
  std::vector<int32_t> bool_data {};
  result.column(1).read(bool_data, 1, 2);

  // Then
  BOOST_CHECK_EQUAL(bool_data[0], 1);
  BOOST_CHECK_EQUAL(bool_data[1], 0);

  // When
  std::vector<int32_t> int_data {};
  result.column(2).read(int_data, 1, 2);

  // Then
  BOOST_CHECK_EQUAL(int_data[0], 1);
  BOOST_CHECK_EQUAL(int_data[1], 12345);

  // When
  std::vector<int64_t> long_data {};
  result.column(3).read(long_data, 1, 2);

  // Then
  BOOST_CHECK_EQUAL(long_data[0], 123);
  BOOST_CHECK_EQUAL(long_data[1], 123456789);

  // When
  std::vector<float> float_data {};
  result.column(4).read(float_data, 1, 2);

  // Then
  BOOST_CHECK_EQUAL(float_data[0], 0.F);
  BOOST_CHECK_EQUAL(float_data[1], 2.3e-2F);

  // When
  std::vector<double> double_data {};
  result.column(5).read(double_data, 1, 2);

  // Then
  BOOST_CHECK_EQUAL(double_data[0], 0.F);
  BOOST_CHECK_CLOSE(double_data[1], 1.12345e-18, 0.001);

  // When
  std::vector<std::string> string_data {};
  result.column(6).read(string_data, 1, 2);

  // Then
  BOOST_CHECK_EQUAL(string_data[0], "first");
  BOOST_CHECK_EQUAL(string_data[1], "second");

}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END ()


