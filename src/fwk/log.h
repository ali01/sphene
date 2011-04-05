/** \file log.h
 * Logging class.
 * \author Matt Sparks
 */
#ifndef __FWK__LOG_H_
#define __FWK__LOG_H_

#include <string>
#include <sstream>
#include <iostream>
#include <ctime>

#include "exception.h"
#include "ptr.h"
#include "ptr_interface.h"

namespace Fwk {

/* Log is a singleton */
class Log;
static Fwk::Ptr<Log> rootLog = NULL;

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
    LogStream(const LogStream& other) {
      log_ = other.log_;
      level_ = other.level_;
      ss_ << other.ss_;
    }
    ~LogStream() {
      log_->entryNew(level_, ss_.str());
    }

    Log::Ptr log_;
    Level level_;
    std::stringstream ss_;

    friend class Log;
  };

  static Log::Ptr LogNew(const string& loggerName="root") {
    if (rootLog.ptr() == NULL)
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

  string levelName(Level level) { return levelNames[level]; }

  inline Level level() const { return logLevel_; }
  void levelIs(Level level) {
    if (name() == "root")
      logLevel_ = level;
    else
      rootLog->levelIs(level);
  }

  inline string name() const { return loggerName_; }
  void nameIs(const string& name) { loggerName_ = name; }

  LogStream::Ptr operator()(Level level) {
    return LogStream::LogStreamNew(this, level);
  }
  LogStream::Ptr operator()() {
    return LogStream::LogStreamNew(this, info());
  }

  virtual void entryNew(Level level, Fwk::NamedInterface *entity,
                        string funcName, string cond) {
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
                        string funcName, string cond) {
    entryNew(info_, entity, funcName, cond);
  }

  virtual void entryNew(Level level, string funcName, string cond) {
    entryNew(level, NULL, funcName, cond);
  }

  virtual void entryNew(string funcName, string cond) {
    entryNew(info_, NULL, funcName, cond);
  }

  virtual void entryNew(Level level, string cond) {
    entryNew(level, NULL, "", cond);
  }

  virtual void entryNew(string cond) {
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
  Log(const string& loggerName)
    : loggerName_(loggerName), logLevel_(info_) { }

  string timestamp() {
    char buf[50];
    time_t rawtime;
    struct tm timeinfo;

    time(&rawtime);
    localtime_r(&rawtime, &timeinfo);

    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &timeinfo);
    return string(buf);
  }

  string loggerName_;
  Level logLevel_;
};


Log& operator<<(Log& log, const std::string& str);
Log::Ptr operator<<(Log::Ptr log, const std::string& str);
Log::LogStream::Ptr operator<<(Log::LogStream::Ptr stream,
                               const std::string& str);

}

#endif
