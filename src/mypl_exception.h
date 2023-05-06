//----------------------------------------------------------------------
// FILE: mypl_exception.h
// DATE: Spring 2023
// NAME: S. Bowers
// DESC: ...
//----------------------------------------------------------------------

#ifndef MYPL_EXCEPTION
#define MYPL_EXCEPTION


#include <string>

// specialized exception for mypl implementation
class MyPLException : public std::exception
{
public:
  
  // construct a "normal" error exception
  MyPLException(const std::string& msg);

  // helpers to create correct type of error
  static MyPLException LexerError(const std::string& msg);
  static MyPLException ParserError(const std::string& msg);
  static MyPLException StaticError(const std::string& msg);
  static MyPLException VMError(const std::string& msg);
  
  // return a string representation for printing
  const char* what() const noexcept;
  
private:

  // the exeception message
  std::string message;

};



#endif
