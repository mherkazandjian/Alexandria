/**
 * @file XYDataset/AsciiParser.h
 *
 * @date Apr 14, 2014
 * @author Nicolas Morisset
 */

#ifndef ASCIIPARSER_H_
#define ASCIIPARSER_H_

#include "ElementsKernel/Export.h"

#include "XYDataset/XYDataset.h"
#include "XYDataset/FileParser.h"


namespace Euclid {
namespace XYDataset {

/**
 * @class AsciiParser
 *
 * @brief
 * Tool for reading ASCII tables from streams
 * @details
 * The AsciiParser reads ASCII files which contain space or tab separated
 * tables of two columns. The first column contains the X data and the second
 * the Y data. The name of the dataset is extracted from the first non-empty
 * line of the file, as the first match of a regular expression. If the regular
 * expression does not match, the name of the file (excluding the extension)
 * is used as the name of the dataset. Comments are supported by using the "#"
 * character.
 * @throw
 * ElementException : File not found
 */
class ELEMENTS_API AsciiParser : public FileParser
{
 public:

  /**
   * @brief Constructor
   * Tool for reading ASCII tables from streams
   * @param regex_str
   * The regex for extracting the dataset name. The default is defined as
   * "^\\s*#\\s*(\\w+)\\s*$".
   */
  AsciiParser(const std::string& regex_str="^\\s*#\\s*(\\w+)\\s*$") : FileParser(), m_regex_name(regex_str) {}

  /**
   * @brief
   * Get the dataset name of a ASCII file
   * @details
   * This function gets the name of a dataset.The datatset name could be
   * the filename or is included in the file. If the dataset is not included
   * into the file, the filename itself is the dataset name removing the path
   * and the extension. If the dataset name is inside the file the function
   * looks for a line starting with a "#" character and extract the word after
   * the hash
   * @param file
   * Filename of the file to be read including absolute path
   * @return
   * The name of the datatset
   * @throw
   * ElementException : File not found
   */
  std::string getName(const std::string& file) override;

  /**
   * @brief
   * Get a XYDataset object reading data from an ASCII file
   * @details
   * Read an ASCII file and put the data into a XYDataset object. It makes the
   * assumption that the file only contains two columns of double values
   * @param file
   * Filename of the file to be read including the absolute path
   * @return
   * A unique pointer to a XYDatatset object or null pointer.
   */
  std::unique_ptr<XYDataset> getDataset(const std::string& file) override;

  /**
    * @brief
    * Check that the ASCII file is a dataset file(with at least one line with
    * 2 double values)
    * @details
    * This checking should avoid reading any files which do not contain
    * any dataset.
    * @param file
    * Filename of the FITS file to be read including the absolute path.
    * @return
    * true if it is a FITS file with dataset(at least with one HDU table)
    */
  bool isDatasetFile(const std::string& file) override;

  /// Default destructor
  virtual ~AsciiParser() = default;

 private:

  std::string m_regex_name;

};

} /* namespace XYDataset */
} // end of namespace Euclid



#endif // ASCIIPARSER_H_