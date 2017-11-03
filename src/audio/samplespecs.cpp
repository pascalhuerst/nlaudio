
#include <iostream>

#include "audio/samplespecs.h"

namespace Nl {

/** \ingroup Tools
 *
 * \brief Returns an index in a bytestream, depending on the input parameters
 * \param frameIndex Index of Frame we are working with
 * \param channel Channel number
 * \param byte Byte of the sample we want to read
 * \param sampleSpecs Sample specification of the current setup
 *
 * Helper function, that returns a index for a bytestream of arbitrary
 * format and channel number
 *
*/
unsigned int getByteIndex(unsigned int frameIndex, unsigned int channel, unsigned int byte, const SampleSpecs &sampleSpecs)
{
    return	// Index of current Frame
            (frameIndex*sampleSpecs.channels*sampleSpecs.bytesPerSample) +
            // Index of current channel in Frame (= Sample)
            (channel*sampleSpecs.bytesPerSample) +
            // Index of current byte in Sample
            byte;
}

// Think about inline here
/** \ingroup Tools
 *
 * \brief Returns a float sample from an audio bytestream of arbitrary format
 * \param in Bytestream of audiodata
 * \param frameIndex Index of the frame we are working with
 * \param channel Channel number
 * \param sampleSpecs Sample specification of the current setup
 *
 * Helper function, that returns a float sample from an audio bytestream of
 * arbitrary format and channel number
 *
*/
float getSample(u_int8_t* in, u_int32_t frameIndex, u_int32_t channel, const SampleSpecs& sampleSpecs)
{
    // Protect against segfault
    if (frameIndex > sampleSpecs.buffersizeInFramesPerPeriode)
        return 0.f;
    if (channel > sampleSpecs.channels)
        return 0.f;

    if (sampleSpecs.isSigned) {

        signed int currentSample = 0;
        signed int currentMask = 0;

        if (sampleSpecs.isLittleEndian) {
            for (unsigned int byte=0; byte<sampleSpecs.bytesPerSample; byte++) {
                currentSample |= in[getByteIndex(frameIndex, channel, byte, sampleSpecs)] << (byte*8);
                currentMask |= (0xFF << (byte*8));
            }
        } else {
            for (unsigned int byte=0; byte<sampleSpecs.bytesPerSample; byte++) {
                currentSample |= in[getByteIndex(frameIndex, channel, byte, sampleSpecs)] << ((sampleSpecs.bytesPerSample-byte-1)*8);
                currentMask |= (0xFF << (byte*8));
            }
        }
        // If MSB is 1 we deal with negative numbers.
        if (currentSample & (1 << (sampleSpecs.bytesPerSample*8-1)))
            currentSample |= ~currentMask;

        // Stephan remarked, that float values should be clipped/normalized to -1,0...1,0
        // If this function is correct, the division below can not be < -1,0 || > 1,0, so
        // I save this extra if statement for now.
        return static_cast<float>(currentSample) / static_cast<float>(currentMask);

    } else { // UNSIGNED
        //TODO: implement me
        return 0.f;
    }
}

// Think about inline here
/** \ingroup Tools
 *
 * \brief Writes a float sample to an audio bytestream of arbitrary format
 * \param out Bytestream of audiodata
 * \param sample Float sample from -1.0 to 1.0
 * \param frameIndex Index of the frame we are working with
 * \param channel Channel number
 * \param sampleSpecs Sample specification of the current setup
 *
 * Helper function, that writes a float sample to an audio bytestream of
 * arbitrary format and channel number
 *
*/
void setSample(u_int8_t* out, float sample, u_int32_t frameIndex, u_int32_t channel, const SampleSpecs& sampleSpecs)
{
    // Protect against segfault
    if (frameIndex > sampleSpecs.buffersizeInFramesPerPeriode)
        return;
    if (channel > sampleSpecs.channels)
        return;

    // Clip/Normalize sample
    if (sample > 1.0) sample = 1.0;
    if (sample < -1.0) sample = -1.0;

    if (sampleSpecs.isSigned) {

        int32_t currentMask = 0;

        for (unsigned int byte=0; byte<sampleSpecs.bytesPerSample; byte++)
            currentMask |= (0xFF << (byte*8));

        currentMask &= ~(1 << (sampleSpecs.bytesPerSample*8-1));

        int32_t currentSample = static_cast<int32_t>(sample * currentMask);

        if (sampleSpecs.isLittleEndian) {
            for (unsigned int byte=0; byte<sampleSpecs.bytesPerSample; byte++) {
                out[getByteIndex(frameIndex, channel, byte, sampleSpecs)] =
                        (currentSample) >> (byte*8);
            }
        } else {
            for (unsigned int byte=0; byte<sampleSpecs.bytesPerSample; byte++) {
                out[getByteIndex(frameIndex, channel, byte, sampleSpecs)] =
                        (currentSample) >> ((sampleSpecs.bytesPerSample-byte-1)*8);
            }
        }
    } else { // UNSIGNED
        //TODO: implement me
    }
}

/** \ingroup Tools
 *
 * \brief Writes a int32 sample to an audio bytestream of arbitrary format
 * \param out Bytestream of audiodata
 * \param int32 sample
 * \param frameIndex Index of the frame we are working with
 * \param channel Channel number
 * \param sampleSpecs Sample specification of the current setup
 *
 * Helper function, that writes a float sample to an audio bytestream of
 * arbitrary format and channel number
 *
*/
void setSample(u_int8_t* out, int32_t sample, u_int32_t frameIndex, u_int32_t channel, const SampleSpecs& sampleSpecs)
{
    // Protect against segfault
    if (frameIndex > sampleSpecs.buffersizeInFramesPerPeriode)
        return;
    if (channel > sampleSpecs.channels)
        return;

    if (sampleSpecs.isSigned) {
        if (sampleSpecs.isLittleEndian) {
            for (unsigned int byte=0; byte<sampleSpecs.bytesPerSample; byte++) {
                out[getByteIndex(frameIndex, channel, byte, sampleSpecs)] =
                        (sample) >> (byte*8);
            }
        } else {
            for (unsigned int byte=0; byte<sampleSpecs.bytesPerSample; byte++) {
                out[getByteIndex(frameIndex, channel, byte, sampleSpecs)] =
                        (sample) >> ((sampleSpecs.bytesPerSample-byte-1)*8);
            }
        }
    } else { // UNSIGNED
        //TODO: implement me
    }
}


/** \ingroup Tools
 *
 * \brief Print buffer information using operator<<
 * \param lhs Left hand Side
 * \param rhs Right hand Side
 *
 * Helper overload of operator<< for SampleSpecs_t
 * so it can be used as:
 *
 * \code{.cpp}
 *	std::cout << "SampleSpecs: " << std::endl << mySampleSpecs << std::endl;
 * \endcode
*/
std::ostream& operator<<(std::ostream& lhs, const SampleSpecs& rhs)
{
    lhs << "Samplerate:                        " << rhs.samplerate << std::endl <<
           "Buffersize in Samples:             " << rhs.buffersizeInSamples << std::endl <<
           "Buffersize in Samples Per Periode: " << rhs.buffersizeInSamplesPerPeriode << std::endl <<
           "Buffersize in Frames:              " << rhs.buffersizeInFrames << std::endl <<
           "Buffersize in Frames Per Periode:  " << rhs.buffersizeInFramesPerPeriode << std::endl <<
           "Buffersize in Bytes:               " << rhs.buffersizeInBytes << std::endl <<
           "Buffersize in Bytes Per Periode:   " << rhs.buffersizeInBytesPerPeriode << std::endl <<
           "Bytes Per Frame:                   " << rhs.bytesPerFrame << std::endl <<
           "Bytes Per Sample:                  " << rhs.bytesPerSample << std::endl <<
           "Bytes Per Sample Physical:         " << rhs.bytesPerSamplePhysical << std::endl <<
           "Channels:                          " << rhs.channels << std::endl <<
           "isLittleEndian:                    " << rhs.isLittleEndian << std::endl <<
           "isFloat                            " << rhs.isFloat << std::endl <<
           "isSigned:                          " << rhs.isSigned << std::endl <<
           "latency:                           " << rhs.latency << " ms" << std::endl;
    return lhs;
}

void to_json(nlohmann::json& j, const SampleSpecs& s)
{
    j = nlohmann::json{
    { "samplerate",                         s.samplerate},
    { "buffersize_in_samples",              s.buffersizeInSamples},
    { "buffersize_in_samples_per_periode",  s.buffersizeInSamplesPerPeriode},
    { "buffersize_in_frames",               s.buffersizeInFrames},
    { "buffersize_in_frames_per_periode",   s.buffersizeInFramesPerPeriode},
    { "buffersize_in_bytes",                s.buffersizeInBytes},
    { "buffersize_in_bytes_per_periode",    s.buffersizeInBytesPerPeriode},
    { "bytes_per_frame",                    s.bytesPerFrame},
    { "bytes_per_sample",                   s.bytesPerSample},
    { "bytes_per_sample_physical",          s.bytesPerSamplePhysical},
    { "channels",                           s.channels},
    { "is_float",                           s.isFloat},
    { "is_little_endian",                   s.isLittleEndian},
    { "is_signed",                          s.isSigned},
    { "latency",                            s.latency}
};
}

void from_json(const nlohmann::json& j, SampleSpecs& s)
{
    s.samplerate                    = j.at("samplerate").get<unsigned int>();
    s.buffersizeInBytes             = j.at("buffersize_in_samples").get<unsigned int>();
    s.buffersizeInBytesPerPeriode   = j.at("buffersize_in_samples_per_periode").get<unsigned int>();
    s.buffersizeInFrames            = j.at("buffersize_in_frames").get<unsigned int>();
    s.buffersizeInFramesPerPeriode  = j.at("buffersize_in_frames_per_periode").get<unsigned int>();
    s.buffersizeInSamples           = j.at("buffersize_in_samples").get<unsigned int>();
    s.buffersizeInSamplesPerPeriode = j.at("buffersize_in_samples_per_periode").get<unsigned int>();
    s.bytesPerFrame                 = j.at("bytes_per_frame").get<unsigned int>();
    s.bytesPerSample                = j.at("bytes_per_sample").get<unsigned int>();
    s.bytesPerSamplePhysical        = j.at("bytes_per_sample_physical").get<unsigned int>();
    s.channels                      = j.at("channels").get<unsigned int>();
    s.isFloat                       = j.at("is_float").get<bool>();
    s.isLittleEndian                = j.at("is_little_endian").get<bool>();
    s.isSigned                      = j.at("is_signed").get<bool>();
    s.latency                       = j.at("latency").get<double>();
}

nlohmann::json toJSON(const SampleSpecs& ss)
{
    return ss;
}


} // namespace Nl
