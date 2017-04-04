#ifndef RW_LOCK_H_
#define RW_LOCK_H_

#include <cassert>
#include <condition_variable>
#include <mutex>

class rw_lock {
private:
	unsigned int _num_writers = 0;
	unsigned int _num_readers = 0;
	std::mutex _guard;
	std::condition_variable _waiting_writers;
	std::condition_variable _waiting_readers;
public:
	void writer_lock();
	void writer_unlock();
	void reader_lock();
	void reader_unlock();
	unsigned print_num_writers();
}; // end rwLock

#endif	// RW_LOCK_H_