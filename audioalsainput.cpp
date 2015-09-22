#include "audioalsainput.h"

namespace Nl {

AudioAlsaInput::AudioAlsaInput(const devicename_t &name, std::shared_ptr<BlockingCircularBuffer<u_int8_t> > buffer) :
	basetype(name, buffer, true)
{
}

void AudioAlsaInput::open()
{
	basetype::openCommon();
}

void AudioAlsaInput::start()
{
	throwOnDeviceClosed(__FILE__, __func__, __LINE__);
	resetTerminateRequest();
	throwOnAlsaError(__FILE__, __func__, __LINE__, snd_pcm_hw_params(m_handle, m_hwParams));

	snd_pcm_format_t sampleFormat;
	snd_pcm_hw_params_get_format(m_hwParams, &sampleFormat);

	unsigned int channels = 0;
	throwOnAlsaError(__FILE__, __func__, __LINE__, snd_pcm_hw_params_get_channels(m_hwParams, &channels));

	SampleSpecs_t specs;
	specs.samplerate = getSamplerate();
	specs.isSigned = snd_pcm_format_signed(sampleFormat) == 1;
	specs.isLittleEndian = snd_pcm_format_little_endian(sampleFormat) == 1;
	specs.isFloat = snd_pcm_format_float(sampleFormat);
	specs.channels = channels;
	specs.buffersizeInSamples = getBuffersize() / getChannelCount();
	specs.buffersizeInSamplesPerPeriode = specs.buffersizeInSamples / getBufferCount();
	specs.buffersizeInFrames = getBuffersize();
	specs.buffersizeInFramesPerPeriode = getBuffersize() / getBufferCount();
	specs.bytesPerSample = snd_pcm_hw_params_get_sbits(m_hwParams) / 8;
	specs.bytesPerSamplePhysical = snd_pcm_format_physical_width(sampleFormat) / 8;
	specs.bytesPerFrame = specs.bytesPerSample * specs.channels;
	specs.buffersizeInBytes = specs.bytesPerSample * specs.channels * specs.buffersizeInFrames;
	specs.buffersizeInBytesPerPeriode = specs.buffersizeInBytes / getBufferCount();

	basetype::m_audioBuffer->init(specs);
	std::cout << "NlAudioAlsaInput Specs: " << std::endl << specs;

	m_audioThread = new std::thread(AudioAlsaInput::worker, specs, this);
}

void AudioAlsaInput::stop()
{
	throwOnDeviceClosed(__FILE__, __func__, __LINE__);
	setTerminateRequest();

	m_audioThread->join();
	delete m_audioThread;
	m_audioThread = nullptr;
}

//static
void AudioAlsaInput::worker(SampleSpecs_t specs, AudioAlsaInput *ptr)
{

	u_int8_t *buffer = new u_int8_t[specs.buffersizeInBytesPerPeriode];
	memset(buffer, 0, specs.buffersizeInBytesPerPeriode);

	//std::cout << __func__ << " Input in" << std::endl;

	while(!ptr->getTerminateRequest()) {

		int ret = snd_pcm_readi(ptr->m_handle, buffer, specs.buffersizeInFramesPerPeriode);

		if (ret < 0)
			ptr->basetype::xrunRecovery(ptr, ret);
		else if (ret != static_cast<int>(specs.buffersizeInFramesPerPeriode))
			std::cout << "### FIXME ###" << std::endl;

		// Might block, if no space in buffer
		ptr->basetype::m_audioBuffer->set(buffer, specs.buffersizeInBytesPerPeriode);
	}

	snd_pcm_abort(ptr->m_handle);
	delete[] buffer;
}

} // namespace Nl
