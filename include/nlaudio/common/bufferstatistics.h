#pragma once

// Input Output forward declaration
#include <iosfwd>
#include "common/json.hpp"

namespace Nl {

/** \ingroup Audio
 * \struct BufferStatistics - Stores information on buffer access and un-/overflow counts.
 *
 * This struct can be printed using operator<< to std::out
 *
*/
struct BufferStatistics {
	unsigned long bytesReadFromBuffer; ///< Number of bytes that have been read from the buffer
	unsigned long bytesWrittenToBuffer; ///< Number of bytes that have been written to the buffer
	unsigned int xrunCount; ///< Number of over-/underflows
};

void to_json(nlohmann::json& j, const BufferStatistics& s);
void from_json(const nlohmann::json& j, BufferStatistics& s);

std::ostream& operator<<(std::ostream& lhs, const BufferStatistics& rhs);
nlohmann::json toJSON(const BufferStatistics& ss);

} // namespace Nl
