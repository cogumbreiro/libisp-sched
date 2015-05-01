#ifndef _MATCHER_HPP
#define _MATCHER_HPP

#include <set>

#include "CB.hpp"
#include "Transition.hpp"

class Matcher {
public:
  virtual ~Matcher(){}
  //virtual bool isMatched(const Transition &) const = 0;
  //virtual optional<shared_ptr<Transition> > findLastMatched(int) const = 0;
  virtual const std::set<shared_ptr<Transition> > & findMatchedSends(const Transition &) const = 0;
};
#endif
