#include "common/controlinterface.h"

#include <iostream>
#include <thread>
#include <sstream>
#include <vector>
#include <algorithm>
#include <iterator>
#include <sstream>

#include <sys/types.h>
#include <sys/socket.h> /* For accept */
#include <unistd.h> /* For read, errno, STDIN_FILENO */
#include <limits.h> /* For PIPE_BUF */

#include<arpa/inet.h>
#include <sys/un.h>

namespace Nl {

/** \ingroup Tools
 *
 * \brief Constructor
 * \param func Function name
 * \param file File name
 * \param line Line number
 * \param errorNumber Alsa error number
 * \param what Descriptive string
 *
*/
ControlInterfaceException::ControlInterfaceException(std::string func, std::string file, int line, int errorNumber, std::string what) :
    m_func(func),
    m_file(file),
    m_msg(what),
    m_line(line),
    m_errno(errorNumber) {}

/** \ingroup Tools
 *
 * \brief Returns the desciptive string of the exception
 * \return A deciptive string of the exception
*/
const char* ControlInterfaceException::what() const noexcept
{
    std::stringstream ss;
    ss << m_file << ":" << m_func << ":" << m_line << ": " << m_msg;
    return ss.str().c_str();
}

std::ostream& operator<<(std::ostream& lhs, CommandDescriptor const& rhs)
{
    lhs << rhs.cmd;
    return lhs;
}

ControlInterface::ControlInterface(JobHandle jobHandle) :
    m_isRunning(false),
    m_terminateRequest(false),
    m_thread(nullptr),
    m_jobHandle(jobHandle)
{
    // Add default commands
    Nl::CommandDescriptor cmdHelp;
    cmdHelp.cmd = "help";
    cmdHelp.func = ControlInterface::help;
    addCommand(cmdHelp);
}

void ControlInterface::start()
{
    if (!m_isRunning) {
        m_terminateRequest.store(false);
        m_thread = new std::thread(&ControlInterface::run, this);
        m_isRunning = true;
    }
}

void ControlInterface::stop()
{
    if (m_isRunning) {
        m_terminateRequest.store(true);
        m_thread->join();
        delete m_thread;
        m_thread = nullptr;
        m_isRunning = false;
    }
}

void ControlInterface::addCommand(const CommandDescriptor &cd)
{
    m_commands.push_back(CommandDescriptor(cd));
}

//static
void ControlInterface::help(std::vector<std::string> args, JobHandle jobHandle, int sockfd, ControlInterface *ptr)
{
    std::string msg = "Available Commands:\n\n";
    write(sockfd, msg.c_str(), msg.size());

    std::stringstream s;
    std::copy(ptr->m_commands.begin(), ptr->m_commands.end(), std::ostream_iterator<CommandDescriptor>(s, "\n"));
    write(sockfd, s.str().c_str(), s.str().length());
}

// Static
void ControlInterface::run(ControlInterface *ptr)
{
    //Create socket
    int sockfd = ::socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd < 0) {
        ptr->m_terminateRequest.store(true);
        throw ControlInterfaceException(__PRETTY_FUNCTION__, __FILE__, __LINE__, errno, "Can not create to socket");
    }

    union {
        struct sockaddr sa;
        struct sockaddr_un un;
    } sa;

    memset(&sa, 0, sizeof(sa));
    sa.un.sun_family = AF_UNIX;
    strncpy(sa.un.sun_path, "/tmp/nlaudio.sock", sizeof(sa.un.sun_path));

    if (::bind(sockfd, &sa.sa, sizeof(sa)) < 0) {
        ptr->m_terminateRequest.store(true);
        close(sockfd);
        throw ControlInterfaceException(__PRETTY_FUNCTION__, __FILE__, __LINE__, errno, "Can not bind to socket");
    }

    if(::listen(sockfd, 5) < 0) {
        ptr->m_terminateRequest.store(true);
        throw ControlInterfaceException(__PRETTY_FUNCTION__, __FILE__, __LINE__, errno, "Can not listen to socket");
    }

    timeval tv;
    fd_set fd;

    while(!ptr->m_terminateRequest.load()) {

        int ret = 0;
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        FD_ZERO(&fd);
        FD_SET(sockfd, &fd);

        if ((ret = select(sockfd+1, &fd, NULL, NULL, &tv)) == 0) {
            continue;
        } else if (ret < 0) {
            ptr->m_terminateRequest.store(true);
            std::cerr << "Error in select: " << ::strerror(ret) << std::endl;
        }

        int fd = TEMP_FAILURE_RETRY(accept(sockfd, NULL, NULL));
        if (fd < 0) {
            ptr->m_terminateRequest.store(true);
            std::cerr << "Can not listen to socket: " << ::strerror(errno) << std::endl;
            continue;
        }

        ControlInterface::handleRequest(fd, ptr);
        TEMP_FAILURE_RETRY(close(fd));
    }

    ::unlink("/tmp/nlaudio.sock");
    ::close(sockfd);
}

// Static
std::string ControlInterface::read(int fd)
{
    std::string result;
    result.resize(1024);

    int ret = TEMP_FAILURE_RETRY(::recv(fd, &result[0], result.size(), MSG_PEEK|MSG_TRUNC));
    if (ret <= 0)
        throw ControlInterfaceException(__PRETTY_FUNCTION__, __FILE__, __LINE__, errno, "Can not read socket");

    return result;
}

// Static
void ControlInterface::handleRequest(int fd, ControlInterface *ptr)
{
    std::string request = ControlInterface::read(fd);
    std::string cmd = "";

    std::istringstream f(request);
    std::vector<std::string> tokens;

    copy(std::istream_iterator<std::string>(f),
         std::istream_iterator<std::string>(),
         std::back_inserter(tokens));

    if (tokens.size() == 0)
        return;

    cmd = tokens[0];
    tokens.erase(tokens.begin());

    // Todo: access over ptr must be mutexed!
    for (auto i = ptr->m_commands.begin(); i != ptr->m_commands.end(); ++i) {
        if (i->cmd == cmd) {
            i->func(tokens, ptr->m_jobHandle, fd, ptr);
            return;
        }
    }

    std::string ret = "Unknown command!\n";
    write(fd, ret.c_str(), ret.size());
}

} // namespace Nl
