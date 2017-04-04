#include "rw_lock.h"

void rw_lock::writer_lock() {
	std::unique_lock<std::mutex> unique_guard(_guard);
    while (_num_readers > 0 || _num_writers > 0) {
        // while there are readers OR writers, wait.
        _waiting_writers.wait(unique_guard);
    } 
    assert(_num_writers == 0);
    assert(_num_readers == 0);
    ++_num_writers;
    assert(_num_writers == 1);
    unique_guard.unlock();  // unlock guard
} // end writer_lock

void rw_lock::writer_unlock() {
    std::unique_lock<std::mutex> unique_guard(_guard);
    assert(_num_writers == 1);
	--_num_writers;
    assert(_num_writers == 0);

    // TODO: might be inefficient
    _waiting_readers.notify_all();
    _waiting_writers.notify_one();
    unique_guard.unlock();
} // end writer_unlock

void rw_lock::reader_lock() {
    std::unique_lock<std::mutex> unique_guard(_guard);
	while (_num_writers > 0) {
        _waiting_readers.wait(unique_guard);
    }
    assert(_num_writers == 0);
    ++_num_readers;
    unique_guard.unlock();
} // end reader_unlock

void rw_lock::reader_unlock() {
    std::unique_lock<std::mutex> unique_guard(_guard);
    assert(_num_writers == 0);
    --_num_readers;
    if (_num_readers == 0) {
    	_waiting_writers.notify_one();
    }
    unique_guard.unlock();
} // end reader_unlock

unsigned rw_lock::print_num_writers() {
   return _num_writers;
}
