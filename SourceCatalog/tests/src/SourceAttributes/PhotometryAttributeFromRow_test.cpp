/**
 * @file tests/src/SourceAttributes/PhotometryAttributeFromRow_test.cpp
 *
 * @date Apr 17, 2014
 * @author Pierre Dubath
 */

#include <boost/test/unit_test.hpp>
#include <memory>
#include <map>
#include "SourceCatalog/SourceAttributes/PhotometryAttributeFromRow.h"

#include "../../../SourceCatalog/PhotometryParsingException.h"
#include "SourceCatalog/SourceAttributes/Photometry.h"
#include "ElementsKernel/Exception.h"
#include "SourceCatalog/AttributeFromRow.h"
#include "tests/src/TableFixture.h"

using namespace Euclid::SourceCatalog;
using namespace std;


//-----------------------------------------------------------------------------
//

BOOST_AUTO_TEST_SUITE (PhotometryAttributeFromRow_test)

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

BOOST_FIXTURE_TEST_CASE( createAttribute_test, TableFixture ) {

  BOOST_TEST_MESSAGE("--> createAttribute test ");

  PhotometryAttributeFromRow paft {column_info_ptr, filter_name_mapping, true, 1.12345e-12, true};

  unique_ptr<Euclid::SourceCatalog::Attribute> attribute_ptr = paft.createAttribute(row1);

  BOOST_CHECK( typeid(*attribute_ptr) == typeid(Euclid::SourceCatalog::Photometry) );

  if(typeid(*attribute_ptr) == typeid(Euclid::SourceCatalog::Photometry)) {
      Euclid::SourceCatalog::Photometry& photometry = dynamic_cast<Euclid::SourceCatalog::Photometry&>( *attribute_ptr );
      BOOST_CHECK_CLOSE(photometry.find(v_filter_name)->flux, flux1_row1, tolerance);
      BOOST_CHECK_CLOSE(photometry.find(v_filter_name)->error, 0, tolerance); // When the missing photometry is set the error is 0
      BOOST_CHECK(photometry.find(v_filter_name)->missing_photometry_flag);
      BOOST_CHECK_CLOSE(photometry.find(r_filter_name)->flux, flux2_row1, tolerance);
      BOOST_CHECK_CLOSE(photometry.find(r_filter_name)->error, error2_row1, tolerance);
      BOOST_CHECK(!photometry.find(r_filter_name)->missing_photometry_flag);
  }
}

BOOST_FIXTURE_TEST_CASE( exceptionalCaseNoMissingNoUpper_test, TableFixture ) {

  BOOST_TEST_MESSAGE("--> No Missing Data, No Upper Limit ");
  PhotometryAttributeFromRow paft_nn {column_info_ptr, filter_name_mapping, false, -99., false};

  // nominal: read as it, no flags
   unique_ptr<Euclid::SourceCatalog::Attribute> attribute_nn_nom_ptr = paft_nn.createAttribute(row_nominal_case);
   BOOST_CHECK( typeid(*attribute_nn_nom_ptr) == typeid(Euclid::SourceCatalog::Photometry) );
   if(typeid(*attribute_nn_nom_ptr) == typeid(Euclid::SourceCatalog::Photometry)) {
       Euclid::SourceCatalog::Photometry& photometry = dynamic_cast<Euclid::SourceCatalog::Photometry&>( *attribute_nn_nom_ptr );
       BOOST_CHECK_CLOSE(photometry.find(v_filter_name)->flux, 1., tolerance);
       BOOST_CHECK_CLOSE(photometry.find(v_filter_name)->error, 0.5, tolerance);
       BOOST_CHECK(!photometry.find(v_filter_name)->missing_photometry_flag);
       BOOST_CHECK(!photometry.find(v_filter_name)->upper_limit_flag);
   }
  // flux zero: read as it, no flags
     unique_ptr<Euclid::SourceCatalog::Attribute> attribute_nn_f0_ptr = paft_nn.createAttribute(row_zero_flux);
     BOOST_CHECK( typeid(*attribute_nn_f0_ptr) == typeid(Euclid::SourceCatalog::Photometry) );
     if(typeid(*attribute_nn_f0_ptr) == typeid(Euclid::SourceCatalog::Photometry)) {
         Euclid::SourceCatalog::Photometry& photometry = dynamic_cast<Euclid::SourceCatalog::Photometry&>( *attribute_nn_f0_ptr );
         BOOST_CHECK_CLOSE(photometry.find(v_filter_name)->flux, 0., tolerance);
         BOOST_CHECK_CLOSE(photometry.find(v_filter_name)->error, 0.5, tolerance);
         BOOST_CHECK(!photometry.find(v_filter_name)->missing_photometry_flag);
         BOOST_CHECK(!photometry.find(v_filter_name)->upper_limit_flag);
   }
  // flux negatif: read as it, no flags
  unique_ptr<Euclid::SourceCatalog::Attribute> attribute_nn_fm_ptr = paft_nn.createAttribute(row_flag_flux);
  BOOST_CHECK( typeid(*attribute_nn_fm_ptr) == typeid(Euclid::SourceCatalog::Photometry) );
  if(typeid(*attribute_nn_fm_ptr) == typeid(Euclid::SourceCatalog::Photometry)) {
      Euclid::SourceCatalog::Photometry& photometry = dynamic_cast<Euclid::SourceCatalog::Photometry&>( *attribute_nn_fm_ptr );
      BOOST_CHECK_CLOSE(photometry.find(v_filter_name)->flux, -99., tolerance);
      BOOST_CHECK_CLOSE(photometry.find(v_filter_name)->error, 0.5, tolerance);
      BOOST_CHECK(!photometry.find(v_filter_name)->missing_photometry_flag);
      BOOST_CHECK(!photometry.find(v_filter_name)->upper_limit_flag);
  }

  // error 0: error
  BOOST_CHECK_THROW( paft_nn.createAttribute(row_zero_error),PhotometryParsingException);
  // error negatif: error
  BOOST_CHECK_THROW( paft_nn.createAttribute(row_neg_error),PhotometryParsingException);

}


BOOST_FIXTURE_TEST_CASE( exceptionalCaseMissingNoUpper_test, TableFixture ) {




  BOOST_TEST_MESSAGE("--> Missing Data, No Upper Limit ");
  PhotometryAttributeFromRow paft_yn {column_info_ptr, filter_name_mapping, true, -99., false};
  // nominal: read as it, no flags
  unique_ptr<Euclid::SourceCatalog::Attribute> attribute_yn_nom_ptr = paft_yn.createAttribute(row_nominal_case);
  BOOST_CHECK( typeid(*attribute_yn_nom_ptr) == typeid(Euclid::SourceCatalog::Photometry) );
  if(typeid(*attribute_yn_nom_ptr) == typeid(Euclid::SourceCatalog::Photometry)) {
         Euclid::SourceCatalog::Photometry& photometry = dynamic_cast<Euclid::SourceCatalog::Photometry&>( *attribute_yn_nom_ptr );
         BOOST_CHECK_CLOSE(photometry.find(v_filter_name)->flux, 1., tolerance);
         BOOST_CHECK_CLOSE(photometry.find(v_filter_name)->error, 0.5, tolerance);
         BOOST_CHECK(!photometry.find(v_filter_name)->missing_photometry_flag);
         BOOST_CHECK(!photometry.find(v_filter_name)->upper_limit_flag);
  }
  // flux zero: read as it, no flags
  unique_ptr<Euclid::SourceCatalog::Attribute> attribute_yn_f0_ptr = paft_yn.createAttribute(row_zero_flux);
  BOOST_CHECK( typeid(*attribute_yn_f0_ptr) == typeid(Euclid::SourceCatalog::Photometry) );
  if(typeid(*attribute_yn_f0_ptr) == typeid(Euclid::SourceCatalog::Photometry)) {
           Euclid::SourceCatalog::Photometry& photometry = dynamic_cast<Euclid::SourceCatalog::Photometry&>( *attribute_yn_f0_ptr );
           BOOST_CHECK_CLOSE(photometry.find(v_filter_name)->flux, 0., tolerance);
           BOOST_CHECK_CLOSE(photometry.find(v_filter_name)->error, 0.5, tolerance);
           BOOST_CHECK(!photometry.find(v_filter_name)->missing_photometry_flag);
           BOOST_CHECK(!photometry.find(v_filter_name)->upper_limit_flag);
  }
  // flux negatif: read as it, no flags
  unique_ptr<Euclid::SourceCatalog::Attribute> attribute_yn_fm_ptr = paft_yn.createAttribute(row_neg_flux);
  BOOST_CHECK( typeid(*attribute_yn_fm_ptr) == typeid(Euclid::SourceCatalog::Photometry) );
  if(typeid(*attribute_yn_fm_ptr) == typeid(Euclid::SourceCatalog::Photometry)) {
        Euclid::SourceCatalog::Photometry& photometry = dynamic_cast<Euclid::SourceCatalog::Photometry&>( *attribute_yn_fm_ptr );
        BOOST_CHECK_CLOSE(photometry.find(v_filter_name)->flux, -1., tolerance);
        BOOST_CHECK_CLOSE(photometry.find(v_filter_name)->error, 0.5, tolerance);
        BOOST_CHECK(!photometry.find(v_filter_name)->missing_photometry_flag);
        BOOST_CHECK(!photometry.find(v_filter_name)->upper_limit_flag);
  }

  // flux =Flag: Missing photometry Flag Error=0
  unique_ptr<Euclid::SourceCatalog::Attribute> attribute_yn_fF_ptr = paft_yn.createAttribute(row_flag_flux);
  BOOST_CHECK( typeid(*attribute_yn_fF_ptr) == typeid(Euclid::SourceCatalog::Photometry) );
  if(typeid(*attribute_yn_fF_ptr) == typeid(Euclid::SourceCatalog::Photometry)) {
        Euclid::SourceCatalog::Photometry& photometry = dynamic_cast<Euclid::SourceCatalog::Photometry&>( *attribute_yn_fF_ptr );
        BOOST_CHECK_CLOSE(photometry.find(v_filter_name)->error, 0., tolerance);
        BOOST_CHECK(photometry.find(v_filter_name)->missing_photometry_flag);
        BOOST_CHECK(!photometry.find(v_filter_name)->upper_limit_flag);
  }
  // flux =Flag error<0: Missing photometry Flag Error=0
   unique_ptr<Euclid::SourceCatalog::Attribute> attribute_yn_fFem_ptr = paft_yn.createAttribute(row_flag_flux_neg_error);
   BOOST_CHECK( typeid(*attribute_yn_fFem_ptr) == typeid(Euclid::SourceCatalog::Photometry) );
   if(typeid(*attribute_yn_fFem_ptr) == typeid(Euclid::SourceCatalog::Photometry)) {
         Euclid::SourceCatalog::Photometry& photometry = dynamic_cast<Euclid::SourceCatalog::Photometry&>( *attribute_yn_fFem_ptr );
         BOOST_CHECK_CLOSE(photometry.find(v_filter_name)->error, 0., tolerance);
         BOOST_CHECK(photometry.find(v_filter_name)->missing_photometry_flag);
         BOOST_CHECK(!photometry.find(v_filter_name)->upper_limit_flag);
   }
   BOOST_TEST_MESSAGE("--> Error <= 0");
   // error 0: error
   BOOST_CHECK_THROW( paft_yn.createAttribute(row_zero_error),PhotometryParsingException);
   // error negatif: error
   BOOST_CHECK_THROW( paft_yn.createAttribute(row_neg_error),PhotometryParsingException);
}




BOOST_FIXTURE_TEST_CASE( exceptionalCaseNoMissingUpper_test, TableFixture ) {
  BOOST_TEST_MESSAGE("--> No Missing Data, Upper Limit ");
  PhotometryAttributeFromRow paft_ny {column_info_ptr, filter_name_mapping, false, -99., true};
  // nominal: read as it, no flags
  unique_ptr<Euclid::SourceCatalog::Attribute> attribute_ny_nom_ptr = paft_ny.createAttribute(row_nominal_case);
  BOOST_CHECK( typeid(*attribute_ny_nom_ptr) == typeid(Euclid::SourceCatalog::Photometry) );
  if(typeid(*attribute_ny_nom_ptr) == typeid(Euclid::SourceCatalog::Photometry)) {
         Euclid::SourceCatalog::Photometry& photometry = dynamic_cast<Euclid::SourceCatalog::Photometry&>( *attribute_ny_nom_ptr );
         BOOST_CHECK_CLOSE(photometry.find(v_filter_name)->flux, 1., tolerance);
         BOOST_CHECK_CLOSE(photometry.find(v_filter_name)->error, 0.5, tolerance);
         BOOST_CHECK(!photometry.find(v_filter_name)->missing_photometry_flag);
         BOOST_CHECK(!photometry.find(v_filter_name)->upper_limit_flag);
  }
  // flux zero: read as it, no flags
  unique_ptr<Euclid::SourceCatalog::Attribute> attribute_ny_f0_ptr = paft_ny.createAttribute(row_zero_flux);
  BOOST_CHECK( typeid(*attribute_ny_f0_ptr) == typeid(Euclid::SourceCatalog::Photometry) );
  if(typeid(*attribute_ny_f0_ptr) == typeid(Euclid::SourceCatalog::Photometry)) {
           Euclid::SourceCatalog::Photometry& photometry = dynamic_cast<Euclid::SourceCatalog::Photometry&>( *attribute_ny_f0_ptr );
           BOOST_CHECK_CLOSE(photometry.find(v_filter_name)->flux, 0., tolerance);
           BOOST_CHECK_CLOSE(photometry.find(v_filter_name)->error, 0.5, tolerance);
           BOOST_CHECK(!photometry.find(v_filter_name)->missing_photometry_flag);
           BOOST_CHECK(!photometry.find(v_filter_name)->upper_limit_flag);
  }
  // flux negatif: read as it, no flags
  unique_ptr<Euclid::SourceCatalog::Attribute> attribute_ny_fm_ptr = paft_ny.createAttribute(row_neg_flux);
  BOOST_CHECK( typeid(*attribute_ny_fm_ptr) == typeid(Euclid::SourceCatalog::Photometry) );
  if(typeid(*attribute_ny_fm_ptr) == typeid(Euclid::SourceCatalog::Photometry)) {
        Euclid::SourceCatalog::Photometry& photometry = dynamic_cast<Euclid::SourceCatalog::Photometry&>( *attribute_ny_fm_ptr );
        BOOST_CHECK_CLOSE(photometry.find(v_filter_name)->flux, -1., tolerance);
        BOOST_CHECK_CLOSE(photometry.find(v_filter_name)->error, 0.5, tolerance);
        BOOST_CHECK(!photometry.find(v_filter_name)->missing_photometry_flag);
        BOOST_CHECK(!photometry.find(v_filter_name)->upper_limit_flag);
  }
  // Error==0: error
  BOOST_CHECK_THROW( paft_ny.createAttribute(row_zero_error),PhotometryParsingException);
  // Error<0 Flux<0: error
  BOOST_CHECK_THROW( paft_ny.createAttribute(row_flag_flux_neg_error),PhotometryParsingException);
  // Error<0 Flux=0: error
  BOOST_CHECK_THROW( paft_ny.createAttribute(row_zero_flux_neg_error),PhotometryParsingException);

  // error negatif: flag as upper limit Error = |Error|
   unique_ptr<Euclid::SourceCatalog::Attribute> attribute_ny_em_ptr = paft_ny.createAttribute(row_neg_error);
   BOOST_CHECK( typeid(*attribute_ny_em_ptr) == typeid(Euclid::SourceCatalog::Photometry) );
   if(typeid(*attribute_ny_em_ptr) == typeid(Euclid::SourceCatalog::Photometry)) {
         Euclid::SourceCatalog::Photometry& photometry = dynamic_cast<Euclid::SourceCatalog::Photometry&>( *attribute_ny_em_ptr );
         BOOST_CHECK_CLOSE(photometry.find(v_filter_name)->flux, 1., tolerance);
         BOOST_CHECK_CLOSE(photometry.find(v_filter_name)->error, 0.5, tolerance);
         BOOST_CHECK(!photometry.find(v_filter_name)->missing_photometry_flag);
         BOOST_CHECK(photometry.find(v_filter_name)->upper_limit_flag);
   }

}

BOOST_FIXTURE_TEST_CASE( exceptionalCaseMissingUpper_test, TableFixture ) {
  BOOST_TEST_MESSAGE("-->Missing Data, Upper Limit ");
  PhotometryAttributeFromRow paft_yy {column_info_ptr, filter_name_mapping, true, -99., true};
  // nominal: read as it, no flags
  unique_ptr<Euclid::SourceCatalog::Attribute> attribute_yy_nom_ptr = paft_yy.createAttribute(row_nominal_case);
  BOOST_CHECK( typeid(*attribute_yy_nom_ptr) == typeid(Euclid::SourceCatalog::Photometry) );
  if(typeid(*attribute_yy_nom_ptr) == typeid(Euclid::SourceCatalog::Photometry)) {
         Euclid::SourceCatalog::Photometry& photometry = dynamic_cast<Euclid::SourceCatalog::Photometry&>( *attribute_yy_nom_ptr );
         BOOST_CHECK_CLOSE(photometry.find(v_filter_name)->flux, 1., tolerance);
         BOOST_CHECK_CLOSE(photometry.find(v_filter_name)->error, 0.5, tolerance);
         BOOST_CHECK(!photometry.find(v_filter_name)->missing_photometry_flag);
         BOOST_CHECK(!photometry.find(v_filter_name)->upper_limit_flag);
  }
  // flux zero: read as it, no flags
  unique_ptr<Euclid::SourceCatalog::Attribute> attribute_yy_f0_ptr = paft_yy.createAttribute(row_zero_flux);
  BOOST_CHECK( typeid(*attribute_yy_f0_ptr) == typeid(Euclid::SourceCatalog::Photometry) );
  if(typeid(*attribute_yy_f0_ptr) == typeid(Euclid::SourceCatalog::Photometry)) {
           Euclid::SourceCatalog::Photometry& photometry = dynamic_cast<Euclid::SourceCatalog::Photometry&>( *attribute_yy_f0_ptr );
           BOOST_CHECK_CLOSE(photometry.find(v_filter_name)->flux, 0., tolerance);
           BOOST_CHECK_CLOSE(photometry.find(v_filter_name)->error, 0.5, tolerance);
           BOOST_CHECK(!photometry.find(v_filter_name)->missing_photometry_flag);
           BOOST_CHECK(!photometry.find(v_filter_name)->upper_limit_flag);
  }
  // flux negatif: read as it, no flags
  unique_ptr<Euclid::SourceCatalog::Attribute> attribute_yy_fm_ptr = paft_yy.createAttribute(row_neg_flux);
  BOOST_CHECK( typeid(*attribute_yy_fm_ptr) == typeid(Euclid::SourceCatalog::Photometry) );
  if(typeid(*attribute_yy_fm_ptr) == typeid(Euclid::SourceCatalog::Photometry)) {
        Euclid::SourceCatalog::Photometry& photometry = dynamic_cast<Euclid::SourceCatalog::Photometry&>( *attribute_yy_fm_ptr );
        BOOST_CHECK_CLOSE(photometry.find(v_filter_name)->flux, -1., tolerance);
        BOOST_CHECK_CLOSE(photometry.find(v_filter_name)->error, 0.5, tolerance);
        BOOST_CHECK(!photometry.find(v_filter_name)->missing_photometry_flag);
        BOOST_CHECK(!photometry.find(v_filter_name)->upper_limit_flag);
  }

  // Error==0: error
  BOOST_CHECK_THROW( paft_yy.createAttribute(row_zero_error),PhotometryParsingException);

  // flux =Flag: Missing photometry Flag Error=0
   unique_ptr<Euclid::SourceCatalog::Attribute> attribute_yy_fF_ptr = paft_yy.createAttribute(row_flag_flux);
   BOOST_CHECK( typeid(*attribute_yy_fF_ptr) == typeid(Euclid::SourceCatalog::Photometry) );
   if(typeid(*attribute_yy_fF_ptr) == typeid(Euclid::SourceCatalog::Photometry)) {
         Euclid::SourceCatalog::Photometry& photometry = dynamic_cast<Euclid::SourceCatalog::Photometry&>( *attribute_yy_fF_ptr );
         BOOST_CHECK_CLOSE(photometry.find(v_filter_name)->error, 0., tolerance);
         BOOST_CHECK(photometry.find(v_filter_name)->missing_photometry_flag);
         BOOST_CHECK(!photometry.find(v_filter_name)->upper_limit_flag);
   }

   // flux = Flag error<0: Missing photometry Flag Error=0
    unique_ptr<Euclid::SourceCatalog::Attribute> attribute_yy_fFem_ptr = paft_yy.createAttribute(row_flag_flux_neg_error);
    BOOST_CHECK( typeid(*attribute_yy_fFem_ptr) == typeid(Euclid::SourceCatalog::Photometry) );
    if(typeid(*attribute_yy_fFem_ptr) == typeid(Euclid::SourceCatalog::Photometry)) {
          Euclid::SourceCatalog::Photometry& photometry = dynamic_cast<Euclid::SourceCatalog::Photometry&>( *attribute_yy_fFem_ptr );
          BOOST_CHECK_CLOSE(photometry.find(v_filter_name)->error, 0., tolerance);
          BOOST_CHECK(photometry.find(v_filter_name)->missing_photometry_flag);
          BOOST_CHECK(!photometry.find(v_filter_name)->upper_limit_flag);
    }

    // error negatif: flag as upper limit Error = |Error|
    unique_ptr<Euclid::SourceCatalog::Attribute> attribute_yy_em_ptr = paft_yy.createAttribute(row_neg_error);
    BOOST_CHECK( typeid(*attribute_yy_em_ptr) == typeid(Euclid::SourceCatalog::Photometry) );
    if(typeid(*attribute_yy_em_ptr) == typeid(Euclid::SourceCatalog::Photometry)) {
            Euclid::SourceCatalog::Photometry& photometry = dynamic_cast<Euclid::SourceCatalog::Photometry&>( *attribute_yy_em_ptr );
            BOOST_CHECK_CLOSE(photometry.find(v_filter_name)->flux, 1., tolerance);
            BOOST_CHECK_CLOSE(photometry.find(v_filter_name)->error, 0.5, tolerance);
            BOOST_CHECK(!photometry.find(v_filter_name)->missing_photometry_flag);
            BOOST_CHECK(photometry.find(v_filter_name)->upper_limit_flag);
    }

    // Error<0 Flux=0: error
    BOOST_CHECK_THROW( paft_yy.createAttribute(row_zero_flux_neg_error),PhotometryParsingException);
    // Error<0 Flux<0: error
    BOOST_CHECK_THROW( paft_yy.createAttribute(row_neg_flux_neg_error),PhotometryParsingException);



}



//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END ()


