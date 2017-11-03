#include "recorder.h"

#include <audio/audioalsainput.h>
#include <audio/audioalsaoutput.h>
#include <audio/audioalsaexception.h>
#include <audio/audiofactory.h>
#include <audio/samplespecs.h>

#include <common/stopwatch.h>
#include <common/tools.h>
#include <common/controlinterface.h>

#include <stdio.h>
#include <sstream>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int fd = 0;
int file_count = 0;

const std::string fileName = "/tmp/record";

int main()
{
    try
    {
        std::string f = fileName + std::to_string(file_count) + ".raw";
        fd = open(f.c_str(), O_WRONLY | O_CREAT, S_IWRITE | S_IREAD);
        if (fd < 0) {
            std::cout << "Can not open file to write to!" << std::endl;
            return fd;
        }

        Nl::AlsaCardIdentifier audioIn(0,0,0, "USB Device");

        const int buffersize = 1024;
        const int samplerate = 48000;

        auto handle = Nl::Examples::recorder(audioIn, buffersize, samplerate, fd);

        Nl::ControlInterface ci(handle);
        Nl::CommandDescriptor cd1;
        cd1.cmd = "specs";
        cd1.func = [](std::vector<std::string> args, Nl::JobHandle jobHandle, int sockfd, Nl::ControlInterface *ptr) { std::stringstream s; s<<toJSON(jobHandle.audioInput->getSpecs()); write(sockfd, s.str().c_str(), s.str().size()); };
        ci.addCommand(cd1);

        Nl::CommandDescriptor cd2;
        cd2.cmd = "stats";
        cd2.func = [](std::vector<std::string> args, Nl::JobHandle jobHandle, int sockfd, Nl::ControlInterface *ptr) { std::stringstream s; s<<toJSON(jobHandle.audioInput->getStats()); write(sockfd, s.str().c_str(), s.str().size()); };
        ci.addCommand(cd2);

        ci.start();

        while(::getchar() != 'q')
        {
            if (handle.audioInput) {
                std::cout << "Audio: Input Statistics:" << std::endl
                          << handle.audioInput->getStats() << std::endl;
                std::cout << "BufferCount: " << handle.audioInput->getBufferCount() << std::endl;
            }
        }

        ci.stop();

        // Tell worker thread to cleanup and quit
        Nl::terminateWorkingThread(handle.workingThreadHandle);
        if (handle.audioInput) handle.audioInput->stop();

        close(fd);

    } catch (Nl::AudioAlsaException& e) {
        std::cout << "### Exception ###" << std::endl << "  " << e.what() << std::endl;
    } catch (std::exception& e) {
        std::cout << "### Exception ###" << std::endl << "  " << e.what() << std::endl;
    } catch(...) {
        std::cout << "### Exception ###" << std::endl << "  default" << std::endl;
    }

    return 0;
}

namespace Nl {
namespace Examples {

// Recorder Example
void recorderCallback(u_int8_t *out,
                      const SampleSpecs &sampleSpecs,
                      SharedUserPtr ptr)
{
    static uint32_t total = 0;

    uint16_t c = 0;
    while (c < sampleSpecs.buffersizeInBytesPerPeriode) {
        c += write(fd, out+c, sampleSpecs.buffersizeInBytesPerPeriode-c);
        total+=c;
    }

    if (total >= 16777216) {

        close(fd);
        file_count++;
        total = 0;
        std::string f = fileName + std::to_string(file_count) + ".raw";
        fd = open(f.c_str(), O_WRONLY | O_CREAT, S_IWRITE | S_IREAD);
        if (fd < 0) {
            std::cout << "Can not create file: " << f << std::endl;
        }
    }
}

JobHandle recorder(const AlsaCardIdentifier &audioInCard,
                   unsigned int buffersize,
                   unsigned int samplerate,
                   int fd)
{
    JobHandle ret;

    ret.inBuffer = createBuffer("InputBuffer");
    ret.audioInput = createAlsaInputDevice(audioInCard, ret.inBuffer, buffersize);
    ret.audioInput->setSamplerate(samplerate);
    ret.audioInput->setChannelCount(2);
    ret.audioInput->setSampleFormat("S16_LE");

    // Start audio Thread
    ret.audioInput->start();

    // Register a Callback
    ret.workingThreadHandle = registerInputCallbackOnBuffer(ret.inBuffer, recorderCallback, nullptr);

    return ret;
}

} // namespace Nl
} // namespace Examples
