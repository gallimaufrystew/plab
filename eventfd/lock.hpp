//
// Created by z on 1/28/16.
//
#pragma once

class SpinLock {
	volatile bool locked_ __attribute__((aligned(64)));
public :

	SpinLock() : locked_(false) { }

	void lock() {
		while(__sync_lock_test_and_set(&locked_, true)){
			;
		}
	}

	void unlock() {
		__sync_lock_release(&locked_);
	}

};


class ScopedLock {
private :
	SpinLock l_;
public :
	ScopedLock() {l_.lock(); }
	~ScopedLock() {l_.unlock(); }
};