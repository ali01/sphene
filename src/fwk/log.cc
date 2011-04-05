#include "log.h"
#include <string>


namespace Fwk {

/* For 'log << "foo" << ...' convenience. */
Log& operator<<(Log& log, const std::string& str) {
  log.entryNew(str);
  return log;
}

Log::Ptr operator<<(Log::Ptr log, const std::string& str) {
  log->entryNew(str);
  return log;
}

Log::LogStream::Ptr operator<<(Log::LogStream::Ptr logstream,
                               const std::string& str) {
  logstream->stream() << str;
  return logstream;
}

}  /* Fwk */
