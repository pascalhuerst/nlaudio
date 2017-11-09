/******************************************************************************/
/** @file		echo.h
    @date		2017-05-11
    @version	1.0
    @author		Anton Schmied[2016-03-18]
    @brief		An implementation of the Echo Effect
                as used in the C15 and implemented in Reaktor
*******************************************************************************/

#pragma once

#include "nltoolbox.h"
#include "nlglobaldefines.h"
#include "onepolefilters.h"
#include <array>

class Echo
{
public:

    Echo ();                            // Default Constructor

    Echo(float _delayTime,
         float _stereo,
         float _feedbackAmnt,
         float _crossFeedbackAmnt,
         float _hiCut,
         float _mix);

    ~Echo();                            // Class Destructor

    float mEchoOut_L;                   // public processed samples
    float mEchoOut_R;

    void applyEcho(float _rawSample_L, float _rawSample_R);        // main audio function, which processes the incoming sample

    void setEchoParams(unsigned char _ctrlId, float _ctrlVal);          // sets the effect parameters and activates smoothers

    inline void applyEchoSmoother();                                    // smoothing function, which is only applied, if new target values are set

#ifdef NANOKONTROL_I
    float applyEcho(float _currSample, uint32_t _chInd);                // main audio function, which processes the incoming sample
#endif

private:
    //******************************** Control Variabels ********************************//

    float mFeedbackAmnt;            // feedback amount - external
    float mCrossFeedbackAmnt;       // cross feedback amount - external

    float mStereoAmnt;				// stereo amount
    float mDelayTime;				// delay time

    float mDry;						// dry amount, dependant on mix amount
    float mWet;						// wet amount, dependant on mix amount
    float mLocalFeedback;           // local feedback amount - internal
    float mCrossFeedback;			// cross feedback amount - internal


    //************************* Channel Variables and Filters ***************************//

    float mChannelStateVar_L;                       // channel state variable
    float mChannelStateVar_R;

    float mDelayTime_L;                             // channel delay time
    float mDelayTime_R;

    std::array<float, 131072> mSampleBuffer_L;      // sample buffer for writing and reading the samples
    std::array<float, 131072> mSampleBuffer_R;
    uint32_t mSampleBufferSize;                     // sample buffer size - 1
    uint32_t mSampleBufferIndx;                     // sample buffer index

    OnePoleFilters* pLowpass_L;                     // lowpass filter
    OnePoleFilters* pLowpass_R;

    OnePoleFilters* pHighpass_L;                    // highpass filter at 50Hz
    OnePoleFilters* pHighpass_R;

    NlToolbox::Filters::Lowpass2Hz* pLowpass2Hz_L;  // 2Hz lowpass filter for smoothing the delay time
    NlToolbox::Filters::Lowpass2Hz* pLowpass2Hz_R;


    //******************************* Smoothing Variabels *******************************//

    uint32_t mSmootherMask;                    // Smoother Mask (ID 1: dry, ID 2: wet, ID 3: local feedback, ID 4: cross feedback)

    // Mask ID: 1
    float mDry_base;
    float mDry_target;
    float mDry_diff;
    float mDry_ramp;

    // Mask ID: 2
    float mWet_base;
    float mWet_target;
    float mWet_diff;
    float mWet_ramp;

    // Mask ID: 3
    float mLFeedback_base;
    float mLFeedback_target;
    float mLFeedback_diff;
    float mLFeedback_ramp;

    // Mask ID: 4
    float mCFeedback_base;
    float mCFeedback_target;
    float mCFeedback_diff;
    float mCFeedback_ramp;


    //*********************************** Controls IDs **********************************//


    enum CtrlID: unsigned char
    {
#ifdef NANOKONTROL_I                    // Korg Nano Kontrol I
        HICUT             = 0x12,
        MIX               = 0x04,
        DELAYTIME         = 0x05,
        STEREOAMNT        = 0x06,
        FEEDBACKAMNT      = 0x08,
        CROSSFEEDBACKAMNT = 0x09
#endif
#ifdef REMOTE61                         // ReMote 61
        DELAYTIME         = 0x29,
        STEREOAMNT        = 0x2A,
        FEEDBACKAMNT      = 0x2B,
        CROSSFEEDBACKAMNT = 0x2C,
        HICUT             = 0x2D,
        MIX               = 0x30
#endif
    };


    //**************************** Helper Functions ***************************//

    void calcFeedback();                    // calculates cross and local feedback amounts
    inline void calcChannelDelayTime();     // calculates delay times for each channel
};
