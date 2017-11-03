#include "audio/audioalsainput.h"

namespace Nl {

AudioAlsaInput::AudioAlsaInput(const AlsaCardIdentifier &card, SharedBufferHandle buffer) :
	basetype(card, buffer, true)
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

    SampleSpecs specs = basetype::getSpecs();

	init();	

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

void AudioAlsaInput::init()
{
	throwOnDeviceClosed(__FILE__, __func__, __LINE__);
	throwOnAlsaError(__FILE__, __func__, __LINE__, snd_pcm_hw_params(m_handle, m_hwParams));

	m_audioBuffer->init(basetype::getSpecs());
}

//static
void AudioAlsaInput::worker(SampleSpecs specs, AudioAlsaInput *ptr)
{

	u_int8_t *buffer = new u_int8_t[specs.buffersizeInBytesPerPeriode];
	memset(buffer, 0, specs.buffersizeInBytesPerPeriode);

	while(!ptr->getTerminateRequest()) {

		int ret = snd_pcm_readi(ptr->m_handle, buffer, specs.buffersizeInFramesPerPeriode);

		if (ret < 0)
			ptr->basetype::xrunRecovery(ptr, ret);
		else if (ret != static_cast<int>(specs.buffersizeInFramesPerPeriode))
			std::cout << "Only read " << ret << " of " << specs.buffersizeInFramesPerPeriode << " from input device." << std::endl;

		// Might block, if no space in buffer
		ptr->basetype::m_audioBuffer->set(buffer, specs.buffersizeInBytesPerPeriode);
	}

	snd_pcm_abort(ptr->m_handle);
	delete[] buffer;

	std::cout << "void AudioAlsaInput::worker(SampleSpecs specs, AudioAlsaInput *ptr)" << std::endl;
}

} // namespace Nl
