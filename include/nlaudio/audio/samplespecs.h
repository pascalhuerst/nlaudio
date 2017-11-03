#pragma once

#include <iosfwd>
#include <limits>
#include <stdint.h>
#include "common/json.hpp"

namespace Nl {

/** \ingroup Audio
 * \struct SampleSpecs - Stores Sampleinformation for an audio interface
 *
*/
struct SampleSpecs {
    unsigned int samplerate;				///< Samplerate in Hz
    unsigned int channels;					///< Channels
    unsigned int bytesPerSample;			///< How many bytes does one sample have. 24_BE3 = 3, S16 = 2, ...
    unsigned int bytesPerSamplePhysical;	///< Sometimes 24_BE3 can be stored in 4Bytes, then this would be 4. Usually same as bytesPerSample
    unsigned int bytesPerFrame;				///< How many bytes does one frame have. Same as channels * bytesPerSample
    unsigned int buffersizeInFrames;		///< Buffersize in Frames
    unsigned int buffersizeInFramesPerPeriode;	///< Buffersize in Frames per Periode. Same as buffersizeInFrames / periodes
    unsigned int buffersizeInBytes;				///< Buffersize in Bytes
    unsigned int buffersizeInBytesPerPeriode;	///< Buffersize in Bytes per Periode. Same as buffersizeInBytes / periodes
    unsigned int buffersizeInSamples;			///< Buffersize in Samples. Same as buffersizeInBytes / bytesPerSample
    unsigned int buffersizeInSamplesPerPeriode; ///< Buffersize in Samples per Periode. Same as buffersizeInSamples / periodes
    bool isFloat;								///< Are we working with floating point samples?
    bool isLittleEndian;						///< Are we working in little endian?
    bool isSigned;								///< Are we using a sample format with signed values?
    double latency;								///< Latency, which is buffersizeInFramesPerPeriode / samplerate
    // bool isInterleaved
};
std::ostream& operator<<(std::ostream& lhs, const SampleSpecs& rhs);
nlohmann::json toJSON(const SampleSpecs& ss);

unsigned int getByteIndex(unsigned int frameIndex, unsigned int channel, unsigned int byte, const SampleSpecs &sampleSpecs);
float getSample(u_int8_t* in, u_int32_t frameIndex, u_int32_t channel, const SampleSpecs& sampleSpecs);
void setSample(u_int8_t* out, float sample, u_int32_t frameIndex, u_int32_t channel, const SampleSpecs& sampleSpecs);
void setSample(u_int8_t* out, int32_t sample, u_int32_t frameIndex, u_int32_t channel, const SampleSpecs& sampleSpecs);

void to_json(nlohmann::json& j, const SampleSpecs& s);
void from_json(const nlohmann::json& j, SampleSpecs& s);

} // namespace Nl
