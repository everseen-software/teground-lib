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

#ifndef TGSEGMENTTRACKTEST_H
#define TGSEGMENTTRACKTEST_H

#include "tgglobal.h"
#include "tgtracktest.h"

namespace tg{

class SegmentAssertion{

public:
    enum ResultType{
        MATCH,
        UNMARKED,
        MISS
    };

    enum AssertionType{
        SINGLE_STAMP,
        SINGLE_OVERLAP,
        MULTI_STAMP,
        MULTI_OVERLAP,
        UNMARKED_SEGMENT
    };

    SegmentAssertion(
        VideoTime position,
        VideoTime length,
        ResultType resultType,
        AssertionType assertionType,
        const std::string& info = "",
        const std::string& file = "",
        int lineNumber = 0,
        Segment* segment = 0
    ) : m_position(position)
      , m_length(length)
      , m_result(resultType)
      , m_type(assertionType)
      , m_segment(segment)
      , m_info(info)
      , m_file(file)
      , m_lineNumber(lineNumber)
    {}
    ~SegmentAssertion(){}

    VideoTime position() const{ return m_position; }
    VideoTime length() const{ return m_length; }
    ResultType result() const{ return m_result; }
    AssertionType type() const{ return m_type; }

    bool hasInfo() const{ return !m_info.empty(); }
    const std::string& info() const{ return m_info; }

    bool hasFile() const{ return !m_file.empty(); }
    const std::string& file() const{ return m_file; }
    int lineNumber() const{ return m_lineNumber; }

    const Segment* segment() const{ return m_segment; }
    bool hasSegment() const{ return m_segment != 0; }


private:
    VideoTime     m_position;
    VideoTime     m_length;
    ResultType    m_result;
    AssertionType m_type;

    std::string   m_info;
    std::string   m_file;
    int           m_lineNumber;

    Segment*      m_segment;

};

class SegmentAssertionSubscriber{

public:
    SegmentAssertionSubscriber(){}
    virtual ~SegmentAssertionSubscriber(){}

    virtual void onSequenceSet(Sequence* sequence) = 0;
    virtual void onAssertionInsert(SegmentAssertion*) = 0;
};

class SegmentTrackTest : public TrackTest{

public:
    typedef std::vector<std::vector<SegmentAssertion*> >::iterator AssertionVectorIterator;
    typedef std::vector<SegmentAssertion*>::iterator       AssertionIterator;
    typedef std::vector<SegmentAssertion*>::const_iterator AssertionConstIteartor;

public:
    class OverlapParameters{
    public:
        OverlapParameters();
        bool isMatch(
            VideoTime pos,
            VideoTime length,
            VideoTime segmPos,
            VideoTime segmLength,
            VideoTime& overlapLength,
            VideoTime& missedLength,
            VideoTime& unmarkedLength
        ) const;

    public:
        VideoTime minOverlapLength;
        VideoTime maxMissedLength;
        VideoTime maxUnmarkedLength;
        double minOverlapPercentToSegment;
        double minOverlapPercentToAssertion;
        double maxMissedPercent;
        double maxUnmarkedPercent;
    };

public:
    SegmentTrackTest(const DataFile* data, const TrackHeader* track);
    ~SegmentTrackTest();

    void advanceCursorSequence(DataFile::SequenceIterator itvit, const std::string &file = "", int lineNumber = 0);
    void advanceCursorPosition(VideoTime position, const std::string& file = "", int lineNumber = 0);

    void singleStamp(
        VideoTime position,
        const std::string& info = "",
        const std::string& file = "",
        int lineNumber = 0
    );
    void multiStamp(
        VideoTime position,
        const std::string& info = "",
        const std::string& file = "",
        int lineNumber = 0
    );
    void singleOverlap(
        VideoTime position,
        VideoTime length,
        const OverlapParameters& overlapParams,
        const std::string& info = "",
        const std::string& file = "",
        int lineNumber = 0
    );
    void multiOverlap(
        VideoTime position,
        VideoTime length,
        const OverlapParameters& overlapParams,
        const std::string& info = "",
        const std::string& file = "",
        int lineNumber = 0
    );

    void read(const cv::FileNode& node);
    void write(cv::FileStorage& fs) const;
    bool isEnd() const;

    void draw(
        cv::Mat& dst,
        DataFile::SequenceIterator seqIt,
        VideoTime framePosition,
        VideoTime numberOfFrames = 100,
        int pixelsPerFrame = 10,
        int trackHeight = 30
    );

    void addAssertionSubscriber(SegmentAssertionSubscriber* subscriber);
    void notifySubscribers(SegmentAssertion* assertion);

    size_t countAssertions(SegmentAssertion::ResultType resultType);

    void clearAssertions();

private:
    bool isUnmarked(DataFile::SequenceConstIterator seqIt, Segment* segm);
    SegmentAssertion* firstAssertionFor(DataFile::SequenceConstIterator seqIt, Segment* segm);

    void insertAssertion(DataFile::SequenceConstIterator seqIt, SegmentAssertion* assertion);
    void insertAssertion(size_t assertionVectorIndex, AssertionIterator it, SegmentAssertion *assertion);

    void stamp(
        bool isSingle,
        VideoTime position,
        const std::string& info,
        const std::string& file,
        int lineNumber
    );
    void overlap(
        bool isSingle,
        VideoTime position,
        VideoTime length,
        const OverlapParameters& overlapParams,
        const std::string& info,
        const std::string& file,
        int lineNumber
    );

    bool findMatchedSegment(VideoTime pos, SegmentTrack::SegmentConstIterator &segmIt);
    bool findMatchedSegment(
        VideoTime pos,
        VideoTime length,
        SegmentTrack::SegmentConstIterator &segmIt,
        const OverlapParameters &overlapParams,
        VideoTime& overlapDistance,
        VideoTime& missedDistance,
        VideoTime& unmarkedDistance
    );

    // prevent copy

    SegmentTrackTest(const SegmentTrackTest&);
    SegmentTrackTest& operator = (const SegmentTrackTest&);

    // iterators

    VideoTime m_cursorPosition;
    DataFile::SequenceConstIterator    m_cursorSequenceIt;
    SegmentTrack::SegmentConstIterator m_cursorSegmentIt;

    // assertions

    std::vector<std::vector<SegmentAssertion*> > m_assertions;
    AssertionIterator m_assertionCursorIt;

    std::vector<SegmentAssertionSubscriber*> m_subscribers;

};

inline SegmentTrackTest::SegmentTrackTest(const DataFile *data, const TrackHeader *track)
    : TrackTest(data, track)
    , m_cursorPosition(0)
    , m_cursorSequenceIt(data->sequencesBegin())
{
    if ( track->type() != "Segment" )
        throw Exception("Track \'" + track->name() + "\' isn\'t a segment type.");
    if ( m_cursorSequenceIt != data->sequencesEnd() ){
        SegmentTrack* seqTrack = static_cast<SegmentTrack*>((*m_cursorSequenceIt)->track(track));
        m_cursorSegmentIt = seqTrack->begin();
    }

    m_assertions.resize(data->sequenceCount());
    if ( m_assertions.size() > 0 ){
        m_assertionCursorIt  = m_assertions.front().begin();
    }
}

inline SegmentTrackTest::~SegmentTrackTest(){
    clearAssertions();
}

inline void SegmentTrackTest::advanceCursorSequence(DataFile::SequenceIterator it, const std::string& file, int lineNumber){
    if ( it <= m_cursorSequenceIt )
        throw Exception("Given cursor sequence is before the current one.");


    SegmentTrack* track  = static_cast<SegmentTrack*>((*m_cursorSequenceIt)->track(trackHeader()));
    while (m_cursorSequenceIt != it){

        while( m_cursorSegmentIt != track->end() ){
            if ( isUnmarked(m_cursorSequenceIt, *m_cursorSegmentIt) ){
                insertAssertion(m_cursorSequenceIt, new SegmentAssertion(
                    (*m_cursorSegmentIt)->position(),
                    (*m_cursorSegmentIt)->length(),
                    SegmentAssertion::UNMARKED,
                    SegmentAssertion::UNMARKED_SEGMENT,
                    "",
                    file,
                    lineNumber,
                    *m_cursorSegmentIt
                ));
            }
            ++m_cursorSegmentIt;
        }

        ++m_cursorSequenceIt;

        if ( m_cursorSequenceIt != data()->sequencesEnd() ){
            track = static_cast<SegmentTrack*>((*m_cursorSequenceIt)->track(trackHeader()));
            m_cursorSegmentIt = track->begin();
        }
    }

    if ( m_cursorSequenceIt != data()->sequencesEnd() ){
        track = static_cast<SegmentTrack*>((*m_cursorSequenceIt)->track(trackHeader()));
        m_cursorSegmentIt = track->begin();
        m_cursorPosition    = 0;
        m_assertionCursorIt = m_assertions[m_cursorSequenceIt - data()->sequencesBegin()].begin();
    }
}

inline void SegmentTrackTest::advanceCursorPosition(VideoTime position, const std::string& file, int lineNumber){
    if ( m_cursorSequenceIt == data()->sequencesEnd() )
        throw Exception("Cannot advance cursor position. No sequence available.");

    Sequence* seq = *m_cursorSequenceIt;
    if ( seq->length() <= position ){
        std::stringstream ss; ss << "Invalid cursor position given: " << position << ".";
        throw Exception(ss.str());
    }

    if ( m_cursorPosition >= position )
        throw Exception("Cannot advance cursor backwards.");
    m_cursorPosition = position;

    SegmentTrack* track = static_cast<SegmentTrack*>((*m_cursorSequenceIt)->track(trackHeader()));
    while ( m_cursorSegmentIt != track->end() ){
        Segment* segm = *m_cursorSegmentIt;
        if ( segm->position() + segm->length() > m_cursorPosition )
            break;

        if ( isUnmarked(m_cursorSequenceIt, segm) ){
            insertAssertion(m_cursorSequenceIt, new SegmentAssertion(
                (*m_cursorSegmentIt)->position(),
                (*m_cursorSegmentIt)->length(),
                SegmentAssertion::UNMARKED,
                SegmentAssertion::UNMARKED_SEGMENT,
                "",
                file,
                lineNumber,
                *m_cursorSegmentIt
            ));
        }
        ++m_cursorSegmentIt;
    }
}

inline void SegmentTrackTest::singleStamp(
        VideoTime position,
        const std::string &info,
        const std::string &file,
        int lineNumber)
{
    stamp(true, position, info, file, lineNumber);
}

inline void SegmentTrackTest::multiStamp(
        VideoTime position,
        const std::string& info,
        const std::string& file,
        int lineNumber)
{
    stamp(false, position, info, file, lineNumber);
}

inline void SegmentTrackTest::singleOverlap(
        VideoTime position,
        VideoTime length,
        const SegmentTrackTest::OverlapParameters& overlapParams,
        const std::string &info,
        const std::string &file,
        int lineNumber
){
    overlap(true, position, length, overlapParams, info, file, lineNumber);
}

inline void SegmentTrackTest::multiOverlap(
    VideoTime position,
    VideoTime length,
    const SegmentTrackTest::OverlapParameters& overlapParams,
    const std::string &info,
    const std::string &file,
    int lineNumber
){
    overlap(false, position, length, overlapParams, info, file, lineNumber);
}

inline void SegmentTrackTest::read(const cv::FileNode& node){
    cv::FileNode seqNode = node["Sequences"];
    if ( seqNode.type() != cv::FileNode::SEQ )
        throw Exception("\'SegmentTrackTest.Sequences\' is not iterable.");
    if ( seqNode.size() != data()->sequenceCount() )
        throw Exception("Different number of sequences between data file and result file.");

    clearAssertions();

    m_assertions.resize(seqNode.size());

    for( cv::FileNodeIterator vit = node.begin(); vit != node.end(); ++vit ){
        const cv::FileNode& nodeV = *vit;
        std::vector<SegmentAssertion*>& assertV = m_assertions[(size_t)((double)nodeV["Index"])];

        cv::FileNode assertNode = nodeV["Assertions"];
        if ( assertNode.type() != cv::FileNode::SEQ )
            throw Exception("\'SegmentTrackTest.Sequences.Assertions\' is not iterable.");

        for ( cv::FileNodeIterator it = nodeV.begin(); it != nodeV.end(); ++it ){
            const cv::FileNode& nodeA = *it;

            std::string typeStr = (std::string)nodeA["Type"];
            SegmentAssertion::AssertionType type;
            if ( typeStr == "SingleStamp" ){
                type = SegmentAssertion::SINGLE_STAMP;
            } else if ( typeStr == "MultiStamp" ){
                type = SegmentAssertion::MULTI_STAMP;
            } else if ( typeStr == "SingleOverlap" ){
                type = SegmentAssertion::SINGLE_OVERLAP;
            } else if ( typeStr == "MultiOverlap" ){
                type = SegmentAssertion::MULTI_OVERLAP;
            } else if ( typeStr == "Unmarked" ){
                type = SegmentAssertion::UNMARKED_SEGMENT;
            }

            std::string resultStr = (std::string)nodeA["Result"];
            SegmentAssertion::ResultType result;
            if ( resultStr == "Match" ){
                result = SegmentAssertion::MATCH;
            } else if ( resultStr == "Miss" ){
                result = SegmentAssertion::MISS;
            } else if ( resultStr == "Unmarked" ){
                result = SegmentAssertion::UNMARKED;
            }

            std::string info = "";
            if ( node["Info"].type() != cv::FileNode::NONE )
                info = (std::string)node["Info"];

            std::string file   = "";
            int fileLine = 0;
            if ( node["File"].type() != cv::FileNode::NONE ){
                file     = (std::string)node["File"];
                fileLine = (int)node["FileLine"];
            }

            Segment* segm = 0;
            if ( node["SegmentPosition"].type() != cv::FileNode::NONE &&
                 node["SegmentLength"].type() != cv::FileNode::NONE
            ){
                VideoTime segmentPosition = static_cast<VideoTime>((double)node["SegmentPosition"]);
                VideoTime segmentLength   = static_cast<VideoTime>((double)node["SegmentLength"]);

                const Sequence* seq       = data()->sequenceAt(vit - node.begin());
                const SegmentTrack* track = static_cast<const SegmentTrack*>(seq->track(trackHeader()));

                SegmentTrack::SegmentConstIterator segmIt = track->segmentFrom(segmentPosition, segmentLength);
                if ( segmIt == track->end() ){
                    std::stringstream errorMessage;
                    errorMessage
                            << "\'SegmentTrackTest.Sequences.Assertions\' failed to find segment in data file: "
                            << segmentPosition << ", " << segmentLength;
                    throw Exception(errorMessage.str());
                }

                segm = *segmIt;
            }

            assertV.push_back(new SegmentAssertion(
                static_cast<VideoTime>((double)node["Position"]),
                static_cast<VideoTime>((double)node["Length"]),
                result,
                type,
                info,
                file,
                fileLine,
                segm
            ));
        }
    }

    m_cursorSequenceIt = data()->sequencesEnd();
}

inline void SegmentTrackTest::write(cv::FileStorage& fs) const{
    fs << "{";
    fs << "Header" << (double)(data()->trackIndex(trackHeader()));
    fs << "Type"   << "SegmentTrackTest";
    fs << "Sequences" << "[";
    size_t index = 0;
    for(
        std::vector<std::vector<SegmentAssertion*> >::const_iterator vit = m_assertions.begin();
        vit != m_assertions.end();
        ++vit
    ){
        fs << "{";
        fs << "Index" << (double)index;
        fs << "Assertions" << "[";
        for ( std::vector<SegmentAssertion*>::const_iterator it = vit->begin(); it != vit->end(); ++it ){
            SegmentAssertion* assertion = *it;

            switch( assertion->type() ){
            case SegmentAssertion::SINGLE_STAMP:     fs << "Type" << "SingleStamp"; break;
            case SegmentAssertion::MULTI_STAMP:      fs << "Type" << "MultiStamp"; break;
            case SegmentAssertion::SINGLE_OVERLAP:   fs << "Type" << "SingleOverlap"; break;
            case SegmentAssertion::MULTI_OVERLAP:    fs << "Type" << "MultiOverlap"; break;
            case SegmentAssertion::UNMARKED_SEGMENT: fs << "Type" << "Unmarked"; break;
            }

            switch( assertion->result() ){
            case SegmentAssertion::MATCH: fs << "Result" << "Match"; break;
            case SegmentAssertion::MISS:  fs << "Result" << "Miss"; break;
            case SegmentAssertion::UNMARKED: fs << "Result" << "Unmarked"; break;
            }

            fs << "Position" << (double)assertion->position();
            fs << "Length" << (double)assertion->length();
            if ( assertion->hasInfo() )
                fs << "Info" << assertion->info();
            if ( assertion->hasFile() ){
                fs << "File" << assertion->file();
                fs << "FileLine" << assertion->lineNumber();
            }
            if ( assertion->hasSegment() ){
                fs << "SegmentPosition" << (double)assertion->segment()->position();
                fs << "SegmentLength" << (double)assertion->segment()->length();
            }
        }
        fs << "]";
        fs << "}";
        ++index;
    }
    fs << "]";
}

inline bool SegmentTrackTest::isEnd() const{
    if ( m_cursorSequenceIt == data()->sequencesEnd() )
        return true;
    return false;
}

inline void SegmentTrackTest::draw(
        cv::Mat &dst,
        DataFile::SequenceIterator seqIt,
        VideoTime framePosition,
        VideoTime numberOfFrames,
        int pixelsPerFrame,
        int trackHeight)
{
    // Value calculation

    VideoTime frameEndInterval = framePosition + numberOfFrames;

    dst.create(
       cv::Size(
           TrackTest::DRAW_HEADER_WIDTH + pixelsPerFrame * (int)numberOfFrames,
           trackHeight
       ), CV_8UC3
    );
    dst.setTo(cv::Scalar(70, 70, 70));

    const TrackHeader* theader = trackHeader();

    cv::rectangle(
        dst,
        cv::Rect(0, 0, TrackTest::DRAW_HEADER_WIDTH, trackHeight - 1),
        cv::Scalar(60, 60, 60),
        -1
    );

    cv::putText(
        dst,
        theader->name().substr(0, 9),
        cv::Point(10, trackHeight / 2 + 10),
        cv::FONT_HERSHEY_SIMPLEX,
        0.44,
        cv::Scalar(200, 200, 200),
        1
    );

    size_t currentSequenceIndex = seqIt - data()->sequencesBegin();
    size_t cursorSequenceIndex  = m_cursorSequenceIt - data()->sequencesBegin();

    VideoTime cursorPosition = 0;
    if ( currentSequenceIndex < cursorSequenceIndex )
        cursorPosition = (*seqIt)->length();
    else if ( currentSequenceIndex == cursorSequenceIndex )
        cursorPosition = m_cursorPosition;

    Sequence* seq       = *seqIt;
    SegmentTrack* track = static_cast<SegmentTrack*>(seq->track(theader));

    // Draw unmarked segments

    SegmentTrack::SegmentIterator it = track->segmentFrom(cursorPosition);
    SegmentTrack::SegmentIterator reversePeekIt = it;
    if ( reversePeekIt != track->begin() ){
        do{
            --reversePeekIt;
            Segment* segm = *reversePeekIt;
            if ( segm->position() + segm->length() <= cursorPosition )
                break;
            it = reversePeekIt;
        } while ( reversePeekIt != track->begin() );
    }

    VideoTime sequencePosition = 0;
    while( seqIt != data()->sequencesEnd() ){
        if ( it == track->end() ){
            sequencePosition += seq->length();
            ++seqIt;
            if ( seqIt == data()->sequencesEnd() )
                break;
            seq = *seqIt;
            track = static_cast<SegmentTrack*>(seq->track(theader));
            it = track->begin();
        } else {
            Segment* segm = *it;
            if ( segm->position() > frameEndInterval )
                break;

            if ( isUnmarked(seqIt, segm) ){

                int drawStartPosition = (int)(segm->position() - framePosition + sequencePosition);
                int drawLength        = (int)(segm->length());
                if ( drawStartPosition < 0 ){
                    drawLength       += drawStartPosition;
                    drawStartPosition = 0;
                }

                cv::rectangle(
                    dst,
                    cv::Rect(
                        TrackTest::DRAW_HEADER_WIDTH + drawStartPosition * pixelsPerFrame, 0,
                        drawLength * pixelsPerFrame, trackHeight
                    ),
                    cv::Scalar(84, 84, 84),
                    -1
                );
            }

            ++it;
        }
    }

    // Draw assertions and their marked segments


    AssertionIterator asIt = m_assertions[currentSequenceIndex].begin();
    sequencePosition = 0;
    while ( currentSequenceIndex < m_assertions.size() ){
        if ( asIt == m_assertions[currentSequenceIndex].end() ){
            sequencePosition += seq->length();
            ++currentSequenceIndex;
            if ( currentSequenceIndex == m_assertions.size() )
                break;
            asIt = m_assertions[currentSequenceIndex].begin();
        } else {
            if ((*asIt)->hasSegment() ){
                const Segment* segm = (*asIt)->segment();

                if ( sequencePosition + segm->position() + segm->length() > framePosition ){
                    int drawStartPosition = (int)(segm->position() - framePosition + sequencePosition);
                    int drawLength        = (int)(segm->length());
                    if ( drawStartPosition < 0 ){
                        drawLength       += drawStartPosition;
                        drawStartPosition = 0;
                    }

                    cv::Scalar drawColor = (*asIt)->result() == SegmentAssertion::MATCH ?
                                cv::Scalar(84, 200, 84) : cv::Scalar(84, 84, 200);

                    cv::rectangle(
                        dst,
                        cv::Rect(
                            TrackTest::DRAW_HEADER_WIDTH + drawStartPosition * pixelsPerFrame, 0,
                            drawLength * pixelsPerFrame, trackHeight
                        ),
                        drawColor,
                        -1
                    );
                }
            }

            if ( sequencePosition + (*asIt)->position() + (*asIt)->length() > framePosition ){
                int drawStartPosition = (int)((*asIt)->position() - framePosition + sequencePosition);
                int drawLength        = (int)((*asIt)->length());
                if ( drawStartPosition < 0 ){
                    drawLength       += drawStartPosition;
                    drawStartPosition = 0;
                }

                cv::Scalar drawColor = (*asIt)->result() == SegmentAssertion::MATCH ?
                            cv::Scalar(30, 120, 30) : cv::Scalar(30, 30, 120);

                cv::rectangle(
                    dst,
                    cv::Rect(
                        TrackTest::DRAW_HEADER_WIDTH + drawStartPosition * pixelsPerFrame, 0,
                        drawLength * pixelsPerFrame, trackHeight
                    ),
                    drawColor,
                    -1
                );
            }

            if ( (*asIt)->position() >= frameEndInterval )
                break;

            ++asIt;
        }
    }

    // Draw Cursor Position

    if ( currentSequenceIndex == cursorSequenceIndex ){
        if ( cursorPosition >= framePosition && cursorPosition < frameEndInterval ){
            int xPositionDraw = TrackTest::DRAW_HEADER_WIDTH + ((int)(cursorPosition - framePosition) * pixelsPerFrame);
            cv::line(dst,
                     cv::Point(xPositionDraw, 0),
                     cv::Point(xPositionDraw, trackHeight),
                     cv::Scalar(220, 220, 220)
            );
        }
    }
}

inline void SegmentTrackTest::addAssertionSubscriber(SegmentAssertionSubscriber* subscriber){
    m_subscribers.push_back(subscriber);
}

inline void SegmentTrackTest::notifySubscribers(SegmentAssertion* assertion){
    for(
         std::vector<SegmentAssertionSubscriber*>::iterator it = m_subscribers.begin();
         it != m_subscribers.end();
         ++it
    ){
        (*it)->onAssertionInsert(assertion);
    }
}

inline size_t SegmentTrackTest::countAssertions(SegmentAssertion::ResultType resultType){
    size_t totalAssertions = 0;
    for ( SegmentTrackTest::AssertionVectorIterator avIt = m_assertions.begin(); avIt != m_assertions.end(); ++avIt ){
        for ( SegmentTrackTest::AssertionIterator asIt = avIt->begin(); asIt != avIt->end(); ++asIt ){
            if ( (*asIt)->result() == resultType )
                ++totalAssertions;
        }
    }
    return totalAssertions;
}

inline void SegmentTrackTest::clearAssertions(){
    for (
        std::vector<std::vector<SegmentAssertion*> >::iterator vit = m_assertions.begin();
        vit != m_assertions.end();
        ++vit
    ){
        for ( AssertionIterator ait = (*vit).begin(); ait != (*vit).end(); ++ait )
            delete *ait;
    }
    m_assertions.clear();
}

inline bool SegmentTrackTest::isUnmarked(DataFile::SequenceConstIterator seqIt, Segment* segm){
    size_t assertionIndex = seqIt - data()->sequencesBegin();

    AssertionIterator asIt =
        (seqIt == m_cursorSequenceIt ? m_assertionCursorIt : m_assertions[assertionIndex].begin());

    while ( asIt != m_assertions[assertionIndex].end() ){
        if ( (*asIt)->segment() == segm )
            return false;
        ++asIt;
    }
    return true;
}

inline SegmentAssertion* SegmentTrackTest::firstAssertionFor(DataFile::SequenceConstIterator seqIt, Segment* segm){
    size_t assertionIndex = seqIt - data()->sequencesBegin();

    AssertionIterator asIt =
        (seqIt == m_cursorSequenceIt ? m_assertionCursorIt : m_assertions[assertionIndex].begin());

    while ( asIt != m_assertions[assertionIndex].end() ){
        if ( (*asIt)->segment() == segm )
            return *asIt;
        ++asIt;
    }
    return 0;
}

inline void SegmentTrackTest::insertAssertion(DataFile::SequenceConstIterator seqIt, SegmentAssertion* assertion){
    size_t assertionIndex = seqIt - data()->sequencesBegin();
    AssertionIterator asIt =
        (seqIt == m_cursorSequenceIt ? m_assertionCursorIt : m_assertions[assertionIndex].begin());

    while ( asIt != m_assertions[assertionIndex].end() ){
        if ( (*asIt)->position() > assertion->position() ){
            insertAssertion(assertionIndex, asIt, assertion);
            return;
        } else if ( (*asIt)->position() == assertion->position() && (*asIt)->length() >= assertion->length() ){
            insertAssertion(assertionIndex, asIt, assertion);
            return;
        }
        ++asIt;
    }

    insertAssertion(assertionIndex, m_assertions[assertionIndex].end(), assertion);
}

inline void SegmentTrackTest::insertAssertion(
        size_t assertionVectorIndex,
        SegmentTrackTest::AssertionIterator it,
        SegmentAssertion* assertion
){
    if ( assertion->result() == SegmentAssertion::UNMARKED ){
        m_assertionCursorIt  = m_assertions[assertionVectorIndex].insert(it, assertion);
        ++m_assertionCursorIt;
    } else {
        size_t assertionCursorIndex = m_assertionCursorIt - m_assertions[assertionVectorIndex].begin();
        m_assertions[assertionVectorIndex].insert(it, assertion);
        m_assertionCursorIt = m_assertions[assertionVectorIndex].begin() + assertionCursorIndex;
    }
    notifySubscribers(assertion);
}

inline void SegmentTrackTest::stamp(
    bool isSingle,
    VideoTime position,
    const std::string &info,
    const std::string &file,
    int lineNumber
){
    SegmentTrack::SegmentConstIterator segmIt = m_cursorSegmentIt;

    if ( m_cursorSequenceIt == data()->sequencesEnd() )
        throw Exception("Current sequence is not set.");
    if ( position >= (*m_cursorSequenceIt)->length() )
        throw Exception("Position is not within the current sequence range.");

    while( findMatchedSegment(position, segmIt) ){
        bool insert = true;
        if ( !isSingle ){
            SegmentAssertion* firstAssertion = firstAssertionFor(m_cursorSequenceIt, *segmIt);
            if( firstAssertion != 0 )
                if ( firstAssertion->type() == SegmentAssertion::SINGLE_STAMP )
                    insert = false;
        } else if ( !isUnmarked(m_cursorSequenceIt, *segmIt ) ){
            insert = false;
        }

        if ( insert ){
            insertAssertion(m_cursorSequenceIt, new SegmentAssertion(
                position,
                1,
                SegmentAssertion::MATCH,
                isSingle ? SegmentAssertion::SINGLE_STAMP : SegmentAssertion::MULTI_STAMP,
                info,
                file,
                lineNumber,
                *segmIt
            ));
            return;
        }
        ++segmIt;
    }

    insertAssertion(m_cursorSequenceIt, new SegmentAssertion(
        position,
        1,
        SegmentAssertion::MISS,
        isSingle ? SegmentAssertion::SINGLE_STAMP : SegmentAssertion::MULTI_STAMP,
        info,
        file,
        lineNumber,
        0
    ));
}

inline void SegmentTrackTest::overlap(
    bool isSingle,
    VideoTime position,
    VideoTime length,
    const SegmentTrackTest::OverlapParameters& overlapParams,
    const std::string &info,
    const std::string &file,
    int lineNumber
){
    SegmentTrack::SegmentConstIterator segmIt = m_cursorSegmentIt;

    if ( m_cursorSequenceIt == data()->sequencesEnd() )
        throw Exception("Current sequence is not set.");
    if ( position >= (*m_cursorSequenceIt)->length() )
        throw Exception("Position is not within the current sequence range.");

    VideoTime overlapLength  = 0;
    VideoTime missedLength   = 0;
    VideoTime unmarkedLength = 0;

    while( findMatchedSegment(position, length, segmIt, overlapParams, overlapLength, missedLength, unmarkedLength) ){
        bool insert = true;
        if ( !isSingle ){
            SegmentAssertion* firstAssertion = firstAssertionFor(m_cursorSequenceIt, *segmIt);
            if( firstAssertion != 0 )
                if ( firstAssertion->type() == SegmentAssertion::SINGLE_STAMP )
                    insert = false;
        } else if ( !isUnmarked(m_cursorSequenceIt, *segmIt ) ){
            insert = false;
        }

        if ( insert ){
            insertAssertion(m_cursorSequenceIt, new SegmentAssertion(
                position,
                length,
                SegmentAssertion::MATCH,
                isSingle ? SegmentAssertion::SINGLE_STAMP : SegmentAssertion::MULTI_STAMP,
                info,
                file,
                lineNumber,
                *segmIt
            ));
            return;
        }
        ++segmIt;
    }

    insertAssertion(m_cursorSequenceIt, new SegmentAssertion(
        position,
        length,
        SegmentAssertion::MISS,
        isSingle ? SegmentAssertion::SINGLE_STAMP : SegmentAssertion::MULTI_STAMP,
        info,
        file,
        lineNumber,
        0
    ));
}

inline bool SegmentTrackTest::findMatchedSegment(
    VideoTime pos,
    SegmentTrack::SegmentConstIterator& segmIt
){
    SegmentTrack* tr = static_cast<SegmentTrack*>((*m_cursorSequenceIt)->track(trackHeader()));
    while ( segmIt != tr->end() ){
        Segment* segm = *segmIt;
        if ( segm->position() > pos ){
            return false;
        }
        if ( segm->position() <= pos && segm->position() + segm->length() > pos )
            return true;
        ++segmIt;
    }
    return false;
}

inline bool SegmentTrackTest::findMatchedSegment(
    VideoTime pos,
    VideoTime length,
    SegmentTrack::SegmentConstIterator& segmIt,
    const SegmentTrackTest::OverlapParameters& overlapParams,
    VideoTime &overlapLength,
    VideoTime &missedLength,
    VideoTime &unmarkedLength
){
    SegmentTrack* tr = static_cast<SegmentTrack*>((*m_cursorSequenceIt)->track(trackHeader()));
    while ( segmIt != tr->end() ){
        Segment* segm = *segmIt;
        if ( segm->position() >= pos + length )
            return false;

        if ( overlapParams.isMatch(
            pos,
            length,
            segm->position(),
            segm->length(),
            overlapLength,
            missedLength,
            unmarkedLength
        )){
            return true;
        }

        ++segmIt;
    }
    return false;
}

// SegmentTrackTest::OverlapParameters Implementation
// --------------------------------------------------

inline SegmentTrackTest::OverlapParameters::OverlapParameters()
    : minOverlapLength(0)
    , maxMissedLength(0)
    , maxUnmarkedLength(0)
    , minOverlapPercentToAssertion(0)
    , minOverlapPercentToSegment(0)
    , maxMissedPercent(0)
    , maxUnmarkedPercent(0)
{
}

inline bool SegmentTrackTest::OverlapParameters::isMatch(
    VideoTime pos,
    VideoTime length,
    VideoTime segmPos,
    VideoTime segmLength,
    VideoTime& overlapLength,
    VideoTime& missedLength,
    VideoTime& unmarkedLength
) const{
    overlapLength  = 0;
    missedLength   = 0;
    unmarkedLength = 0;

    if ( pos + length > segmPos && segmPos + segmLength > pos ){
        VideoTime overlapStart = 0;
        VideoTime overlapEnd   = 0;
        if ( segmPos > pos ){
            overlapStart = segmPos;
            missedLength = segmPos - pos;
        } else {
            overlapStart   = pos;
            unmarkedLength = pos - segmPos;
        }

        if ( segmPos + segmLength > pos + length ){
            overlapEnd      = pos + length;
            unmarkedLength += (segmPos + segmLength) - (pos + length);
        } else {
            overlapEnd    = segmPos + segmLength;
            missedLength += (pos + length) - (segmPos + segmLength);
        }
        overlapLength = overlapEnd - overlapStart;

        if ( overlapLength < minOverlapLength )
            return false;
        if ( minOverlapPercentToAssertion > 0 && (double)overlapLength / length < minOverlapPercentToAssertion)
            return false;
        if ( minOverlapPercentToSegment > 0 && (double)overlapLength / segmLength < minOverlapPercentToSegment)
            return false;

        if ( maxMissedLength > 0 && missedLength > maxMissedLength )
            return false;
        if ( maxMissedPercent > 0 && (double)missedLength / length > maxMissedPercent )
            return false;

        if ( maxUnmarkedLength > 0 && unmarkedLength > maxUnmarkedLength )
            return false;
        if ( maxUnmarkedPercent > 0 && (double)unmarkedLength / segmLength > maxUnmarkedPercent)
            return false;

        return true;
    }
    return false;
}

}// namespace


#ifndef TG_DISABLE_MACROS

#define TG_SEGMENT_TRACK_TEST(_var, _dataFile, _track) \
    SegmentTrackTest* _var = new SegmentTrackTest((_dataFile), (_track))

#define TG_TEST_SUITE_ADD_SEGMENT_TEST(_testSuite, _var, _dataFile, _track) \
    SegmentTrackTest* _var = new SegmentTrackTest((_dataFile), (_track)); \
    _testSuite.addTest(_var)

#define TG_SEGMENT_SINGLE_STAMP(_var, _position, _info) \
    _var->singleStamp(_position, _info, __FILE__, __LINE__)

#define TG_SEGMENT_MULTI_STAMP(_var, _position, _info) \
    _var->multiStamp(_position, _info, __FILE__, __LINE__)

#define TG_SEGMENT_SINGLE_OVERLAP(_var, _position, _length, _overlapParams, _info) \
    _var->singleOverlap(_position, _length, _overlapParams, _info, __FILE__, __LINE__)

#define TG_SEGMENT_MULTI_OVERLAP(_var, _position, _length, _overlapParams, _info) \
    _var->multiOverlap(_position, _length, _overlapParams, _info, __FILE__, __LINE__)

#define TG_SEGMENT_ADVANCE_CURSOR_POSITION(_var, _position) \
    _var->advanceCursorPosition(_position, __FILE__, __LINE__)

#define TG_SEGMENT_ADVANCE_CURSOR_SEQUENCE(_var, _sequenceIt) \
    _var->advanceCursorSequence(_sequenceIt, __FILE__, __LINE__)

#endif

#endif // TGSEGMENTTRACKTEST_H
