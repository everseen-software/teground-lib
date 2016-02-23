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

#ifndef TGSEGMENTTRACK_H
#define TGSEGMENTTRACK_H

#include "tgglobal.h"
#include "tgtrack.h"
#include "tgsegment.h"
#include <iostream>

namespace tg{

class SegmentTrack : public Track{

public:
    typedef std::vector<Segment*>::iterator       SegmentIterator;
    typedef std::vector<Segment*>::const_iterator SegmentConstIterator;

public:
    SegmentTrack(TrackHeader* header, VideoTime length)
        : Track(header, length)
    {}
    ~SegmentTrack();

    void write(cv::FileStorage& fs, size_t headerIndex) const;
    void read(const cv::FileNode& node);

    void clearSegments();
    SegmentIterator insertSegment(Segment* segment);
    void removeSegment(SegmentIterator segmIt);
    Segment* takeSegment(SegmentIterator segmIt);

    size_t totalSegments() const;

    void assignSegmentCoords(Segment* segment, VideoTime position, VideoTime length);
    SegmentIterator assignSegmentCoords(SegmentIterator it, VideoTime position, VideoTime length);

    SegmentIterator segmentFrom(VideoTime position);
    SegmentConstIterator segmentFrom(VideoTime position) const;
    SegmentIterator segmentFrom(VideoTime position, VideoTime length);
    SegmentConstIterator segmentFrom(VideoTime position, VideoTime length) const;

    SegmentIterator begin();
    SegmentConstIterator begin() const;
    SegmentIterator end();
    SegmentConstIterator end() const;
    SegmentIterator findSegment(Segment* segment);
    SegmentConstIterator findSegment(Segment *segment) const;

private:
    size_t segmentIndexFrom(VideoTime position) const;
    size_t segmentIndexFrom(VideoTime position, VideoTime length) const;

    // prevent copy
    SegmentTrack(const SegmentTrack& other);
    SegmentTrack& operator = (const SegmentTrack& other);

    std::vector<Segment*> m_segments;
};

inline SegmentTrack::~SegmentTrack(){
    clearSegments();
}

inline void SegmentTrack::write(cv::FileStorage &fs, size_t headerIndex) const{
    fs << "{";
    fs << "Header" << (double)(headerIndex);
    fs << "Children" << "[";
    for ( SegmentConstIterator it = begin(); it != end(); ++ it )
        fs << *(*it);
    fs << "]";

    fs << "}";
}

inline void SegmentTrack::read(const cv::FileNode &node){
    cv::FileNode seqNode = node["Children"];
    if (seqNode.type() != cv::FileNode::SEQ)
        throw Exception("\'Segment.Track.Children\' is not iterable.");

    clearSegments();

    for (cv::FileNodeIterator it = seqNode.begin(); it != seqNode.end(); ++it){
        Segment* segment = new Segment;
        *it >> *segment;
        insertSegment(segment);
    }
}

inline void SegmentTrack::clearSegments(){
    for ( SegmentIterator it = begin(); it != end(); ++it )
        delete *it;
    m_segments.clear();
}

inline SegmentTrack::SegmentIterator SegmentTrack::insertSegment(Segment *segment){
    if ( segment->position() + segment->length() > length() )
        throw tg::Exception("Cannot add segment longer than track.");

    SegmentIterator it = segmentFrom(segment->position());
    while ( it != end() ){
        Segment* itseg = *it;
        if ( itseg->position() > segment->position() ){
            return m_segments.insert(it, segment);
        } else if ( itseg->position() == segment->position() && itseg->length() >= segment->length() ){
            return m_segments.insert(it, segment);
        }
        ++it;
    }
    return m_segments.insert(m_segments.end(), segment);
}

inline void SegmentTrack::removeSegment(SegmentTrack::SegmentIterator it){
    if ( it != end() ){
        delete *it;
        m_segments.erase(it);
    }
}

inline Segment *SegmentTrack::takeSegment(SegmentTrack::SegmentIterator segmIt){
    if ( segmIt != end() ){
        Segment* segm = *segmIt;
        m_segments.erase(segmIt);
        return segm;
    }
    return 0;
}

inline size_t SegmentTrack::totalSegments() const{
    return m_segments.size();
}

inline void SegmentTrack::assignSegmentCoords(Segment *segment, VideoTime position, VideoTime length){
    assignSegmentCoords(findSegment(segment), position, length);
}

inline SegmentTrack::SegmentIterator SegmentTrack::assignSegmentCoords(
        SegmentTrack::SegmentIterator it,
        VideoTime position,
        VideoTime length)
{
    if ( it == end() )
        return end();
    Segment* segm = *it;
    if ( segm->position() == position && segm->length() == length )
        return it;

    segm->m_position = position;
    segm->m_length   = length;

    // if position or length different, we need to see if the inserted position is the same

    bool reposition = false;
    if ( it != begin() ){
        SegmentTrack::SegmentIterator prevIt = it - 1;
        Segment* prevSegm = *prevIt;
        if ( prevSegm->position() > segm->position() )
            reposition = true;
        else if ( prevSegm->position() == segm->position() && prevSegm->length() > segm->length() )
            reposition = true;
    }
    SegmentTrack::SegmentIterator nextIt = it + 1;
    if ( nextIt != end() ){
        Segment* nextSegm = *nextIt;
        if ( nextSegm->position() < segm->position() )
            reposition = true;
        else if ( nextSegm->position() == segm->position() && nextSegm->length() < segm->length() )
            reposition = true;
    }

    if ( reposition ){
        m_segments.erase(it);
        return insertSegment(segm);
    }

    return it;
}

inline SegmentTrack::SegmentIterator SegmentTrack::segmentFrom(VideoTime position){
    return m_segments.begin() + segmentIndexFrom(position);
}

inline SegmentTrack::SegmentConstIterator SegmentTrack::segmentFrom(VideoTime position) const{
    return m_segments.begin() + segmentIndexFrom(position);
}

inline SegmentTrack::SegmentIterator SegmentTrack::segmentFrom(VideoTime position, VideoTime length){
    return m_segments.begin() + segmentIndexFrom(position, length);
}

inline SegmentTrack::SegmentConstIterator SegmentTrack::segmentFrom(VideoTime position, VideoTime length) const{
    return m_segments.begin() + segmentIndexFrom(position, length);
}

inline SegmentTrack::SegmentIterator SegmentTrack::begin(){
    return m_segments.begin();
}

inline SegmentTrack::SegmentConstIterator SegmentTrack::begin() const{
    return m_segments.begin();
}

inline SegmentTrack::SegmentIterator SegmentTrack::end(){
    return m_segments.end();
}

inline SegmentTrack::SegmentConstIterator SegmentTrack::end() const{
    return m_segments.end();
}

inline SegmentTrack::SegmentIterator SegmentTrack::findSegment(Segment *segment){
    SegmentTrack::SegmentIterator ict = segmentFrom(segment->position(), segment->length());
   while ( ict != end() ){
       if ( *ict == segment )
           return ict;
       if ( (*ict)->position() != segment->position() || (*ict)->length() != segment->length() )
           return end();
       ++ict;
   }
   return ict;
}

inline SegmentTrack::SegmentConstIterator SegmentTrack::findSegment(Segment *segment) const{
    SegmentTrack::SegmentConstIterator ict = segmentFrom(segment->position(), segment->length());
    while ( ict != end() ){
        if ( *ict == segment )
            return ict;
        if ( (*ict)->position() != segment->position() || (*ict)->length() != segment->length() )
            return end();
        ++ict;
    }
    return ict;
}

inline size_t SegmentTrack::segmentIndexFrom(VideoTime position) const{
    if ( m_segments.size() == 0 )
        return 0;
    if ( m_segments.back()->position() < position )
        return m_segments.size();

    int first  = 0, last = (int)(m_segments.size()) - 1, middle = (first + last) / 2;

    while( first <= last ){
        if ( m_segments[middle]->position() < position ){
            first = middle + 1;
        } else {
            last  = middle - 1;
        }

        middle = (first + last) / 2;
    }

    if ( m_segments[middle]->position() < position )
        return middle + 1;
    else
        return middle;
}

inline size_t SegmentTrack::segmentIndexFrom(VideoTime position, VideoTime length) const{
    size_t index = segmentIndexFrom(position);
    while ( index < m_segments.size() ){
        if ( m_segments[index]->position() != position )
            return m_segments.size();
        if ( m_segments[index]->length() == length )
            return index;
        ++index;
    }
    return m_segments.size();
}

} // namespace

#endif // TGSEGMENTTRACK_H
