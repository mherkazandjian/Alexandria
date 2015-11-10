/*  
 * Copyright (C) 2012-2020 Euclid Science Ground Segment    
 *  
 * This library is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General  
 * Public License as published by the Free Software Foundation; either version 3.0 of the License, or (at your option)  
 * any later version.  
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied  
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more  
 * details.  
 *  
 * You should have received a copy of the GNU Lesser General Public License along with this library; if not, write to  
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA  
 */  

/**
 * @file src/lib/ConfigManager.cpp
 * @date 11/05/15
 * @author nikoapos
 */

#include "ElementsKernel/Exception.h"
#include "Configuration/Configuration.h"
#include "Configuration/ConfigManager.h"

namespace po = boost::program_options;

namespace Euclid {
namespace Configuration {

ConfigManager& ConfigManager::getInstance(long id) {
  static std::map<long, std::unique_ptr<ConfigManager>> manager_map {};
  auto& manager_ptr = manager_map[id];
  if (manager_ptr == nullptr) {
    manager_ptr.reset(new ConfigManager{id});
  }
  return *manager_ptr;
}

ConfigManager::ConfigManager(long id) : m_id{id} {
}

std::unique_ptr<std::type_index> hasCircularDependencies(const std::map<std::type_index, std::set<std::type_index>>& dependency_map,
                                                         const std::type_index& root, const std::pair<const std::type_index, std::set<std::type_index>>& config_pair) {
  if (config_pair.second.find(root) != config_pair.second.end()) {
    return std::unique_ptr<std::type_index> {new std::type_index{config_pair.first}};
  }
  for (auto& config : config_pair.second) {
    auto found = hasCircularDependencies(dependency_map, root, *dependency_map.find(config));
    if (found) {
      return found;
    }
  }
  return nullptr;
}

po::options_description ConfigManager::closeRegistration() {
  m_state = State::WAITING_INITIALIZATION;
  
  // Populate the dependencies map
  for (auto& pair : m_config_dictionary) {
    m_dependency_map[pair.first].insert(pair.second->getDependencies().begin(),
                                        pair.second->getDependencies().end());
  }
  // Check for circular dependencies
  for (auto& pair : m_config_dictionary) {
    auto found = hasCircularDependencies(m_dependency_map, pair.first, *m_dependency_map.find(pair.first));
    if (found) {
      throw Elements::Exception() << "Circular dependency between configurations "
          << pair.first.name() << " and " << found->name();
    }
  }
  
  std::map<std::string, po::options_description> all_options {};
  for (auto& config : m_config_dictionary) {
    for (auto& pair : config.second->getProgramOptions()) {
      if (all_options.find(pair.first) == all_options.end()) {
        all_options.emplace(pair.first, po::options_description{pair.first});
      }
      auto& group = all_options.at(pair.first);
      for (auto& option : pair.second) {
        group.add(boost::shared_ptr<po::option_description>{new po::option_description{option}});
      }
    }
  }
  
  po::options_description result {};
  for (auto& pair : all_options) {
    result.add(pair.second);
  }
  
  return result;
}

static void recursiveInitialization(const std::map<std::type_index, std::unique_ptr<Configuration>>& dictionary,
                                    const std::map<std::type_index, std::set<std::type_index>>& dependency_map,
                                    const std::map<std::string, po::variable_value>& user_values,
                                    const std::type_index& config) {
  if (dictionary.at(config)->getCurrentState() >= Configuration::State::INITIALIZED) {
    return;
  }
  
  for (auto& dependency : dependency_map.at(config)) {
    recursiveInitialization(dictionary, dependency_map, user_values, dependency);
  }
  
  dictionary.at(config)->initialize(user_values);
  dictionary.at(config)->getCurrentState() = Configuration::State::INITIALIZED;
}

void ConfigManager::initialize(const std::map<std::string, po::variable_value>& user_values) {
  m_state = State::INITIALIZED;
  for (auto& pair : m_config_dictionary) {
    pair.second->preInitialize(user_values);
    pair.second->getCurrentState() = Configuration::State::PRE_INITIALIZED;
  }
  for (auto& pair : m_config_dictionary) {
    recursiveInitialization(m_config_dictionary, m_dependency_map, user_values, pair.first);
  }
  for (auto& pair : m_config_dictionary) {
    pair.second->postInitialize(user_values);
    pair.second->getCurrentState() = Configuration::State::FINAL;
  }
}

} // Configuration namespace
} // Euclid namespace



