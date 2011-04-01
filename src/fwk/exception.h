/* Copyright (c) 2008-2010. Ali H. Yahya, All rights reserved. */
/* Adapted from David R. Cheriton's Advanced Object Oriented Programming from a
   Modeling & Simulation's Perspective ~ Chapter 7: Exceptions & Exception 
   Handling */

#pragma once
#include <string>
#include <iostream>
#include <sstream>
#include <stdint.h>
using std::stringstream;
using std::ostream;
using std::string;
using std::cerr;
using std::endl;

namespace Fwk {

/* forward declarations */
class Exception;
ostream& operator<<(ostream& out, const Exception& ex);

class Exception {
public:
  virtual ~Exception() {}
  
  string filename() const { return filename_; }
  uint32_t line() const { return line_; }

  string msg() const { return msg_; }

protected:
  Exception(string filename, uint32_t line, string msg) :
    filename_(filename), line_(line), msg_(msg)
  {
    #ifdef __DEBUG__
      cerr << *this;
    #endif
  }

private:
  /* data members */
  string filename_;
  uint32_t line_;

  string msg_;
};

class IncompleteInitializationException : public Exception {
public:
  IncompleteInitializationException(string filename, uint32_t ln, string msg):
    Exception(filename, ln, msg) {}
};

class NoImplementationException : public Exception {
public:
  NoImplementationException(string filename, uint32_t line, string msg) :
    Exception(filename, line, msg) {}
};

class AttributeNotSupportedException : public NoImplementationException {
public:
  AttributeNotSupportedException(string filename, uint32_t line, string msg) :
    NoImplementationException(filename, line, msg) {}
};

class UnknownTypeException : public Exception {
public:
  UnknownTypeException(string filename, uint32_t line, string msg) :
    Exception(filename, line, msg) {}
};

class UnknownAttrException : public Exception {
public:
  UnknownAttrException(string filename, uint32_t line, string msg) :
    Exception(filename, line, msg) {}
};

class UnknownDelimiterException : public Exception {
public:
  UnknownDelimiterException(string filename, uint32_t line, string msg) :
    Exception(filename, line, msg) {}
};

class UnknownArgException : public Exception {
public:
  UnknownArgException(string filename, uint32_t line, string msg) :
    Exception(filename, line, msg) {}
};


class RangeException : public Exception {
public:
  RangeException(string filename, uint32_t line, string msg) :
    Exception(filename, line, msg) {}
};

class MemoryException : public Exception {
 public:
  MemoryException(string filename, uint32_t line, string msg) :
    Exception(filename, line, msg) {}
};

class StorageException : public Exception {
 public:
  StorageException(string filename, uint32_t line, string msg) :
    Exception(filename, line, msg) {}
};

class NameInUseException : public Exception {
public:
  NameInUseException(string filename, uint32_t line, string msg) :
    Exception(filename, line, msg) {}

};

class IllegalNameException: public Exception {
 public:
  IllegalNameException(string filename, uint32_t line, string msg) :
    Exception(filename, line, msg) {}

};

class EntityNotFoundException : public Exception {
public:
  EntityNotFoundException(string filename, uint32_t line, string msg) :
    Exception(filename, line, msg) {}

};


class MemoryLimitExceededException : public MemoryException {
public:
  MemoryLimitExceededException(string filename, uint32_t line, string msg) :
    MemoryException(filename, line, msg) {}

};

class ReadOnlyException : public Exception {
public:
  ReadOnlyException(string filename, uint32_t line, string msg) :
    Exception(filename, line, msg) {}

};

class NetworkException : public Exception {
public:
  NetworkException(string filename, uint32_t line, string msg) :
    Exception(filename, line, msg) {}
};

class InvalidFormattingException: public Exception {
public:
  InvalidFormattingException(string filename, uint32_t line, string msg) :
    Exception(filename, line, msg) {}

};

class InvalidOperationException: public Exception {
public:
  InvalidOperationException(string filename, uint32_t line, string msg) :
    Exception(filename, line, msg) {}

};

class InvalidValueException: public Exception {
public:
  InvalidValueException(string filename, uint32_t line, string msg) :
    Exception(filename, line, msg) {}
};

class NullPointerException : public InvalidValueException {
public:
  NullPointerException(string filename, uint32_t line, string msg) :
    InvalidValueException(filename, line, msg) {}
};

inline ostream&
operator<<(ostream& out, const Exception& ex) {
  out << "exception thrown at ";
  out << ex.filename() << ":" << ex.line() << " - " << ex.msg();
  return out << endl;
}

} /* end of namespace Simone */
