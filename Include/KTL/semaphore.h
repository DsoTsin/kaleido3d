#pragma once 
#include <mutex>
#include <condition_variable>

namespace std {
  // reference: 
  class semaphore{
  private:
    mutex mtx;
    condition_variable cv;
    int count;

  public:
    semaphore( int count_ = 0 ) :count( count_ ){ ; }
    void notify()
    {
      unique_lock<mutex> lck( mtx );
      ++count;
      cv.notify_one();
    }
    void wait()
    {
      unique_lock<mutex> lck( mtx );

      while ( count == 0 ){
        cv.wait( lck );
      }
      count--;
    }
  };
}