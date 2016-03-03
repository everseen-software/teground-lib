/****************************************************************************
**
** Copyright (C) 2016 Everseen Ltd.
**
** Concept, design and implementation by Dinu SV
** (contact: mail@dinusv.com)
** This file is part of Teground library.
**
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
****************************************************************************/

#ifndef TGGLOBAL_H
#define TGGLOBAL_H

#include <string>
#include <opencv2/core/core.hpp>


#define TEGROUND_LIBRARY_VERSION_MAJOR 0
#define TEGROUND_LIBRARY_VERSION_MINOR 4
#define TEGROUND_LIBRARY_VERSION_PATCH 2

#define TEGROUND_LIBRARY_VERSION_ENCODE(major, minor, patch) ( \
      ((major) * 1000000) \
    + ((minor) *    1000) \
    + ((patch) *       1))

#define TEGROUND_LIBRARY_VERSION TEGROUND_LIBRARY_VERSION_ENCODE( \
    TEGROUND_LIBRARY_VERSION_MAJOR, \
    TEGROUND_LIBRARY_VERSION_MINOR, \
    TEGROUND_LIBRARY_VERSION_PATCH)

#define TEGROUND_LIBRARY_VERSION_STRINGIZE(major, minor, patch) \
    #major"."#minor"."#patch

#define TEGROUND_LIBRARY_VERSION_STRING TEGROUND_LIBRARY_VERSION_STRINGIZE( \
    TEGROUND_LIBRARY_VERSION_MAJOR, \
    TEGROUND_LIBRARY_VERSION_MINOR, \
    TEGROUND_LIBRARY_VERSION_PATCH  \
)


namespace tg{

typedef long long VideoTime;


class Exception : public std::exception{

private:
    std::string m_message;

public:
    Exception(const std::string& message) : m_message(message){}
    virtual ~Exception() throw() {}

    const std::string& message() const throw(){ return m_message; }
};


}// namespace tg

#endif
