/**
 * @file Source.h
 *
 * @author Pierre Dubath
 *
 * Created on: Jan 31, 2013
 */

#ifndef SOURCE_H_
#define SOURCE_H_

#include <string>
#include <map>

#include "ChDataModel/SourceAttributes/Photometry.h"
#include "ChDataModel/AttributeName.h"
#include "ChDataModel/Attribute.h"

namespace ChDataModel {

/**
 * The Source class include all information related to a sky source
 */
class Source {

public:

  /// Constructor with member assignment
  Source(int64_t source_id, std::map<AttributeName, Attribute> attribute_map) :
      m_source_id(source_id), m_attribute_map(attribute_map) {
  }

  /// Virtual default destructor
  virtual ~Source() {
  }

  int64_t getSourceId() const {
    return m_source_id;
  }

  std::map<AttributeName, Attribute> getAttributeMap() const {
    return m_attribute_map;
  }

private:

  /// Source identification (attributed in the survey)
  const int64_t m_source_id { };

  std::map<AttributeName, Attribute> m_attribute_map;

};
// Eof class Source

} /* namespace ChDataModel */
#endif /* SOURCE_H_ */
