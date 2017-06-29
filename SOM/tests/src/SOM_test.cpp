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
 * @file tests/src/SOM_test.cpp
 * @date 06/21/17
 * @author nikoapos
 */

#include <boost/test/unit_test.hpp>

#include "SOM/SOM.h"

using namespace Euclid::SOM;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE (SOM_test)

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE( example_test ) {

  BOOST_FAIL("!!!! Please implement your tests !!!!");
  
  SOM<2> som {2, 3};
  som.findBMU({1,2});

}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END ()


