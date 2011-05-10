/** \file log.h
 * Logging class.
 * \author Matt Sparks
 */
#ifndef __FWK__LOG_H_
#define __FWK__LOG_H_

#include <iostream>
#include <string>
#include <sstream>
#include <ctime>

#include "exception.h"
#include "ptr.h"
#include "ptr_interface.h"

// Convenience macros for LogStreams. Assumes Log object is 'log_'.
// TODO(ms): Maybe these should be streams a la 'cout', 'cerr'.
#define LOG  (*log_)()                 // default level
#define DLOG (*log_)(log_->debug())    // debug level
#define ILOG (*log_)(log_->info())     // info level
#define WLOG (*log_)(log_->warning())  // warning level
#define ELOG (*log_)(log_->error())    // error level
#define CLOG (*log_)(log_->clog())     // critical level


namespace Fwk {

// TODO(ms): A lot of this code should go into log.cc.

static const char *levelNames[] = {
  "debug",
  "info",
  "warning",
  "error",
  "critical"
};

class Log : public Fwk::PtrInterface<Log> {
public:
  typedef Fwk::Ptr<Log const> PtrConst;
  typedef Fwk::Ptr<Log> Ptr;

  enum Level {
    debug_,
    info_,
    warning_,
    error_,
    critical_
  };

  class LogStream : public Fwk::PtrInterface<LogStream> {
   public:
    typedef Fwk::Ptr<LogStream> Ptr;
    static Ptr LogStreamNew(Log::Ptr log, Level level) {
      return new LogStream(log, level);
    }

    std::stringstream& stream() { return ss_; }

   protected:
    LogStream(Log::Ptr log, Level level) : log_(log), level_(level) { }
    ~LogStream() {
      log_->entryNew(level_, ss_.str());
    }

    Log::Ptr log_;
    Level level_;
    std::stringstream ss_;
  };

  static Log::Ptr LogNew(const std::string& loggerName="root") {
    if (rootLog == NULL)
      rootLog = new Log("root");
    if (loggerName == "root")
      return rootLog;

    return new Log(loggerName);
  }

  Level debug() { return debug_; }
  Level info() { return info_; }
  Level warning() { return warning_; }
  Level error() { return error_; }
  Level critical() { return critical_; }

  std::string levelName(Level level) { return levelNames[level]; }

  inline Level level() const { return logLevel_; }
  void levelIs(Level level) {
    if (name() == "root")
      logLevel_ = level;
    else
      rootLog->levelIs(level);
  }

  inline std::string name() const { return loggerName_; }
  void nameIs(const std::string& name) { loggerName_ = name; }

  LogStream::Ptr operator()(Level level) {
    return LogStream::LogStreamNew(this, level);
  }
  LogStream::Ptr operator()() {
    return LogStream::LogStreamNew(this, level());
  }

  virtual void entryNew(Level level, Fwk::NamedInterface *entity,
                        std::string funcName, std::string cond) {
    if (level < rootLog->level())
      return;

    std::cout << timestamp() << " [" << levelName(level) << "] ";
    if (name() != "root")
      std::cout << name() << ": ";
    if (entity != NULL && funcName.size() > 0)
      std::cout << entity->name() << "::" << funcName << ": ";
    else if (funcName.size() > 0)
      std::cout << funcName << ": ";
    std::cout << cond << std::endl;
  }

  virtual void entryNew(Fwk::NamedInterface *entity,
                        std::string funcName, std::string cond) {
    entryNew(info_, entity, funcName, cond);
  }

  virtual void entryNew(Level level, std::string funcName, std::string cond) {
    entryNew(level, NULL, funcName, cond);
  }

  virtual void entryNew(std::string funcName, std::string cond) {
    entryNew(info_, NULL, funcName, cond);
  }

  virtual void entryNew(Level level, std::string cond) {
    entryNew(level, NULL, "", cond);
  }

  virtual void entryNew(std::string cond) {
    entryNew(info_, NULL, "", cond);
  }

  virtual void entryNew(Level level, Exception& e) {
    entryNew(level, e.entity(), e.funcName(), e.message());
  }

  virtual void entryNew(Exception& e) {
    entryNew(critical_, e);
  }

protected:
  virtual ~Log() { }
  Log(const std::string& loggerName)
    : loggerName_(loggerName), logLevel_(info_) { }

  std::string timestamp() {
    char buf[50];
    time_t rawtime;
    struct tm timeinfo;

    time(&rawtime);
    localtime_r(&rawtime, &timeinfo);

    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &timeinfo);
    return std::string(buf);
  }

  std::string loggerName_;
  Level logLevel_;

  // Log is a singleton
  static Log::Ptr rootLog;
};


Log::LogStream::Ptr operator<<(Log::LogStream::Ptr ls, const std::string& val);
Log::LogStream::Ptr operator<<(Log::LogStream::Ptr ls, bool val);
Log::LogStream::Ptr operator<<(Log::LogStream::Ptr ls, short val);
Log::LogStream::Ptr operator<<(Log::LogStream::Ptr ls, unsigned short val);
Log::LogStream::Ptr operator<<(Log::LogStream::Ptr ls, int val);
Log::LogStream::Ptr operator<<(Log::LogStream::Ptr ls, unsigned int val);
Log::LogStream::Ptr operator<<(Log::LogStream::Ptr ls, long val);
Log::LogStream::Ptr operator<<(Log::LogStream::Ptr ls, unsigned long val);
Log::LogStream::Ptr operator<<(Log::LogStream::Ptr ls, float val);
Log::LogStream::Ptr operator<<(Log::LogStream::Ptr ls, double val);
Log::LogStream::Ptr operator<<(Log::LogStream::Ptr ls, long double val);
Log::LogStream::Ptr operator<<(Log::LogStream::Ptr ls, const void* val);
Log::LogStream::Ptr operator<<(Log::LogStream::Ptr ls, char c);
Log::LogStream::Ptr operator<<(Log::LogStream::Ptr ls, signed char c);
Log::LogStream::Ptr operator<<(Log::LogStream::Ptr ls, unsigned char c);
Log::LogStream::Ptr operator<<(Log::LogStream::Ptr ls, const char* s);
Log::LogStream::Ptr operator<<(Log::LogStream::Ptr ls, const signed char* s);
Log::LogStream::Ptr operator<<(Log::LogStream::Ptr ls, const unsigned char* s);

}

#endif
