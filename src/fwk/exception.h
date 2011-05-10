/** \file exception.h
 * Exception classes.
 * \author Matt Sparks
 */
#ifndef __FWK__EXCEPTION_H__
#define __FWK__EXCEPTION_H__

#include <string>
#include <cstring>
#include <cerrno>

#include "named_interface.h"

namespace Fwk {

class Exception {
public:
  Fwk::NamedInterface *entity() const { return entity_; }
  std::string funcName() const { return funcName_; }
  std::string message() const { return message_; }

protected:
  Fwk::NamedInterface *entity_;
  std::string funcName_;
  std::string message_;

  Exception(const std::string& funcName, const std::string& message)
      : entity_(NULL), funcName_(funcName), message_(message) { }

  Exception(Fwk::NamedInterface *entity, const std::string& funcName,
            const std::string& message)
      : entity_(entity), funcName_(funcName), message_(message) { }
};


class RangeException : public Exception {
public:
  RangeException(const std::string& funcName, const std::string& message)
      : Exception(funcName, message) { }
  RangeException(Fwk::NamedInterface *entity, const std::string& funcName,
                 const std::string& message)
      : Exception(entity, funcName, message) { }
};


class ResourceException : public Exception {
public:
  ResourceException(const std::string& funcName, const std::string& message)
      : Exception(funcName, message) { }
  ResourceException(Fwk::NamedInterface *entity, const std::string& funcName,
                    const std::string& message)
      : Exception(entity, funcName, message) { }
};


class NameInUseException : public ResourceException {
public:
  NameInUseException(const std::string& funcName, const std::string& message)
      : ResourceException(funcName, message) { }
  NameInUseException(Fwk::NamedInterface *entity, const std::string& funcName,
                     const std::string& message)
      : ResourceException(entity, funcName, message) { }
};


class TimeoutException : public ResourceException {
public:
  TimeoutException(const std::string& funcName, const std::string& message)
      : ResourceException(funcName, message) { }
  TimeoutException(Fwk::NamedInterface *entity, const std::string& funcName,
                   const std::string& message)
      : ResourceException(entity, funcName, message) { }
};


class NotImplementedException : public Exception {
public:
  NotImplementedException(const std::string& funcName,
                          const std::string& message)
      : Exception(funcName, message) { }

  NotImplementedException(Fwk::NamedInterface *entity,
                          const std::string& funcName,
                          const std::string& message)
      : Exception(entity, funcName, message) { }
};

}

#endif
