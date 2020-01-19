#ifndef THREAD_HPP
#define THREAD_HPP

#include <string>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

class Lockable {

protected : // HACK for Waitable::wait()

   mutable std::mutex mutex_;

public :

   void lock   () const;
   void unlock () const;
};

class Waitable : public Lockable {

private :

   std::condition_variable_any cond_;

public :

   void wait   ();
   void signal ();
};

void listen_input();
bool get_line(std::string & line);

void test_bit();

#endif