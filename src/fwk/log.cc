#include "log.h"
#include <string>


namespace Fwk {

Log::Ptr Log::rootLog;

#define L_LS_OP_LL(TYPE)                                              \
  Log::LogStream::Ptr operator<<(Log::LogStream::Ptr ls, TYPE val) {  \
    ls->stream() << val;                                              \
    return ls;                                                        \
  }

L_LS_OP_LL(const std::string&);
L_LS_OP_LL(bool);
L_LS_OP_LL(short);
L_LS_OP_LL(unsigned short);
L_LS_OP_LL(int);
L_LS_OP_LL(unsigned int);
L_LS_OP_LL(long);
L_LS_OP_LL(unsigned long);
L_LS_OP_LL(float);
L_LS_OP_LL(double);
L_LS_OP_LL(long double);
L_LS_OP_LL(const void*);
L_LS_OP_LL(char);
L_LS_OP_LL(signed char);
L_LS_OP_LL(unsigned char);
L_LS_OP_LL(const char*);
L_LS_OP_LL(const signed char*);
L_LS_OP_LL(const unsigned char*);

#undef L_LS_OP_LL

}  /* Fwk */
