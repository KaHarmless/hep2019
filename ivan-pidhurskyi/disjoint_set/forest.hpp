#pragma once

#include <cassert>
#include <cstdlib>
#include <new>
#include <ostream>

namespace find_method
{
  struct naive {};
  struct path_compression {};
  struct path_halving {};
  struct path_splitting {};
};
namespace union_method
{
  struct naive {};
  struct by_rank {};
  struct by_size {};
};
template <typename  FindMethod,
          typename UnionMethod>
struct forest_traits
{
  typedef FindMethod find_type;
  typedef UnionMethod union_type;
};

// Basic fields for element with non-void identifier.
template <typename Id>
struct element_base
{
  size_t parent;
  Id name;
  element_base(size_t id, const Id& name)
  : parent { id },
    name { name }
  { }
};
// Specialization for elements without additional identifier.
template <>
struct element_base<void>
{
  size_t parent;
  element_base(size_t id)
  : parent { id }
  { }
};

template <typename>
struct element_union_part;
// Naive union don't need any additional fields.
template <>
struct element_union_part<union_method::naive> { };
// Union by rank or by size needs additional field.
template <>
struct element_union_part<union_method::by_rank> { size_t rank = 0; };
template <>
struct element_union_part<union_method::by_size> { size_t size = 1; };

template <typename Id, typename UnionMethod>
struct element: element_base<Id>, element_union_part<UnionMethod> {
  using element_base<Id>::element_base;
};

template <typename Id, typename Traits>
class forest
{
  public:
  using value_type = element<Id, typename Traits::union_type>;

  private:
  value_type* fels;
  size_t fsize;
  size_t fcap;

  public:
  explicit forest(size_t cap = 42)
  : fels { (value_type*)malloc(cap * sizeof(value_type)) },
    fsize { 0 },
    fcap { cap }
  {
    assert(fels);
  }

  forest(const forest& other)
  : fels { (value_type*)malloc(other.fcap * sizeof(value_type)) },
    fsize { other.fsize },
    fcap { other.fcap }
  {
    assert(fels);
    for (size_t i = 0; i < fsize; i++)
      fels[i] = other.fels[i];
  }

  forest(forest&& other)
  : fels { other.fels },
    fsize { other.fsize },
    fcap { other.fcap }
  {
    other.fels = nullptr;
    other.fsize = 0;
    other.fcap = 0;
  }
  ~forest() {
    if (!fsize)
      return;

    for (size_t i = 0; i < fsize; i++)
      fels[i].~value_type();
    free(fels);
  }

  public:
  template <typename T>
  size_t make_set(const T& name) {
    assert(fsize < fcap);
    new (fels + fsize) value_type { fsize, name };
    return fsize++;
  }
  size_t make_set() {
    assert(fsize < fcap);
    new (fels + fsize) value_type { fsize };
    return fsize++;
  }

  size_t parent(size_t id) const { return fels[id].parent; }
  size_t set_parent(size_t id, size_t p) { return fels[id].parent = p; }

  size_t find(size_t id);
  void join(size_t x, size_t y);

  template <typename T, typename U>
  friend std::ostream& operator << (std::ostream& , const forest<T, U>&);
  template <typename U>
  friend std::ostream& operator << (std::ostream& , const forest<void, U>&);
  template <typename Id_, typename Traits_>
  friend size_t __find(forest<Id_, Traits_>& f, size_t id, find_method::naive);
  template <typename Id_, typename Traits_>
  friend size_t __find(forest<Id_, Traits_>& f, size_t id, find_method::path_compression);
  template <typename Id_, typename Traits_>
  friend void __join(forest<Id_, Traits_>& f, size_t x, size_t y, union_method::by_rank);
  template <typename Id_, typename Traits_>
  friend void __join(forest<Id_, Traits_>& f, size_t x, size_t y, union_method::by_size);
};

template <typename Id, typename Traits>
size_t __find(forest<Id, Traits>& f, size_t id, find_method::naive)
{
  while (id != f.parent(id))
    id = f.parent(id);
  return id;
}
template <typename Id, typename Traits>
size_t __find(forest<Id, Traits>& f, size_t id, find_method::path_compression)
{ return f.set_parent(id, __find(f, id, find_method::naive { })); }
template <typename Id, typename Traits>
size_t __find(forest<Id, Traits>& f, size_t id, find_method::path_halving)
{
  while (id != f.parent(id)) {
    f.set_parent(id, f.parent(f.parent(id)));
    id = f.parent(id);
  }
  return id;
}
template <typename Id, typename Traits>
size_t __find(forest<Id, Traits>& f, size_t id, find_method::path_splitting)
{
  size_t tmp;
  while (id != f.parent(id)) {
    tmp = f.parent(id);
    f.set_parent(id, f.parent(tmp));
    id = tmp;
  }
  return id;
}
template <typename Id, typename Traits>
size_t forest<Id, Traits>::find(size_t x)
{ return __find(*this, x, typename Traits::find_type { }); }


template <typename Id, typename Traits>
void __join(forest<Id, Traits>& f, size_t x, size_t y, union_method::naive)
{
  typename Traits::find_type find_method;
  auto x_root = __find<Id, Traits>(f, x, find_method);
  auto y_root = __find<Id, Traits>(f, y, find_method);
  f.fels[y_root].parent = x_root;
}
template <typename Id, typename Traits>
void __join(forest<Id, Traits>& f, size_t x, size_t y, union_method::by_rank)
{
  typename Traits::find_type find_method;
  auto x_root = __find<Id, Traits>(f, x, find_method);
  auto y_root = __find<Id, Traits>(f, y, find_method);

  if (x_root == y_root)
    return;

  // if same ranks, then rank of resulting tree will be one larger
  if (f.fels[x_root].rank == f.fels[y_root].rank) {
    f.fels[y_root].parent = x_root;
    f.fels[x_root].rank ++;
    return;
  }

  // if ranks are different, then join tree with smaller rank
  // to the tree with larger rank.
  if (f.fels[x_root].rank > f.fels[y_root].rank)
    f.fels[y_root].parent = x_root;
  else
    f.fels[x_root].parent = y_root;

  return;
}
template <typename Id, typename Traits>
void __join(forest<Id, Traits>& f, size_t x, size_t y, union_method::by_size)
{
  typename Traits::find_type find_method;
  auto x_root = __find<Id, Traits>(f, x, find_method);
  auto y_root = __find<Id, Traits>(f, y, find_method);

  if (x_root == y_root)
    return;

  // x_root will be the new root, y_root is attached root
  //
  // swap roots according to size
  if (f.fels[x_root].size < f.fels[y_root].size) {
    size_t tmp = x_root;
    x_root = y_root;
    y_root = tmp;
  }

  // attach tree with smaller size to one with bigger size
  f.fels[y_root].parent = x_root;
  f.fels[x_root].size += f.fels[y_root].size;
}
template <typename Id, typename Traits>
void forest<Id, Traits>::join(size_t x, size_t y)
{ __join(*this, x, y, typename Traits::union_type { }); }

template <typename T, typename U>
std::ostream& operator << (std::ostream& os, const forest<T, U>& f)
{
  os << "digraph g {" << std::endl;
  //os << "\t graph [rankdir=LR];" << std::endl;
  for (size_t i = 0; i < f.fsize; i++) {
    if (f.fels[i].parent == i)
      //os << '\t' << f.fels[i].name;
      continue;
    else
      os << '\t' << f.fels[i].name << " -> " << f.fels[f.fels[i].parent].name;
  }
  return os << '}' << std::endl;
}

template <typename T>
std::ostream& operator << (std::ostream& os, const forest<void, T>& f)
{
  os << "digraph g {" << std::endl;
  //os << "\t graph [rankdir=LR];" << std::endl;
  for (size_t i = 0; i < f.fsize; i++) {
    if (f.fels[i].parent == i)
      //os << '\t' << i;
      continue;
    else
      os << '\t' << i << " -> " << f.fels[i].parent;
    os << ';' << std::endl;
  }
  return os << '}' << std::endl;
}

