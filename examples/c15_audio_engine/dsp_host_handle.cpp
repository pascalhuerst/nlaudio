#include "dsp_host_handle.h"

#include <audio/audioalsainput.h>
#include <audio/audioalsaoutput.h>
#include <midi/rawmididevice.h>

namespace Nl {
namespace DSP_HOST_HANDLE {

    dsp_host dsp_host;

    /** @brief    Callback function for Sine Generator and Audio Input - testing with ReMote 61
            @param    Input Buffer
            @param    Output Buffer

            @param    buffer size
            @param    Sample Specs
    */
    void dspHostCallback(uint8_t *in, uint8_t *out, const SampleSpecs &sampleSpecs __attribute__ ((unused)), SharedUserPtr ptr)
    {
        auto midiBuffer = getBufferForName("MidiBuffer");

        //---------------- Retrieve Midi Information if midi values have changed
        if (midiBuffer)
        {
            unsigned char midiByteBuffer[3];                                // MidiByteBuffer Structure: [0] - , [1] - Played Note Address, [2] - Velocity

            while (midiBuffer->availableToRead() >= 3)
            {
                midiBuffer->get(midiByteBuffer, 3);

                // printf("%02X %02X %02X\n", midiByteBuffer[0], midiByteBuffer[1], midiByteBuffer[2]);      // MIDI Value Control Output

#if 1
                dsp_host.evalMidi();
#else
                dsp_host.evalMidi();
#endif
            }
        }

        for (unsigned int frameIndex = 0; frameIndex < sampleSpecs.buffersizeInFramesPerPeriode; ++frameIndex)
        {
            dsp_host.tickMain();

            float outputSample;

            for (unsigned int channelIndex = 0; channelIndex < sampleSpecs.channels; ++channelIndex)
            {
                if (channelIndex)
                {
                     outputSample = dsp_host.m_mainOut_R;
                }
                else if (!channelIndex)
                {
                     outputSample = dsp_host.m_mainOut_L;
                }


                if (outputSample > 1.f || outputSample < -1.f)                  // Clipping
                {
                    printf("WARNING!!! C15 CLIPPING!!!\n");
                }

                setSample(out, outputSample, frameIndex, channelIndex, sampleSpecs);
            }
        }
    }







    JobHandle dspHostTCDControl(const AlsaCardIdentifier &audioInCard,
                                const AlsaCardIdentifier &audioOutCard,
                                const AlsaCardIdentifier &midiInCard,
                                unsigned int buffersize,
                                unsigned int samplerate)
    {
        dsp_host.setSamplerate(samplerate);

        JobHandle ret;

        ret.inBuffer = createBuffer("InputBuffer");
        ret.outBuffer = createBuffer("OutputBuffer");

        ret.audioInput = createAlsaInputDevice(audioInCard, ret.inBuffer, buffersize);
        ret.audioInput->setSamplerate(samplerate);

        ret.audioOutput = createAlsaOutputDevice(audioOutCard, ret.outBuffer, buffersize);
        ret.audioOutput->setSamplerate(samplerate);

        ret.inMidiBuffer = createBuffer("MidiBuffer");
        ret.midiInput = createRawMidiDevice(midiInCard, ret.inMidiBuffer);

        ret.audioOutput->start();
        ret.audioInput->start();
        ret.midiInput->start();

        ret.workingThreadHandle = registerInOutCallbackOnBuffer(ret.inBuffer, ret.outBuffer, dspHostCallback, nullptr);

        return ret;
    }
} // namespace DSP_HOST
} // namespace Nl