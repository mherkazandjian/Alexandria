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
 * @file src/lib/PhotometryCatalogConfig.cpp
 * @date 11/06/15
 * @author nikoapos
 */

#include <utility>
#include <vector>
#include <fstream>
#include <sstream>
#include <boost/regex.hpp>
using boost::regex;
using boost::regex_match;
using boost::smatch;
#include <boost/algorithm/string.hpp>
#include "ElementsKernel/Exception.h"
#include "ElementsKernel/Logging.h"
#include "SourceCatalog/SourceAttributes/PhotometryAttributeFromRow.h"
#include "Configuration/CatalogConfig.h"
#include "Configuration/PhotometryCatalogConfig.h"

namespace po = boost::program_options;
namespace fs = boost::filesystem;

namespace Euclid {
namespace Configuration {

static Elements::Logging logger = Elements::Logging::getLogger("PhotometryCatalogConfig");

static const std::string FILTER_MAPPING_FILE {"filter-mapping-file"};
static const std::string EXCLUDE_FILTER {"exclude-filter"};
static const std::string MISSING_PHOTOMETRY_FLAG {"missing-photometry-flag"};

PhotometryCatalogConfig::PhotometryCatalogConfig(long manager_id) : Configuration(manager_id) {
  declareDependency<CatalogConfig>();
}

auto PhotometryCatalogConfig::getProgramOptions() -> std::map<std::string, OptionDescriptionList> {
  return {{"Input catalog options", {
    {FILTER_MAPPING_FILE.c_str(), po::value<std::string>()->default_value("filter_mapping.txt"),
        "The file containing the photometry mapping of the catalog columns"},
    {EXCLUDE_FILTER.c_str(), po::value<std::vector<std::string>>()->default_value(std::vector<std::string>{}, ""),
        "A list of filters to ignore"},
    {MISSING_PHOTOMETRY_FLAG.c_str(), po::value<double>()->default_value(-99.),
        "It is a flag value for missing flux for a source"}
  }}};
}

static fs::path getMappingFileFromOptions(const Configuration::UserValues& args,
                                          const fs::path& base_dir) {
  fs::path mapping_file {args.at(FILTER_MAPPING_FILE).as<std::string>()};
  if (mapping_file.is_relative()) {
    mapping_file = base_dir / mapping_file;
  }
  if (!fs::exists(mapping_file)) {
    throw Elements::Exception() << "Photometry mapping file " << mapping_file << " does not exist";
  }
  if (fs::is_directory(mapping_file)) {
    throw Elements::Exception() << "Photometry mapping file " << mapping_file << " is not a file";
  }
  return mapping_file;
}

static std::vector<std::pair<std::string, std::pair<std::string, std::string>>> parseFile(fs::path filename) {
  std::vector<std::pair<std::string, std::pair<std::string, std::string>>> filter_name_mapping {};
  std::ifstream in {filename.string()};
  std::string line;
  regex expr {"\\s*([^\\s#]+)\\s+([^\\s#]+)\\s+([^\\s#]+)\\s*(#.*)?"};
  while (std::getline(in, line)) {
    boost::trim(line);
    if (line[0] == '#') {
      continue;
    }
    smatch match_res;
    if (!regex_match(line, match_res, expr)) {
      logger.error() << "Syntax error in " << filename << ": " << line;
      throw Elements::Exception() << "Syntax error in " << filename << ": " << line;
    }
    filter_name_mapping.emplace_back(match_res.str(1), std::make_pair(match_res.str(2), match_res.str(3)));
  }
  return filter_name_mapping;
}

void PhotometryCatalogConfig::initialize(const UserValues& args) {
  
  // Parse the file with the mapping
  auto filename = getMappingFileFromOptions(args, m_base_dir);
  auto all_filter_name_mapping = parseFile(filename);
  
  // Remove the filters which are marked to exclude
  std::vector<std::pair<std::string, std::pair<std::string, std::string>>> filter_name_mapping {};
  auto exclude_vector = args.at(EXCLUDE_FILTER).as<std::vector<std::string>>();
  std::set<std::string> exclude_filters {exclude_vector.begin(), exclude_vector.end()};
  for (auto& pair : all_filter_name_mapping) {
    if (exclude_filters.count(pair.first) > 0) {
      exclude_filters.erase(pair.first);
    } else {
      filter_name_mapping.push_back(pair);
    }
  }

  if (!exclude_filters.empty()) {
    std::stringstream wrong_filters {};
    for (auto& f : exclude_filters) {
      wrong_filters << f << " ";
    }
    throw Elements::Exception() << "Wrong " << EXCLUDE_FILTER << " option value(s) : " << wrong_filters.str();
  }
  
  // Get the flag
  double missing_photo_flag = args.at(MISSING_PHOTOMETRY_FLAG).as<double>();
  
  // Keep a list of the filters to process
  m_photometric_bands.clear();
  for (auto& pair : filter_name_mapping) {
    m_photometric_bands.push_back(pair.first);
  }

  auto column_info = getDependency<CatalogConfig>().getAsTable().getColumnInfo();
  
  // Add the row handler to parse the photometries
  std::shared_ptr<SourceCatalog::AttributeFromRow> handler_ptr {
    new SourceCatalog::PhotometryAttributeFromRow {column_info,
                                                   std::move(filter_name_mapping),
                                                   missing_photo_flag}
  };
  getDependency<CatalogConfig>().addAttributeHandler(std::move(handler_ptr));
}

void PhotometryCatalogConfig::setBaseDir(const boost::filesystem::path& base_dir) {
  m_base_dir = base_dir;
}

const std::vector<std::string>& PhotometryCatalogConfig::getPhotometricBands() {
  return m_photometric_bands;
}

} // Configuration namespace
} // Euclid namespace



