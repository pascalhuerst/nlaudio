#include "common/bufferstatistics.h"
#include <ostream>

namespace Nl {

/** \ingroup Audio
 *
 * \brief Print BufferStatistics using operator<<
 * \param lhs Left hand Side
 * \param rhs Right hand Side
 * \return std::ostream with string data
 *
 * Helper overload of operator<< for BufferStatistics
 * so it can be used as:
 *
 * \code{.cpp}
 *	std::cout << "Statistics: " << std::endl << handle->getStats() << std::endl;
 * \endcode
*/
std::ostream& operator<<(std::ostream& lhs, const BufferStatistics& rhs)
{
    lhs << "  Bytes Read From Buffer:   " << rhs.bytesReadFromBuffer << std::endl
        << "  Bytes Written To Buffer:  " << rhs.bytesWrittenToBuffer << std::endl
        << "  Over-/Underrun Count:     " << rhs.xrunCount << std::endl;

    return lhs;
}

void to_json(nlohmann::json& j, const BufferStatistics& s)
{
    j = nlohmann::json{
    { "bytes_read_from_buffer",     s.bytesReadFromBuffer},
    { "bytes_written_to_buffer",    s.bytesWrittenToBuffer},
    { "xrun",                       s.xrunCount}
};
}

void from_json(const nlohmann::json& j, BufferStatistics& s)
{
    s.bytesReadFromBuffer       = j.at("bytes_read_from_buffer").get<unsigned long>();
    s.bytesWrittenToBuffer      = j.at("bytes_written_to_buffer").get<unsigned long>();
    s.xrunCount                 = j.at("xrun").get<unsigned int>();
}

nlohmann::json toJSON(const BufferStatistics& ss)
{
    return ss;
}

} // namespace Nl
