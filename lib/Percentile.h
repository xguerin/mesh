#pragma once

#include <algorithm>
#include <vector>

namespace mesh {

template<typename T>
class Percentile
{
 public:

  using Data = std::vector<T>;

  Percentile(Data const & data)
    : m_data(data)
  {
    std::sort(m_data.begin(), m_data.end(), std::less<T>());
  }

  void
  print(std::ostream & out, const size_t v)
  {
    out << v << ":" << get(v) << std::endl;
  }

  template<typename...Args>
  void
  print(std::ostream & out, const size_t v, Args...args)
  {
    out << v << ":" << get(v) << ", ";
    print(out, args...);
  }

 private:

  T get(const size_t percentile) const
  {
    size_t index = percentile * m_data.size() / 100;
    return m_data[index];
  }

  Data m_data;
};

}
