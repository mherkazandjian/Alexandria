/**
 * @file AttributeHandler.h
 *
 * @date Apr 17, 2014
 * @author Pierre Dubath
 */
#ifndef ATTRIBUTEHANDLER_H_
#define ATTRIBUTEHANDLER_H_

#include "ChCatalog/Attribute.h"
#include "ChTable/Row.h"

namespace ChCatalog {

/**
 * @class AttributeHandler
 * @brief Interface for building a source Attribute from a Table
 *
 * @details This interface is defined to build source Attributes from Tables. The rules
 * for formatting the specific Attribute from the general Table columns must be provided
 * in the implementation constructors. They are then defined during object creation only
 * once for the complete Table. The createAttribute method can then be called for each
 * source, i.e., for each row of the Table.
 *
 */
class AttributeHandler {
public:
  virtual ~AttributeHandler() {
  }

  /**
   * @brief The createAttribute method for creating an Attribute from a Table row
   * @details
   * @param A reference to a Row of a Table
   * @return A unique pointer to the newly created Attribute
   */
  virtual std::unique_ptr<Attribute> createAttribute(
      const ChTable::Row& row) = 0;

};

} // namespace ChCatalog 

#endif // ATTRIBUTEHANDLER_H_ 
