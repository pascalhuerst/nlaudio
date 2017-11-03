#include "audio/audiojack.h"
#include "common/blockingcircularbuffer.h"
#include "audio/alsa/alsacardidentifier.h"
#include "common/bufferstatistics.h"
#include "audio/samplespecs.h"

namespace Nl {

AudioJack::AudioJack(const AlsaCardIdentifier &card, SharedBufferHandle buffer, bool isInput) :
	m_isInput(isInput),
	m_audioBuffer(buffer),
	m_jackPort(nullptr),
	m_jackClient(nullptr),
	m_card(card)

{
	std::cout << "New " << __func__ << " as " << (isInput ? "input" : "output") << std::endl;
}

AudioJack::~AudioJack()
{
	close();
}

void AudioJack::open()
{
	jack_status_t status;
	if ((m_jackClient = jack_client_open("jack_client_open", JackNullOption, &status)) == nullptr) {
		std::cout << "### JACK server not running? ret=" << m_jackClient << " status=" << status << std::endl;
	}

	if ((m_jackPort = jack_port_register(m_jackClient, "jack_port_register", JACK_DEFAULT_AUDIO_TYPE,
										 m_isInput ? JackPortIsInput : JackPortIsOutput, 0)) == nullptr) {
		std::cout << "### some error on calling jack_port_register" << std::endl;
	}

}

void AudioJack::close()
{
	int ret;
	if ((ret = jack_port_unregister(m_jackClient, m_jackPort)) != 0) {
		std::cout << "### some error on calling jack_port_unregister" << std::endl;
	}

	if ((ret = jack_client_close(m_jackClient)) != 0) {
		std::cout << "### some error on calling jack_client_close" << std::endl;
	}
}

void AudioJack::start()
{
	int ret = 0;

	init();

	if ((ret = jack_set_process_callback(m_jackClient, AudioJack::worker, nullptr)) != 0) {
		std::cout << "### some error on calling jack_set_process_callback(" << ret << ")" << std::endl;
	}

	if ((ret = jack_activate(m_jackClient)) != 0) {
		std::cout << "### some error on calling jack_activate" << std::endl;
	}

}

void AudioJack::stop()
{
	int ret;
	if ((ret = jack_deactivate(m_jackClient)) != 0) {
		std::cout << "### some error on calling jack_deaktivate" << std::endl;
	}

}

void AudioJack::init()
{
	m_audioBuffer->init(512);
}

void AudioJack::setBuffersize(unsigned int buffersize)
{
	std::cout << __func__ << ": implement me!" << std::endl;
}

unsigned int AudioJack::getBuffersize()
{
	std::cout << __func__ << ": implement me!" << std::endl;
	return 0;
}

void AudioJack::setBufferCount(unsigned int buffercount)
{
	std::cout << __func__ << ": implement me!" << std::endl;
}

unsigned int AudioJack::getBufferCount()
{
	std::cout << __func__ << ": implement me!" << std::endl;
	return 0;
}

void AudioJack::setSamplerate(samplerate_t rate)
{
	std::cout << __func__ << ": implement me!" << std::endl;
}

samplerate_t AudioJack::getSamplerate() const
{
	std::cout << __func__ << ": implement me!" << std::endl;
	return 0;
}

std::list<sampleformat_t> AudioJack::getAvailableSampleformats() const
{
	std::cout << __func__ << ": implement me!" << std::endl;
	std::list<sampleformat_t> ret;
	return ret;
}

sampleformat_t AudioJack::getSampleFormat() const
{
	std::cout << __func__ << ": implement me!" << std::endl;
	return "";
}

void AudioJack::setSampleFormat(sampleformat_t format)
{
	std::cout << __func__ << ": implement me!" << std::endl;
}

void AudioJack::setChannelCount(channelcount_t n)
{
	std::cout << __func__ << ": implement me!" << std::endl;
}

channelcount_t AudioJack::getChannelCount()
{
	std::cout << __func__ << ": implement me!" << std::endl;
}

BufferStatistics AudioJack::getStats()
{
	//TODO: Implement me!!
    return { 0, 0, 0 };
}

SampleSpecs AudioJack::getSpecs()
{
    //TODO: implement me!!
    return  { 0 };
}


//static
int AudioJack::worker(jack_nframes_t nframes, void *arg)
{
	AudioJack *instance = static_cast<AudioJack*>(arg);
	bool isInput = instance->m_isInput;

	unsigned int i;

	void* buffer = jack_port_get_buffer(instance->m_jackPort, nframes);

	return 0;

	if (isInput)
		instance->m_audioBuffer->set(static_cast<unsigned char*>(buffer), nframes);
	else
		instance->m_audioBuffer->get(static_cast<unsigned char*>(buffer), nframes);

	return 0;
}

} // namespace Nl
