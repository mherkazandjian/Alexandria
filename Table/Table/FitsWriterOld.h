/**
 * @file Table/FitsWriter.h
 * @date April 23, 2014
 * @author Nikolaos Apostolakos
 */

#ifndef TABLE_FITSWRITER_H
#define	TABLE_FITSWRITER_H

#include <CCfits/CCfits>
#include <string>

#include "ElementsKernel/Export.h"

#include "Table/Table.h"

namespace Euclid {
namespace Table {

/**
 * @class FitsWriter
 *
 * @brief Tool for writing tables in FITS format
 *
 * @details
 * The FitsWriter class is a tool for writing Table objects in FITS format. It
 * can be parameterized with the arguments of its constructor and it provides
 * write() methods for writing the tables.
 */
class ELEMENTS_API FitsWriter {

public:

  /// The format of the HDUs a FitsWriter creates
  enum class Format {
    /// FITS ASCII table HDU format
    ASCII,
    /// FITS binary table HDU format
    BINARY
  };

  /**
   * @brief
   * Constructs a new FitsWriter
   *
   * @param format The output HDU format
   */
  FitsWriter(Format format = Format::BINARY);

  /// Default destructor
  virtual ~FitsWriter() = default;

  /**
   * @brief
   * Adds a table HDU to a FITS object
   * @details
   * The format of the columns of the table depends on the format the FitsWriter
   * is initialized for. The conversions are the following:
   *
   * ASCII format:
   * - bool<br>
   *   I1, where true is represented as 1 and false is represented as 0
   * - int32_t, int64_t<br>
   *   Iw, where w is the length required for the data of the column
   * - float, double<br>
   *   E12 (at the moment the length is is not modifiable)<br>
   * - std::string<br>
   *   Aw, where w is the length required for the data of the column
   *
   * Binary format:
   * - bool : L
   * - int32_t : J
   * - int64_t : K
   * - float : E
   * - double : D
   * - std::string : wA, where w is the length required for the data of the column
   * - vector<int32_t> : wJ, where w is the length of the vector
   * - vector<int64_t> : wK, where w is the length of the vector
   * - vector<float> : wE, where w is the length of the vector
   * - vector<double> : wD, where w is the length of the vector
   * 
   * Note that, at the moment, only fixed length vector columns are supported
   * and that there is no support for vector columns for ASCII FITS tables.
   * 
   * The TUNITn fits keywords are populated using the unit of the of the
   * ColumnDescriptions of the Table. The descriptions of the columns are
   * set as the values of the (non standard) keywords TDESCn.
   *
   * @param fits The FITS object to add the table HDU
   * @param hdu_name The name of the HDU
   * @param table The table to output
   * @param the comments to add to the table HDU
   */
  void write(CCfits::FITS& fits, const std::string& hdu_name, const Table& table,
             const std::vector<std::string>& comments={}) const;

private:
  Format m_format;

};

}
} // end of namespace Euclid

#endif	/* TABLE_FITSWRITER_H */
