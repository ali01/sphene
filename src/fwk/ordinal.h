/* Inspired by David Cheriton's Advanced Object Oriented Programming from a
   Modeling & Simulation's Perspective ~ Chapter 16: Value-Oriented
   Programming & Value Types, Numerical class implementation. */

#ifndef ORDINAL_H_G09I1KEU
#define ORDINAL_H_G09I1KEU

namespace Fwk {

template<typename UnitType, typename RepType>
class Ordinal {
public:
  /* constructors */
  Ordinal() {
    valueIs(0);
  }

  Ordinal(RepType v) {
    valueIs(v);
  }

  Ordinal(const Ordinal<UnitType,RepType>& v) {
    valueIs(v.value_);
  }

  virtual ~Ordinal() {}

  RepType value() const {
    return value_;
  }

  /* assignment */
  const Ordinal<UnitType,RepType>&
    operator=(const Ordinal<UnitType,RepType>& v);

  const Ordinal<UnitType,RepType>&
    operator=(const RepType& v);

  /* relational operators */

  bool operator==(const Ordinal<UnitType,RepType>& v) const;
  bool operator!=(const Ordinal<UnitType,RepType>& v) const;
  bool operator>=(const Ordinal<UnitType,RepType>& v) const;
  bool operator<=(const Ordinal<UnitType,RepType>& v) const;
  bool operator< (const Ordinal<UnitType,RepType>& v) const;
  bool operator> (const Ordinal<UnitType,RepType>& v) const;

  bool operator==(const RepType& v) const;
  bool operator!=(const RepType& v) const;
  bool operator>=(const RepType& v) const;
  bool operator<=(const RepType& v) const;
  bool operator< (const RepType& v) const;
  bool operator> (const RepType& v) const;

  template<typename U, typename R>
  friend bool operator==(const R& left, const Ordinal<U,R>& right);

  template<typename U, typename R>
  friend bool operator!=(const R& left, const Ordinal<U,R>& right);

  template<typename U, typename R>
  friend bool operator>=(const R& left, const Ordinal<U,R>& right);

  template<typename U, typename R>
  friend bool operator<=(const R& left, const Ordinal<U,R>& right);

  template<typename U, typename R>
  friend bool operator<(const R& left, const Ordinal<U,R>& right);

  template<typename U, typename R>
  friend bool operator>(const R& left, const Ordinal<U,R>& right);

protected:
  /* member functions */
  virtual void valueIs(RepType v) {
    value_ = v;
  }

  virtual bool equal(RepType v) const {
    return value_ == v;
  }

  /* data members */
  RepType value_;
};

/* assignment */
template<typename U, typename RepType>
inline const Ordinal<U,RepType>&
Ordinal<U,RepType>::operator=(const Ordinal<U,RepType>& v) {
  if (this != &v)
    valueIs(v.value_);

  return *this;
}

template<typename U, typename RepType>
inline const Ordinal<U,RepType>&
Ordinal<U,RepType>::operator=(const RepType& v) {
  valueIs(v);
  return *this;
}


/* relational operators */

template<typename U, typename R>
inline bool
Ordinal<U,R>::operator==(const Ordinal<U,R>& v) const {
  return equal(v.value());
}

template<typename U, typename R>
inline bool
Ordinal<U,R>::operator!=(const Ordinal<U,R>& v) const {
  return !equal(v.value());
}

template<typename U, typename R>
inline bool
Ordinal<U,R>::operator<=(const Ordinal<U,R>& v) const {
  return value_ <= v.value_;
}

template<typename U, typename R>
inline bool
Ordinal<U,R>::operator>=(const Ordinal<U,R>& v) const {
  return value_ >= v.value_;
}

template<typename U, typename R>
inline bool
Ordinal<U,R>::operator<(const Ordinal<U,R>& v) const {
  return value_ < v.value_;
}

template<typename U, typename R>
inline bool
Ordinal<U,R>::operator>(const Ordinal<U,R>& v) const {
  return value_ > v.value_;
}

template<typename U, typename R>
inline bool
Ordinal<U,R>::operator==(const R& v) const {
  return equal(v);
}

template<typename U, typename R>
inline bool
Ordinal<U,R>::operator!=(const R& v) const {
  return !equal(v);
}

template<typename U, typename R>
inline bool
Ordinal<U,R>::operator>=(const R& v) const {
  return value_ >= v;
}

template<typename U, typename R>
inline bool
Ordinal<U,R>::operator<=(const R& v) const {
  return value_ <= v;
}

template<typename U, typename R>
inline bool
Ordinal<U,R>::operator<(const R& v) const {
  return value_ < v;
}

template<typename U, typename R>
inline bool
Ordinal<U,R>::operator>(const R& v) const {
  return value_ > v;
}

template<typename U, typename R>
inline bool
operator==(const R& left, const Ordinal<U,R>& right) {
  return right.equal(left);
}

template<typename U, typename R>
inline bool
operator!=(const R& left, const Ordinal<U,R>& right) {
  return !right.equal(left);
}

template<typename U, typename R>
inline bool
operator>=(const R& left, const Ordinal<U,R>& right) {
  return left >= right.value_;
}

template<typename U, typename R>
inline bool
operator<=(const R& left, const Ordinal<U,R>& right) {
  return left <= right.value_;
}

template<typename U, typename R>
inline bool
operator<(const R& left, const Ordinal<U,R>& right) {
  return left < right.value_;
}

template<typename U, typename R>
inline bool
operator>(const R& left, const Ordinal<U,R>& right) {
  return left > right.value_;
}

} /* end of namespace Fwk */

#endif
