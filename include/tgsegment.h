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

#ifndef TGSEGMENT_H
#define TGSEGMENT_H

#include "tgglobal.h"

namespace tg{

class Segment{

public:
    friend class SegmentTrack;

public:
    Segment()
        : m_position(0)
        , m_length(0)
    {}
    Segment(VideoTime position, VideoTime length = 10, const std::string& data = "")
        : m_position(position)
        , m_length(length)
        , m_data(data)
    {}
    ~Segment()
    {}

    const std::string& data() const;
    VideoTime position() const;
    VideoTime length() const;

    void setData(const std::string& data);

    void write(cv::FileStorage& fs) const;
    void read(const cv::FileNode& node);

private:
    // disable copy
    Segment(const Segment&);
    Segment& operator = (const Segment&);

    VideoTime   m_position;
    VideoTime   m_length;
    std::string m_data;

};

inline const std::string&Segment::data() const{
    return m_data;
}

inline VideoTime Segment::position() const{
    return m_position;
}

inline VideoTime Segment::length() const{
    return m_length;
}

inline void Segment::setData(const std::string& data){
    m_data = data;
}

// Serialization

inline void Segment::write(cv::FileStorage& fs) const{
    fs << "{" <<
          "Pos"    << (double)m_position <<
          "Length" << (double)m_length <<
          "Data"   << m_data
       << "}";
}

inline void Segment::read(const cv::FileNode& node){
    m_position = (VideoTime)(double)node["Pos"];
    m_length   = (VideoTime)(double)node["Length"];
    m_data     = (std::string)node["Data"];
}

static inline void write(cv::FileStorage& fs, const std::string&, const Segment& x){
    x.write(fs);
}

static inline void read(const cv::FileNode& node, Segment& x, const Segment&){
    if(!node.empty())
        x.read(node);
}

} // namespace tg

#endif // TGSEGMENT_H
