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
 * @file tests/src/SourceAttributes/Photometry_test.cpp
 *
 * Created on: Jan 20, 2014
 *     Author: Pierre Dubath
 */
#include <boost/test/unit_test.hpp>
#include "SourceCatalog/SourceAttributes/Photometry.h"


//-----------------------------------------------------------------------------
// Include the CatalogFixture which comprises a photometry mock object use here
// as a test reference
#include "tests/src/CatalogFixture.h"

using namespace Euclid::SourceCatalog;
using namespace std;

//----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE (Photometry_test)

BOOST_FIXTURE_TEST_CASE( find_test, CatalogFixture ) {

  BOOST_TEST_MESSAGE("--> find test ");

  shared_ptr<FluxErrorPair> ptr1 = photometry.find(expected_filter_name_1);
  shared_ptr<FluxErrorPair> ptr2 = photometry.find(expected_filter_name_2);

  BOOST_CHECK_CLOSE(expected_flux_1, ptr1->flux, tolerence);
  BOOST_CHECK_CLOSE(expected_error_1, ptr1->error, tolerence);
  BOOST_CHECK_CLOSE(expected_flux_2, ptr2->flux, tolerence);
  BOOST_CHECK_CLOSE(expected_error_2, ptr2->error, tolerence);
}

BOOST_FIXTURE_TEST_CASE( not_found_test, CatalogFixture ) {

  BOOST_TEST_MESSAGE("--> not found find test! ");

  unique_ptr<FluxErrorPair> ptr1 = photometry.find("AnotherFilterGroup/AnotherFilterName");
  BOOST_CHECK(ptr1 == nullptr);
}


BOOST_FIXTURE_TEST_CASE( iterator_test, CatalogFixture ) {

  BOOST_TEST_MESSAGE("--> iterator test ");

  // This is to get the expected filter name from the fixture for comparison
  auto expected_filter_name_iter = filter_name_vector_ptr->begin();
  // This is to get the expected fluxes and errors from the fixture for comparison
  auto expected_photo_iter = photometry_vector.begin();

  // loop over the photometry object to check the different filter names, values and errors
  for (auto photo_iter = photometry.begin(); photo_iter != photometry.end(); ++photo_iter) {
    BOOST_CHECK( photo_iter.filterName() ==  *expected_filter_name_iter );
    ++expected_filter_name_iter;
    BOOST_CHECK_CLOSE( (*photo_iter).flux, expected_photo_iter->flux, tolerence);
    BOOST_CHECK_CLOSE( (*photo_iter).error, expected_photo_iter->error, tolerence);
    ++expected_photo_iter;
  }

}

// This is how to iterate through the photometry values in most cases
BOOST_FIXTURE_TEST_CASE( iterator_getter_test, CatalogFixture ) {

  BOOST_TEST_MESSAGE("--> iterator getter test ");

  // This is to get the expected fluxes and errors from the fixture for comparison
  auto expected_photo_iter = photometry_vector.cbegin();

  // loop over the photometry object to check the different values and errors
  for (auto FluxErrorPair :  photometry) {
    BOOST_CHECK_CLOSE( FluxErrorPair.flux, expected_photo_iter->flux, tolerence);
    BOOST_CHECK_CLOSE( FluxErrorPair.error, expected_photo_iter->error, tolerence);
    ++expected_photo_iter;
  }

}


BOOST_AUTO_TEST_SUITE_END ()
