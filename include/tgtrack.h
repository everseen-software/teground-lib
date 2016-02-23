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

#ifndef TGTRACK_H
#define TGTRACK_H

#include "tgglobal.h"

namespace tg{

class TrackHeader;
class Track{

public:
    Track(TrackHeader* header, VideoTime length)
        : m_header(header)
        , m_length(length)
    {}
    virtual ~Track(){}

    TrackHeader* header();
    const TrackHeader* header() const;

    VideoTime length() const;

    virtual void write(cv::FileStorage& fs, size_t headerIndex) const = 0;
    virtual void read(const cv::FileNode& node) = 0;

private:
    Track();
    Track(const Track& other);
    Track& operator = (const Track& other);

    TrackHeader* m_header;
    VideoTime    m_length;
};


inline TrackHeader *Track::header(){
    return m_header;
}

inline const TrackHeader *Track::header() const{
    return m_header;
}

inline VideoTime Track::length() const{
    return m_length;
}


} // namespace

#endif // !TGTRACK_H
