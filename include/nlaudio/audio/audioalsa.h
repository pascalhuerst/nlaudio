#pragma once

#include <alsa/asoundlib.h>

#include <atomic>
#include <thread>
#include <iosfwd>

#include "audio/audio.h"
#include "audio/alsa/alsacardidentifier.h"
#include "audio/alsa/alsacardinfo.h"
#include "common/bufferstatistics.h"
#include "common/blockingcircularbuffer.h"

namespace Nl {

class AudioAlsa : public Audio
{
public:
	typedef Audio basetype;

	AudioAlsa(const AlsaCardIdentifier& card, SharedBufferHandle buffer, bool isInput);
	virtual ~AudioAlsa();

	virtual void open() = 0; // Might throw, therefore not in constructor
	virtual void close();

	virtual void start() = 0;
	virtual void stop() = 0;

	virtual void init() = 0;

	//TODO: Changing buffer size, while playback is running might have strange sideeffects!
	virtual void setBuffersize(unsigned int buffersize);
	virtual unsigned int getBuffersize();

	virtual void setBufferCount(unsigned int buffercount);
	virtual unsigned int getBufferCount();

	virtual samplerate_t getSamplerate() const;
	virtual void setSamplerate(samplerate_t rate);

	virtual std::list<sampleformat_t> getAvailableSampleformats() const;
	virtual sampleformat_t getSampleFormat() const;
	virtual void setSampleFormat(sampleformat_t format);

	virtual void setChannelCount(channelcount_t n);
	virtual channelcount_t getChannelCount();

	virtual BufferStatistics getStats();
    virtual SampleSpecs getSpecs();

protected:
	void openCommon();
	void throwOnDeviceClosed(const std::string &file, const std::string &func, int line) const;
	void throwOnDeviceRunning(const std::string& file, const std::string& func, int line) const;

	void setTerminateRequest() { m_requestTerminate.store(true); }
	void resetTerminateRequest() { m_requestTerminate.store(false); }
	bool getTerminateRequest() const { return m_requestTerminate; }

	static int xrunRecovery(AudioAlsa *ptr, int err);

protected:
	snd_pcm_t *m_handle;
	std::thread *m_audioThread;
	snd_pcm_hw_params_t *m_hwParams;
	std::atomic<bool> m_requestTerminate;
	std::atomic<unsigned int> m_xrunRecoveryCounter;
	SharedBufferHandle m_audioBuffer;

	void throwOnAlsaError(const std::string &file, const std::string &func, int line, int e) const;
private:
	AlsaCardIdentifier m_card;

	bool m_deviceOpen;
	bool m_isInput;

};

} // namespace Nl
