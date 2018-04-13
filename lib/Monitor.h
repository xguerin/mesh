#pragma once

#include <atomic>
#include <list>
#include <memory>
#include <mutex>
#include <ostream>
#include <thread>
#include <string>
#include <vector>

namespace mesh {

class Monitor
{
 public:

  class Watcher
  {
   public:

    using Ref = std::unique_ptr<Watcher>;

    virtual ~Watcher() { }

    virtual void print(std::ostream & out) = 0;
  };

  template<typename T> class Variable : public Watcher
  {
   public:

    Variable(std::string const & name, T const & ref, const bool delta)
      : m_name(name), m_ref(ref), m_last(ref), m_delta(delta) { }

    void print(std::ostream & out)
    {
      if (m_delta) {
        out << m_name << ": " << m_ref - m_last << std::endl;
        m_last = m_ref;
      }
      else {
        out << m_name << ": " << m_ref << std::endl;
      }
    }

   private:

    std::string m_name;
    T const &   m_ref;
    T           m_last;
    bool        m_delta;
  };

  Monitor(const int sec);
  ~Monitor();

  template<typename T>
  void
  addVariable(std::string const & name, T const & ref, const bool delta = false)
  {
    std::lock_guard<std::mutex> _(m_mutex);
    m_watchers.push_back(Watcher::Ref(new Variable<T>(name, ref, delta)));
  }

 private:

  void run();

  int                     m_sec;
  int                     m_fd;
  std::atomic<bool>       m_shutdown;
  std::thread             m_thread;
  std::list<Watcher::Ref> m_watchers;
  std::mutex              m_mutex;
};

}
