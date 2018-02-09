/******************************************************************************/
/** @file		voicemanager.cpp
    @date		2016-07-26
    @version	0.1
    @author		Anton Schmied[2016-07-26]
    @brief		..
*******************************************************************************/

#include "voicemanager.h"

float PARAMSIGNALDATA[NUM_VOICES][NUM_SIGNALS];

/******************************************************************************/
/** Voice Manager Default Constructor
 * @brief    initialization of the modules local variabels with default values
*******************************************************************************/

VoiceManager::VoiceManager()
    : loop_watch()
{
#if 0
    callbackArray[0] = &VoiceManager::keyDown;
    callbackArray[1] = &VoiceManager::presetChange;
#endif

    pParamengine = new Paramengine();

    uint32_t i;

    for (i = 0; i < NUM_VOICES; i++)
    {
        pEnvelopes[i] = new Envelopes();
    }

    for (i = 0; i < NUM_VOICES; i++)
    {
        pSoundGenerator[i] = new Soundgenerator();
        pSoundGenerator[i]->setVoiceNumber(i);
    }

    for (i = 0; i < NUM_VOICES; i++)
    {
        pCombFilter[i] = new CombFilter();
    }

    for (i = 0; i < NUM_VOICES; i++)
    {
        pSVFilter[i] = new StateVariableFilter();
    }

    for (i = 0; i < NUM_VOICES; i++)
    {
        pFeedbackMixer[i] = new FeedbackMixer();
    }


    pOutputMixer = new Outputmixer();
    pFlanger = new Flanger();
    pCabinet_L = new Cabinet();
    pCabinet_R = new Cabinet();
    pGapFilter = new GapFilter();
    pEcho = new Echo();
    pReverb = new Reverb();

    mainOut_L = 0.f;
    mainOut_R = 0.f;

    vallocInit();                                       // Voice allocation initialization
}



/******************************************************************************/
/** Voice Manager Destructor
 * @brief
*******************************************************************************/

VoiceManager::~VoiceManager()
{
    delete pParamengine;

    for (uint32_t i = 0; i < NUM_VOICES; i++)
    {
        delete pEnvelopes[i];
        pEnvelopes[i] = NULL;

        delete pSoundGenerator[i];
        pSoundGenerator[i] = NULL;

        delete pCombFilter[i];
        pCombFilter[i] = NULL;

        delete pSVFilter[i];
        pSVFilter[i] = NULL;

        delete pFeedbackMixer[i];
        pFeedbackMixer[i] = NULL;
    }


    delete pOutputMixer;
    delete pFlanger;
    delete pCabinet_L;
    delete pCabinet_R;
    delete pGapFilter;
    delete pEcho;
    delete pReverb;
}

/******************************************************************************/
/** @brief    Parameter setter function
*******************************************************************************/

void VoiceManager::evalMidiEvents(unsigned char _instrID, unsigned char _ctrlID, float _ctrlVal)
{
    switch (_instrID)
    {
        case InstrID::PARAM_ENGINE:
            pParamengine->setParams(_ctrlID, _ctrlVal);

            break;

        case InstrID::ENEVELOPES_PARAM:
            for (uint32_t i = 0; i < NUM_VOICES; i++)
            {
                pEnvelopes[i]->setEnvelopePramas(_ctrlID, _ctrlVal);
            }
            break;

        case InstrID::OSC_A_PARAM:
        case InstrID::OSC_B_PARAM:
        case InstrID::SHAPER_A_PARAM:
        case InstrID::SHAPER_B_PARAM:
            for (uint32_t i = 0; i < NUM_VOICES; i++)
            {
                pSoundGenerator[i]->setGenParams(_instrID, _ctrlID, _ctrlVal);
            }
            break;

        case InstrID::COMBFILTER_PARAM:

            for (uint32_t i = 0; i < NUM_VOICES; i++)
            {
                pCombFilter[i]->setCombFilterParams(_ctrlID, _ctrlVal);
            }
            break;

        case InstrID::STATEVARIABLE_PARAM:

            for (uint32_t i = 0; i < NUM_VOICES; i++)
            {
                pSVFilter[i]->setStateVariableFilterParams(_ctrlID, _ctrlVal);
            }
            break;

        case InstrID::FEEDBACKMIXER_PARAM:

            /// Reverb Level should be enough for just one FB Mixer instance ...

            for (uint32_t i = 0; i < NUM_VOICES; i++)
            {
                pFeedbackMixer[i]->setFeedbackMixerParams(_ctrlID, _ctrlVal);
            }
            break;

        case InstrID::OUTPUTMIXER_PARAM:

            pOutputMixer->setOutputmixerParams(_ctrlID, _ctrlVal);
            break;

        case InstrID::FLANGER_PARAM:

            pFlanger->setFlangerParams(_ctrlID, _ctrlVal);
            break;

        case InstrID::CABINET_PARAM:

            pCabinet_L->setCabinetParams(_ctrlID, _ctrlVal);
            pCabinet_R->setCabinetParams(_ctrlID, _ctrlVal);
            break;

        case InstrID::GAP_PARAM:

            pGapFilter->setGapFilterParams(_ctrlID, _ctrlVal);
            break;


        case InstrID::ECHO_PARAM:

            pEcho->setEchoParams(_ctrlID, _ctrlVal);
            break;

        case InstrID::REVERB_PARAM:

            pReverb->setReverbParams(_ctrlID, _ctrlVal);
            break;

        case InstrID::KEYUP_0:
        case InstrID::KEYUP_1:
        case InstrID::KEYUP_2:
        case InstrID::KEYUP_3:
        case InstrID::KEYUP_4:
        case InstrID::KEYUP_5:
        case InstrID::KEYUP_6:
        case InstrID::KEYUP_7:
        case InstrID::KEYUP_8:
        case InstrID::KEYUP_9:
        case InstrID::KEYUP_10:
        case InstrID::KEYUP_11:
        case InstrID::KEYUP_12:
        case InstrID::KEYUP_13:
        case InstrID::KEYUP_14:
        case InstrID::KEYUP_15:

            vallocProcess(0, _ctrlID, _ctrlVal);
            break;

        case InstrID::KEYDOWN_0:
        case InstrID::KEYDOWN_1:
        case InstrID::KEYDOWN_2:
        case InstrID::KEYDOWN_3:
        case InstrID::KEYDOWN_4:
        case InstrID::KEYDOWN_5:
        case InstrID::KEYDOWN_6:
        case InstrID::KEYDOWN_7:
        case InstrID::KEYDOWN_8:
        case InstrID::KEYDOWN_9:
        case InstrID::KEYDOWN_10:
        case InstrID::KEYDOWN_11:
        case InstrID::KEYDOWN_12:
        case InstrID::KEYDOWN_13:
        case InstrID::KEYDOWN_14:
        case InstrID::KEYDOWN_15:

            vallocProcess(1, _ctrlID, _ctrlVal);
            break;
    }
}



/******************************************************************************/
/** @brief    voice loop function which calculates all module output values
 *            for all voices
*******************************************************************************/

void VoiceManager::voiceLoop()
{
    float *voiceSignal = nullptr;

    static int counter = 0;
    static int deltaCounter = 1;
    static float deltaCpu = 0.f;

    loop_watch.start();

    //***************************** Main DSP Loop ***************************//
    for (uint32_t voiceNumber = 0; voiceNumber < NUM_VOICES; voiceNumber++)
    {

#ifdef SINGLARRAY
#ifdef STACKSINGLE
        pEnvelopes[voiceNumber]->applyEnvelope(voiceNumber);
        pSoundGenerator[voiceNumber]->generateSound(pFeedbackMixer[voiceNumber]->mFeedbackOut, voiceNumber);
#endif
#ifdef HEAPSINGLE
        pEnvelopes[voiceNumber]->applyEnvelope(voiceNumber);
        pSoundGenerator[voiceNumber]->generateSound(pFeedbackMixer[voiceNumber]->mFeedbackOut, voiceNumber);
#endif
#endif
#ifdef GLBLARRAY
        voiceSignal = PARAMSIGNALDATA[voiceNumber];
        pEnvelopes[voiceNumber]->applyEnvelope(voiceSignal);
        pSoundGenerator[voiceNumber]->generateSound(pFeedbackMixer[voiceNumber]->mFeedbackOut, voiceSignal);
#endif
#ifdef NOARRAY
        pEnvelopes[voiceNumber]->applyEnvelope();
        pSoundGenerator[voiceNumber]->generateSound(pFeedbackMixer[voiceNumber]->mFeedbackOut, pEnvelopes[voiceNumber]->mEnvRamp_A, pEnvelopes[voiceNumber]->mEnvRamp_B, pEnvelopes[voiceNumber]->mEnvRamp_C, pEnvelopes[voiceNumber]->mGateRamp);
#endif

        pCombFilter[voiceNumber]->applyCombFilter(pSoundGenerator[voiceNumber]->mSampleA, pSoundGenerator[voiceNumber]->mSampleB);
        pSVFilter[voiceNumber]->applyStateVariableFilter(pSoundGenerator[voiceNumber]->mSampleA, pSoundGenerator[voiceNumber]->mSampleB, pCombFilter[voiceNumber]->mCombFilterOut);
        pFeedbackMixer[voiceNumber]->applyFeedbackMixer(pCombFilter[voiceNumber]->mCombFilterOut, pSVFilter[voiceNumber]->mSVFilterOut, pReverb->mFeedbackOut);
        pOutputMixer->applyOutputMixer(voiceNumber, pSoundGenerator[voiceNumber]->mSampleA, pSoundGenerator[voiceNumber]->mSampleB, pCombFilter[voiceNumber]->mCombFilterOut, pSVFilter[voiceNumber]->mSVFilterOut);
    }

    loop_watch.stop();
    counter++;

    if (counter == NUMBER_OF_TS)
    {
        loop_watch.calcCPU();
        std::cout << "CPU Peak: " << loop_watch.mCPU_peak << std::endl;
        counter = 0;

        deltaCpu = deltaCpu + loop_watch.mCPU_peak;

        std::cout << "CPU Peak delta: " << deltaCpu / deltaCounter << std::endl;
        deltaCounter++;
    }


    //******************************** Flanger ******************************//

    pFlanger->applyFlanger(pOutputMixer->mSample_L, pOutputMixer->mSample_R);


    //******************************** Cabinet ******************************//

    pCabinet_L->applyCab(pFlanger->mFlangerOut_L);
    pCabinet_R->applyCab(pFlanger->mFlangerOut_R);


    //****************************** Gap Filter *****************************//

    pGapFilter->applyGapFilter(pCabinet_L->mCabinetOut, pCabinet_R->mCabinetOut);


    //********************************** Echo *******************************//

    pEcho->applyEcho(pGapFilter->mGapFilterOut_L, pGapFilter->mGapFilterOut_R);


    //********************************* Reverb ******************************//

    pReverb->applyReverb(pEcho->mEchoOut_L, pEcho->mEchoOut_R, pFeedbackMixer[0]->mReverbLevel);


    //******************************* Soft Clip *****************************//

    mainOut_L = pReverb->mReverbOut_L * 1.f;                  /// Volume anstatt 1.f
    mainOut_L *= 0.1588f;

    if (mainOut_L > 0.25f)
    {
        mainOut_L = 0.25f;
    }
    if (mainOut_L < -0.25f)
    {
        mainOut_L = -0.25f;
    }

    mainOut_L += -0.25f;
    mainOut_L += mainOut_L;

    mainOut_L = 0.5f - fabs(mainOut_L);

    float sample_square = mainOut_L * mainOut_L;
    mainOut_L = mainOut_L * ((2.26548 * sample_square - 5.13274) * sample_square + 3.14159);


    mainOut_R = pReverb->mReverbOut_R * 1.f;                  /// Volume anstatt 1.f
    mainOut_R *= 0.1588f;

    if (mainOut_R > 0.25f)
    {
        mainOut_R = 0.25f;
    }
    if (mainOut_R < -0.25f)
    {
        mainOut_R = -0.25f;
    }

    mainOut_R += -0.25f;
    mainOut_R += mainOut_R;

    mainOut_R = 0.5f - fabs(mainOut_R);

    sample_square = mainOut_R * mainOut_R;
    mainOut_R = mainOut_R * ((2.26548 * sample_square - 5.13274) * sample_square + 3.14159);
}



/******************************************************************************/
/** @brief    voice allocation initialization
 *
*******************************************************************************/

void VoiceManager::vallocInit()
{
    for (uint32_t i = 0; i < NUM_VOICES; i++)
    {
        vVoiceState[i] = -1;
        vNextAssigned[i] = 0;
        vPreviousAssigned[i] = 0;
    }

    vNumAssigned = 0;
    vOldestAssigned = 0;
    vOldestReleased = 0;
    vYoungestAssigned = 0;
    vYoungestReleased = NUM_VOICES - 1;

    for (uint32_t i = 0; i < NUM_VOICES - 1; i++)
    {
        vNextReleased[i] = i + 1;
    }
}



/******************************************************************************/
/** @brief    voice allocation process
 *  @param    ID for keyUp or keyDown
*******************************************************************************/

void VoiceManager::vallocProcess(uint32_t _keyDirection, float _pitch, float _velocity)
{
    //************************* Key Down *************************************//
    if (_keyDirection)                          // _keyDirection = 1
    {
        uint32_t v;

        if (vNumAssigned < NUM_VOICES)
        {
            if (vNumAssigned == 0)
            {
                vOldestAssigned = vOldestReleased;
                vYoungestAssigned = vOldestReleased;
            }

            v = vOldestReleased;
            vOldestReleased = vNextReleased[v];

            vNumAssigned++;
        }
        else
        {
            v = vOldestAssigned;
            vOldestAssigned = vNextAssigned[v];
        }

        vPreviousAssigned[v] = vYoungestAssigned;
        vNextAssigned[vYoungestAssigned] = v;

        vYoungestAssigned = v;

        vVoiceState[v] = _pitch;


        //************** Pass Pitch and Velocity to Modules ******************//

        float offsetPitch = _pitch - 60.f;

        pEnvelopes[v]->setEnvelope(_velocity/ 127.f);
        pSoundGenerator[v]->setPitch(offsetPitch);
//        callbackArray[0](80,v);
        pSoundGenerator[v]->resetPhase();
        pCombFilter[v]->setPitch(offsetPitch);
        pSVFilter[v]->setPitch(offsetPitch);
        pFeedbackMixer[v]->setPitchInfluence(offsetPitch);

        pOutputMixer->setKeyPitch(v, _pitch);
        pFlanger->triggerLFO(_velocity/127.f);
    }

    //************************** Key Up **************************************//
    else if (!_keyDirection)                // _keyDirection = 0
    {
        for (uint32_t v = 0; v < NUM_VOICES; v++)
        {
            if (vVoiceState[v] == _pitch)
            {
                vNextReleased[vYoungestReleased] = v;
                vYoungestReleased = v;

                if (vNumAssigned == NUM_VOICES)
                {
                    vOldestReleased = v;
                }

                vNumAssigned--;

                if (vOldestAssigned == v)
                {
                    vOldestAssigned = vNextAssigned[v];

                }
                else if (vYoungestAssigned == v)
                {
                    vYoungestAssigned = vPreviousAssigned[v];
                }
                else
                {
                    vNextAssigned[vPreviousAssigned[v]] = vNextAssigned[v];
                    vPreviousAssigned[vNextAssigned[v]] = vPreviousAssigned[v];
                }

                vVoiceState[v] = -1;

                pEnvelopes[v]->killEnvelope();
                break;
            }
        }
    }
}

