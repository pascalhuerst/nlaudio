#pragma once

#include <mutex>
#include <condition_variable>
#include <atomic>
#include <iostream>
#include <cmath>
#include <cstring>

#include "audio/samplespecs.h"

namespace Nl {

/** \ingroup Audio
 *
 * \brief A blocking circular buffer implementation
 * \tparam Type of buffer elements
 * \param size Buffersize in elements of type T
 *
 * A circular buffer implementation which blocks the callee of get()
 * if nothing to read and the callee of set() when no space
 * to write
 *
*/
template <typename T>
class BlockingCircularBuffer
{
public:
	/** \ingroup Audio
	 *
	 * \brief Constructor
	 * \tparam T element type
	 * \param size Buffersize in elements
	 *
	 * Sets up a new buffer.
	 *
	*/
    BlockingCircularBuffer(const std::string& name) :
        m_buffer(nullptr),
        m_size(0),
        m_name(name),
        m_bytesRead(0),
        m_bytesWritten(0),
        m_readIndex(0),
        m_writeIndex(0)
    {}

    ~BlockingCircularBuffer()
    {
        if (m_buffer)
            delete[] m_buffer;
    }

    void init(int size)
    {
        std::unique_lock<std::mutex> mlock(m_mutex);

        if (m_buffer)
            delete [] m_buffer;

        m_size = size;

		std::cout << m_size << std::endl;

		m_buffer = new T[m_size];
        memset(m_buffer, 0, m_size);

        m_bytesRead = 0;
        m_bytesWritten = 0;
        m_readIndex = 0;
        m_writeIndex = 0;

        m_condition.notify_one();
    }

    // Not sure if this is proper. This class should not know something about SampleSpecs.
	void init(const SampleSpecs &sampleSpecs)
    {
        init(sampleSpecs.buffersizeInBytes);
        m_sampleSpecs = sampleSpecs;
    }

	/** \ingroup Audio
	 *
	 * \brief Read data from the buffer
	 * \param buffer Buffer to save data to
	 * \param size Buffersize int bytes
	 *
	 * Reads size bytes into buffer. Blocks callee if
	 * less data available than requested.
	 *
	*/
	void get(T *buffer, unsigned int size)
    {
        if (!m_buffer) {
            std::cout << "Buffer (" << m_name << ") not initialized!" << std::endl;
        }

        std::unique_lock<std::mutex> mlock(m_mutex);
        while (availableToRead() < size || !m_buffer) {
            m_condition.wait(mlock);
        }

        m_bytesRead += size;

        for (unsigned int i=0; i<size; i++) {
            m_readIndex++;
            m_readIndex = m_readIndex % m_size;
            buffer[i] = m_buffer[m_readIndex];
        }

        m_condition.notify_one();
    }

	/** \ingroup Audio
	 *
	 * \brief Write data to the buffer
	 * \param buffer Buffer to read data from
	 * \param size Buffersize int bytes
	 *
	 * Writes size bytes into buffer. Blocks callee if
	 * less space available than requested. Wakes waiting
	 * readers.
	 *
	*/
	void set(T *buffer, unsigned int size)
    {
        if (!m_buffer) {
            std::cout << "Buffer (" << m_name << ") not initialized!" << std::endl;
        }

        std::unique_lock<std::mutex> mlock(m_mutex);
		while (availableToWrite() < size || !m_buffer) {
            m_condition.wait(mlock);
        }

        m_bytesWritten += size;

        for (unsigned int i=0; i<size; i++) {
            m_writeIndex++;
            m_writeIndex = m_writeIndex % m_size;
            m_buffer[m_writeIndex] = buffer[i];
        }

        m_condition.notify_one();
    }

	/** \ingroup Audio
	 *
	 * \brief Returns information on read/write cycles on the buffer
	 * \param readBytes Total bytes read from buffer
	 * \param writtenBytes Total writes to buffer
	 *
	 * Returns how many write and how man read cycles have been performed
	 * on the buffer.
	*/
    void getStat(unsigned long *readBytes, unsigned long *writtenBytes) const
    {
        *readBytes = m_bytesRead;
        *writtenBytes = m_bytesWritten;
    }

	/** \ingroup Audio
	 *
	 * \brief Returns number of available elements to read.
	 * \return A number of available elements to read.
	 *
	 * Returns how many elements can be read from the buffer,
	 * before the callee is blocked.
	*/
    inline unsigned int availableToRead() const
    {
        return m_size - availableToWrite();
    }

	/** \ingroup Audio
	 *
	 * \brief Returns number of available elements to write.
	 * \return A number of available elements to write.
	 *
	 * Returns how many elements can be written to the buffer,
	 * before the callee is blocked.
	*/
    inline unsigned int availableToWrite() const
    {
        int tmp = m_readIndex - m_writeIndex;
        int distance = tmp < 0 ? -tmp : tmp;

        return m_writeIndex < m_readIndex ? distance : m_size - distance;
    }

	/** \ingroup Audio
	 *
	 * \brief Returns the size of the buffer in elements.
	 * \return The size of the buffer in elements.
	 *
	 * Returns the total size in elements of the buffer.
	*/
    inline int size() const
    {
        return m_size;
    }

	/** \ingroup Audio
	 *
	 * \brief Returns the name of the buffer.
	 * \return The name of the Buffer.
	 *
	 * Returns the name of the buffer.
	*/
    inline std::string name() const
    {
        return m_name;
    }

	/** \ingroup Audio
	 *
	 * \brief Returns the SampleSpecs_t of the buffer.
	 * \return SampleSpecs_t which has been used to initialize this buffer.
	 *
	 * Returns the SampleSpecs_t which has been used to initialize this buffer.
	*/
	inline SampleSpecs sampleSpecs()
    {
        return m_sampleSpecs;
    }

private:
    T *m_buffer;
    std::atomic<int> m_size;
    std::string m_name;

    std::mutex m_mutex;
    std::condition_variable m_condition;

    std::atomic<unsigned long> m_bytesRead;
    std::atomic<unsigned long> m_bytesWritten;

    std::atomic<unsigned int> m_readIndex;
    std::atomic<unsigned int> m_writeIndex;

	SampleSpecs m_sampleSpecs;
};

/*! A shared handle to a \ref BlockingCircularBuffer<uint8_t> */
typedef std::shared_ptr<BlockingCircularBuffer<uint8_t>> SharedBufferHandle;

} // namespace Nl
