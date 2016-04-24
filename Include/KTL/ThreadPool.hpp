//#ifndef THREAD_POOL_H
//#define THREAD_POOL_H
//
//#include <vector>
//#include <queue>
//#include <memory>
//#include <thread>
//#include <mutex>
//#include <condition_variable>
//#include <future>
//#include <functional>
//#include <stdexcept>
//
//namespace std {
//
//class thread_pool {
//public:
//  thread_pool( size_t );
//
//  template<class F, class... Args>
//  auto enqueue( F&& f, Args&&... args )->future<typename result_of<F( Args... )>::type>;
//
//  ~thread_pool();
//
//  size_t  size();
//private:
//  // need to keep track of threads so we can join them
//  vector< thread > workers;
//  // the task queue
//  queue< function<void()> > tasks;
//
//  // synchronization
//  mutex queue_mutex;
//  condition_variable condition;
//  bool stop;
//};
//
//class single_thread_pool : public thread_pool {
//public:
//	single_thread_pool() : thread_pool(1)
//	{
//	}
//
//	explicit single_thread_pool(string const &name) : thread_pool(1), thread_name(name)
//	{
//	}
//
//	string & name() { return thread_name; }
//private:
//	string thread_name;
//};
//
//
//// the constructor just launches some amount of workers
//inline thread_pool::thread_pool( size_t threads )
//: stop( false )
//{
//  for ( size_t i = 0; i<threads; ++i )
//    workers.emplace_back(
//    [this]
//  {
//    for ( ;; )
//    {
//      unique_lock<mutex> lock( this->queue_mutex );
//      while ( !this->stop && this->tasks.empty() )
//        this->condition.wait( lock );
//      if ( this->stop && this->tasks.empty() )
//        return;
//      function<void()> task( this->tasks.front() );
//      this->tasks.pop();
//      lock.unlock();
//      task();
//    }
//  }
//  );
//}
//
//inline size_t thread_pool::size()
//{
//    unique_lock<mutex> lock( this->queue_mutex );
//    return tasks.size();
//}
//
//// add new work item to the pool
//template<class F, class... Args>
//auto thread_pool::enqueue( F&& f, Args&&... args )
//-> future<typename result_of<F( Args... )>::type>
//{
//  typedef typename result_of<F( Args... )>::type return_type;
//
//  // don't allow enqueueing after stopping the pool
//  if ( stop )
//    throw runtime_error( "enqueue on stopped thread_pool" );
//
//  auto task = make_shared< packaged_task<return_type()> >(
//    bind( forward<F>( f ), forward<Args>( args )... )
//    );
//
//  future<return_type> res = task->get_future();
//  {
//    unique_lock<mutex> lock( queue_mutex );
//    tasks.push( [task] (){ (*task)(); } );
//  }
//  condition.notify_one();
//  return res;
//}
//
//// the destructor joins all threads
//inline thread_pool::~thread_pool()
//{
//  {
//    unique_lock<mutex> lock( queue_mutex );
//    stop = true;
//  }
//  condition.notify_all();
//  for ( size_t i = 0; i<workers.size(); ++i )
//    workers[ i ].join();
//}
//
//}
//#endif
