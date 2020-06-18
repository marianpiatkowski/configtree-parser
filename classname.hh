// -*- tab-width: 2; indent-tabs-mode: nil -*-
// vi: set et ts=2 sw=2 sts=2:
#ifndef CLASSNAME_HH
#define CLASSNAME_HH

/** \file
 * \brief A free function to provide the demangled class name
 *        of a given object or type as a string
 */

#include <cstdlib>
#include <string>
#include <typeinfo>

#if HAVE_CXA_DEMANGLE
#include <cxxabi.h>
#endif // #if HAVE_CXA_DEMANGLE

/** \brief Provide the demangled class name of a type T as a string */
template <class T>
std::string className ()
{
  typedef typename std::remove_reference<T>::type TR;
  std::string className = typeid( TR ).name();
#if HAVE_CXA_DEMANGLE
  int status;
  char *demangled = abi::__cxa_demangle( className.c_str(), 0, 0, &status );
  if( demangled )
  {
    className = demangled;
    std::free( demangled );
  }
#endif // #if HAVE_CXA_DEMANGLE
  if (std::is_const<TR>::value)
      className += " const";
  if (std::is_volatile<TR>::value)
      className += " volatile";
  if (std::is_lvalue_reference<T>::value)
      className += "&";
  else if (std::is_rvalue_reference<T>::value)
      className += "&&";
  return className;
}

/** \brief Provide the demangled class name of a given object as a string */
template <class T>
std::string className ( T& )
{
  return className<T>();
}

#endif
