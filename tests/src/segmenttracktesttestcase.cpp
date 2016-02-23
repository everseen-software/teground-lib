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

#include "catch.hpp"

#include "tgdatafile.h"
#include "tgsegment.h"
#include "tgsegmenttrack.h"
#include "tgsegmenttracktest.h"

using namespace tg;

namespace tgsegmenttracktest_test{

class AssertionSubscriberMock : public SegmentAssertionSubscriber{

public:
    AssertionSubscriberMock(){}
    const SegmentAssertion* lastAssertion(){ return (m_assertions.size() > 0 ? m_assertions.back() : 0); }
    void removeAssertions(){ m_assertions.clear(); }

    size_t totalAssertions(){ return m_assertions.size(); }
    SegmentAssertion* assertionAt(size_t index){ return m_assertions.at(index); }

private:
    std::vector<SegmentAssertion*> m_assertions;

    void onSequenceSet(Sequence*){}
    void onAssertionInsert(SegmentAssertion* assertion){ m_assertions.push_back(assertion); }

};

TEST_CASE("Teground SegmentTrackTest Test", "[segmenttracktesttestcase]"){

    SECTION("Null sequences"){
        DataFile dfile;
        TrackHeader* theader = dfile.appendTrack("Segment", "Track");

        SegmentTrackTest testsuite(&dfile, theader);

        REQUIRE_THROWS_AS(testsuite.advanceCursorPosition(100), tg::Exception);
        REQUIRE_THROWS_AS(testsuite.advanceCursorSequence(dfile.sequencesBegin()), tg::Exception);
        REQUIRE_THROWS_AS(testsuite.singleStamp(100), tg::Exception);
    }

    SECTION("Single sequence - No Segment"){
        DataFile dfile;
        TrackHeader* theader = dfile.appendTrack("Segment", "Track");
        dfile.appendSequence(new Sequence("test1", "StandardVideoDecoder", Sequence::Video, 100));

        AssertionSubscriberMock assertionSubscriber;
        SegmentTrackTest testsuite(&dfile, theader);
        testsuite.addAssertionSubscriber(&assertionSubscriber);

        REQUIRE_THROWS_AS(testsuite.advanceCursorPosition(100), tg::Exception);
        REQUIRE_THROWS_AS(testsuite.advanceCursorSequence(dfile.sequencesBegin()), tg::Exception);
        REQUIRE_THROWS_AS(testsuite.singleStamp(100), tg::Exception);

        testsuite.singleStamp(50);
        REQUIRE(assertionSubscriber.lastAssertion() != 0);
        REQUIRE(assertionSubscriber.lastAssertion()->result() == SegmentAssertion::MISS);

        testsuite.advanceCursorPosition(60);
        REQUIRE_THROWS_AS(testsuite.advanceCursorPosition(50), tg::Exception);
    }

    SECTION("Single sequence - One segment - No Match"){
        DataFile dfile;
        TrackHeader* theader = dfile.appendTrack("Segment", "Track");
        Sequence* seq = new Sequence("test1", "StandardVideoDecoder", Sequence::Video, 100);
        dfile.appendSequence(seq);

        SegmentTrack* track = static_cast<SegmentTrack*>(seq->track("Track"));
        track->insertSegment(new Segment(50, 10));

        AssertionSubscriberMock assertionSubscriber;
        SegmentTrackTest testsuite(&dfile, theader);
        testsuite.addAssertionSubscriber(&assertionSubscriber);

        testsuite.singleStamp(60);
        REQUIRE(assertionSubscriber.lastAssertion() != 0);
        REQUIRE(assertionSubscriber.lastAssertion()->result() == SegmentAssertion::MISS);
        assertionSubscriber.removeAssertions();

        testsuite.advanceCursorPosition(60);
        REQUIRE(assertionSubscriber.lastAssertion() != 0);
        REQUIRE(assertionSubscriber.lastAssertion()->result() == SegmentAssertion::UNMARKED);
        REQUIRE(assertionSubscriber.lastAssertion()->segment() == *(track->begin()));
    }

    SECTION("Single sequence - One segment - Match(Stamp)"){
        DataFile dfile;
        TrackHeader* theader = dfile.appendTrack("Segment", "Track");
        Sequence* seq = new Sequence("test1", "StandardVideoDecoder", Sequence::Video, 100);
        dfile.appendSequence(seq);

        SegmentTrack* track = static_cast<SegmentTrack*>(seq->track("Track"));
        track->insertSegment(new Segment(50, 10));

        AssertionSubscriberMock assertionSubscriber;
        SegmentTrackTest testsuite(&dfile, theader);
        testsuite.addAssertionSubscriber(&assertionSubscriber);

        testsuite.singleStamp(50);
        REQUIRE(assertionSubscriber.lastAssertion() != 0);
        REQUIRE(assertionSubscriber.lastAssertion()->result() == SegmentAssertion::MATCH);
        assertionSubscriber.removeAssertions();

        testsuite.advanceCursorPosition(60);
        REQUIRE(assertionSubscriber.lastAssertion() == 0);
    }

    SECTION("Single Sequence - Multi Segment - No Match"){
        DataFile dfile;
        TrackHeader* theader = dfile.appendTrack("Segment", "Track");
        Sequence* seq = new Sequence("test1", "StandardVideoDecoder", Sequence::Video, 100);
        dfile.appendSequence(seq);

        SegmentTrack* track = static_cast<SegmentTrack*>(seq->track("Track"));
        track->insertSegment(new Segment(20, 10));
        track->insertSegment(new Segment(25, 10));
        track->insertSegment(new Segment(50, 10));
        track->insertSegment(new Segment(55, 10));
        track->insertSegment(new Segment(75, 10));

        AssertionSubscriberMock assertionSubscriber;
        SegmentTrackTest testsuite(&dfile, theader);
        testsuite.addAssertionSubscriber(&assertionSubscriber);

        testsuite.advanceCursorPosition(40);
        REQUIRE(assertionSubscriber.totalAssertions() == 2);
        REQUIRE(assertionSubscriber.assertionAt(0)->result() == SegmentAssertion::UNMARKED);
        REQUIRE(assertionSubscriber.assertionAt(1)->result() == SegmentAssertion::UNMARKED);
        REQUIRE(assertionSubscriber.assertionAt(0)->segment() != 0);
        REQUIRE(assertionSubscriber.assertionAt(1)->segment() != 0);
        REQUIRE(assertionSubscriber.assertionAt(0)->segment()->position() == 20);
        REQUIRE(assertionSubscriber.assertionAt(1)->segment()->position() == 25);

        assertionSubscriber.removeAssertions();
        testsuite.advanceCursorPosition(90);
        REQUIRE(assertionSubscriber.totalAssertions() == 3);
        REQUIRE(assertionSubscriber.assertionAt(0)->result() == SegmentAssertion::UNMARKED);
        REQUIRE(assertionSubscriber.assertionAt(1)->result() == SegmentAssertion::UNMARKED);
        REQUIRE(assertionSubscriber.assertionAt(2)->result() == SegmentAssertion::UNMARKED);
    }

    SECTION("Single Sequence - Multi Segment - No Match - Segment Overlap"){
        DataFile dfile;
        TrackHeader* theader = dfile.appendTrack("Segment", "Track");
        Sequence* seq = new Sequence("test1", "StandardVideoDecoder", Sequence::Video, 100);
        dfile.appendSequence(seq);

        SegmentTrack* track = static_cast<SegmentTrack*>(seq->track("Track"));
        track->insertSegment(new Segment(20, 50));
        track->insertSegment(new Segment(25, 30));
        track->insertSegment(new Segment(35, 50));
        track->insertSegment(new Segment(50, 10));
        track->insertSegment(new Segment(65, 20));
        track->insertSegment(new Segment(75, 10));

        AssertionSubscriberMock assertionSubscriber;
        SegmentTrackTest testsuite(&dfile, theader);
        testsuite.addAssertionSubscriber(&assertionSubscriber);

        testsuite.advanceCursorPosition(40);
        REQUIRE(assertionSubscriber.totalAssertions() == 0);

        assertionSubscriber.removeAssertions();
        testsuite.advanceCursorPosition(70);
        REQUIRE(assertionSubscriber.totalAssertions() == 2);
        REQUIRE(assertionSubscriber.assertionAt(0)->result() == SegmentAssertion::UNMARKED);
        REQUIRE(assertionSubscriber.assertionAt(1)->result() == SegmentAssertion::UNMARKED);

        assertionSubscriber.removeAssertions();
        testsuite.advanceCursorPosition(90);
        REQUIRE(assertionSubscriber.totalAssertions() == 4);
        REQUIRE(assertionSubscriber.assertionAt(0)->result() == SegmentAssertion::UNMARKED);
        REQUIRE(assertionSubscriber.assertionAt(1)->result() == SegmentAssertion::UNMARKED);
        REQUIRE(assertionSubscriber.assertionAt(2)->result() == SegmentAssertion::UNMARKED);
        REQUIRE(assertionSubscriber.assertionAt(3)->result() == SegmentAssertion::UNMARKED);
    }

    SECTION("Single Sequence - Multi Segment - Match(Stamp)"){
        DataFile dfile;
        TrackHeader* theader = dfile.appendTrack("Segment", "Track");
        Sequence* seq = new Sequence("test1", "StandardVideoDecoder", Sequence::Video, 100);
        dfile.appendSequence(seq);

        SegmentTrack* track = static_cast<SegmentTrack*>(seq->track("Track"));
        track->insertSegment(new Segment(20, 10));
        track->insertSegment(new Segment(25, 10));
        track->insertSegment(new Segment(50, 10));
        track->insertSegment(new Segment(55, 10));
        track->insertSegment(new Segment(75, 10));

        AssertionSubscriberMock assertionSubscriber;
        SegmentTrackTest testsuite(&dfile, theader);
        testsuite.addAssertionSubscriber(&assertionSubscriber);

        testsuite.singleStamp(25);
        testsuite.singleStamp(26);
        testsuite.singleStamp(50);
        testsuite.singleStamp(52);
        REQUIRE(assertionSubscriber.totalAssertions() == 4);
        REQUIRE(assertionSubscriber.assertionAt(0)->result() == SegmentAssertion::MATCH);
        REQUIRE(assertionSubscriber.assertionAt(1)->result() == SegmentAssertion::MATCH);
        REQUIRE(assertionSubscriber.assertionAt(2)->result() == SegmentAssertion::MATCH);
        REQUIRE(assertionSubscriber.assertionAt(3)->result() == SegmentAssertion::MISS);

        assertionSubscriber.removeAssertions();
        testsuite.multiStamp(60);
        testsuite.multiStamp(61);
        testsuite.multiStamp(62);
        REQUIRE(assertionSubscriber.assertionAt(0)->result() == SegmentAssertion::MATCH);
        REQUIRE(assertionSubscriber.assertionAt(1)->result() == SegmentAssertion::MATCH);
        REQUIRE(assertionSubscriber.assertionAt(2)->result() == SegmentAssertion::MATCH);

        assertionSubscriber.removeAssertions();
        testsuite.advanceCursorPosition(90);
        REQUIRE(assertionSubscriber.totalAssertions() == 1);
        REQUIRE(assertionSubscriber.assertionAt(0)->result() == SegmentAssertion::UNMARKED);
    }

    SECTION("Single Sequence - Multi Segment - Match(Overlap)"){
        DataFile dfile;
        TrackHeader* theader = dfile.appendTrack("Segment", "Track");
        Sequence* seq = new Sequence("test1", "StandardVideoDecoder", Sequence::Video, 200);
        dfile.appendSequence(seq);

        SegmentTrack* track = static_cast<SegmentTrack*>(seq->track("Track"));
        track->insertSegment(new Segment(20, 10));
        track->insertSegment(new Segment(50, 10));
        track->insertSegment(new Segment(80, 10));
        track->insertSegment(new Segment(100, 10));
        track->insertSegment(new Segment(120, 10));
        track->insertSegment(new Segment(150, 10));

        AssertionSubscriberMock assertionSubscriber;
        SegmentTrackTest testsuite(&dfile, theader);
        testsuite.addAssertionSubscriber(&assertionSubscriber);

        SegmentTrackTest::OverlapParameters oparamsMiss;
        oparamsMiss.maxMissedLength = 5;
        testsuite.singleOverlap(14, 15, oparamsMiss);
        testsuite.singleOverlap(15, 15, oparamsMiss);
        REQUIRE(assertionSubscriber.totalAssertions() == 2);
        REQUIRE(assertionSubscriber.assertionAt(0)->result() == SegmentAssertion::MISS);
        REQUIRE(assertionSubscriber.assertionAt(1)->result() == SegmentAssertion::MATCH);
        assertionSubscriber.removeAssertions();

        oparamsMiss.maxMissedPercent = 0.7;
        testsuite.singleOverlap(45, 7, oparamsMiss);
        testsuite.singleOverlap(45, 10, oparamsMiss);
        REQUIRE(assertionSubscriber.totalAssertions() == 2);
        REQUIRE(assertionSubscriber.assertionAt(0)->result() == SegmentAssertion::MISS);
        REQUIRE(assertionSubscriber.assertionAt(1)->result() == SegmentAssertion::MATCH);
        assertionSubscriber.removeAssertions();


        SegmentTrackTest::OverlapParameters oparamsUnmark;
        oparamsUnmark.maxUnmarkedLength = 5;
        testsuite.singleOverlap(80, 4, oparamsUnmark);
        testsuite.singleOverlap(80, 5, oparamsUnmark);
        REQUIRE(assertionSubscriber.totalAssertions() == 2);
        REQUIRE(assertionSubscriber.assertionAt(0)->result() == SegmentAssertion::MISS);
        REQUIRE(assertionSubscriber.assertionAt(1)->result() == SegmentAssertion::MATCH);
        assertionSubscriber.removeAssertions();

        oparamsUnmark.maxUnmarkedLength  = 0;
        oparamsUnmark.maxUnmarkedPercent = 0.7;
        testsuite.singleOverlap(100, 2, oparamsUnmark);
        testsuite.singleOverlap(100, 3, oparamsUnmark);
        REQUIRE(assertionSubscriber.totalAssertions() == 2);
        REQUIRE(assertionSubscriber.assertionAt(0)->result() == SegmentAssertion::MISS);
        REQUIRE(assertionSubscriber.assertionAt(1)->result() == SegmentAssertion::MATCH);
        assertionSubscriber.removeAssertions();


        SegmentTrackTest::OverlapParameters oparamsOverlapToSegment;
        oparamsOverlapToSegment.minOverlapLength = 6;
        testsuite.singleOverlap(120, 5, oparamsOverlapToSegment);
        REQUIRE(assertionSubscriber.lastAssertion() != 0);
        REQUIRE(assertionSubscriber.lastAssertion()->result() == SegmentAssertion::MISS);
        assertionSubscriber.removeAssertions();

        oparamsOverlapToSegment.minOverlapPercentToSegment = 0.7;
        testsuite.singleOverlap(120, 6, oparamsOverlapToSegment);
        REQUIRE(assertionSubscriber.lastAssertion() != 0);
        REQUIRE(assertionSubscriber.lastAssertion()->result() == SegmentAssertion::MISS);
        assertionSubscriber.removeAssertions();

        testsuite.singleOverlap(120, 7, oparamsOverlapToSegment);
        REQUIRE(assertionSubscriber.lastAssertion() != 0);
        REQUIRE(assertionSubscriber.lastAssertion()->result() == SegmentAssertion::MATCH);
        assertionSubscriber.removeAssertions();

        SegmentTrackTest::OverlapParameters oparamsOverlapToAssertion;
        oparamsOverlapToAssertion.minOverlapPercentToAssertion = 0.5;
        testsuite.singleOverlap(145, 7, oparamsOverlapToAssertion);
        testsuite.singleOverlap(145, 10, oparamsOverlapToAssertion);
        REQUIRE(assertionSubscriber.totalAssertions() == 2);
        REQUIRE(assertionSubscriber.assertionAt(0)->result() == SegmentAssertion::MISS);
        REQUIRE(assertionSubscriber.assertionAt(1)->result() == SegmentAssertion::MATCH);
        assertionSubscriber.removeAssertions();

        testsuite.advanceCursorPosition(199);
        REQUIRE(assertionSubscriber.lastAssertion() == 0);
    }

    SECTION("Single Sequence - Multi Segment - Match(Stamp) - Segment Overlap"){
        DataFile dfile;
        TrackHeader* theader = dfile.appendTrack("Segment", "Track");
        Sequence* seq = new Sequence("test1", "StandardVideoDecoder", Sequence::Video, 100);
        dfile.appendSequence(seq);

        SegmentTrack* track = static_cast<SegmentTrack*>(seq->track("Track"));
        track->insertSegment(new Segment(20, 50));
        track->insertSegment(new Segment(25, 30));
        track->insertSegment(new Segment(35, 50));
        track->insertSegment(new Segment(50, 10));
        track->insertSegment(new Segment(65, 20));
        track->insertSegment(new Segment(75, 10));

        AssertionSubscriberMock assertionSubscriber;
        SegmentTrackTest testsuite(&dfile, theader);
        testsuite.addAssertionSubscriber(&assertionSubscriber);

        testsuite.singleStamp(25);
        testsuite.singleStamp(26);
        testsuite.singleStamp(50);
        testsuite.singleStamp(51);
        REQUIRE(assertionSubscriber.totalAssertions() == 4);
        REQUIRE(assertionSubscriber.assertionAt(0)->result() == SegmentAssertion::MATCH);
        REQUIRE(assertionSubscriber.assertionAt(1)->result() == SegmentAssertion::MATCH);
        REQUIRE(assertionSubscriber.assertionAt(2)->result() == SegmentAssertion::MATCH);
        REQUIRE(assertionSubscriber.assertionAt(3)->result() == SegmentAssertion::MATCH);

        assertionSubscriber.removeAssertions();
        testsuite.advanceCursorPosition(90);
        REQUIRE(assertionSubscriber.totalAssertions() == 2);
        REQUIRE(assertionSubscriber.assertionAt(0)->result() == SegmentAssertion::UNMARKED);
        REQUIRE(assertionSubscriber.assertionAt(1)->result() == SegmentAssertion::UNMARKED);

        REQUIRE(testsuite.countAssertions(SegmentAssertion::MATCH) == 4);
    }

    SECTION("Multi Sequence - Divided Segments - No Match"){
        DataFile dfile;
        TrackHeader* theader = dfile.appendTrack("Segment", "Track");
        Sequence* seq = new Sequence("test1", "StandardVideoDecoder", Sequence::Video, 100);
        Sequence* seq2 = new Sequence("test2", "StandardVideoDecoder", Sequence::Video, 100);
        dfile.appendSequence(seq);
        dfile.appendSequence(seq2);

        SegmentTrack* track  = static_cast<SegmentTrack*>(seq->track("Track"));
        track->insertSegment(new Segment(20, 50));
        track->insertSegment(new Segment(30, 30));
        track->insertSegment(new Segment(30, 30));
        SegmentTrack* track2 = static_cast<SegmentTrack*>(seq2->track("Track"));
        track2->insertSegment(new Segment(10, 10));
        track2->insertSegment(new Segment(25, 20));

        AssertionSubscriberMock assertionSubscriber;
        SegmentTrackTest testsuite(&dfile, theader);
        testsuite.addAssertionSubscriber(&assertionSubscriber);

        testsuite.singleStamp(10);
        testsuite.singleStamp(80);
        testsuite.advanceCursorSequence(dfile.sequencesBegin() + 1);
        REQUIRE(assertionSubscriber.totalAssertions() == 5);
        REQUIRE(assertionSubscriber.assertionAt(0)->result() == SegmentAssertion::MISS);
        REQUIRE(assertionSubscriber.assertionAt(1)->result() == SegmentAssertion::MISS);
        REQUIRE(assertionSubscriber.assertionAt(2)->result() == SegmentAssertion::UNMARKED);
        REQUIRE(assertionSubscriber.assertionAt(3)->result() == SegmentAssertion::UNMARKED);
        REQUIRE(assertionSubscriber.assertionAt(4)->result() == SegmentAssertion::UNMARKED);

        assertionSubscriber.removeAssertions();
        testsuite.singleStamp(20);
        testsuite.advanceCursorPosition(60);
        REQUIRE(assertionSubscriber.totalAssertions() == 3);
        REQUIRE(assertionSubscriber.assertionAt(0)->result() == SegmentAssertion::MISS);
        REQUIRE(assertionSubscriber.assertionAt(1)->result() == SegmentAssertion::UNMARKED);
        REQUIRE(assertionSubscriber.assertionAt(2)->result() == SegmentAssertion::UNMARKED);

        REQUIRE(testsuite.countAssertions(SegmentAssertion::UNMARKED) == 5);
    }

    SECTION("Multi Sequence - Divided Segments - Match(Stamp)"){
        DataFile dfile;
        TrackHeader* theader = dfile.appendTrack("Segment", "Track");
        Sequence* seq  = new Sequence("test1", "StandardVideoDecoder", Sequence::Video, 100);
        Sequence* seq2 = new Sequence("test2", "StandardVideoDecoder", Sequence::Video, 100);
        dfile.appendSequence(seq);
        dfile.appendSequence(seq2);

        SegmentTrack* track  = static_cast<SegmentTrack*>(seq->track("Track"));
        track->insertSegment(new Segment(20, 50));
        track->insertSegment(new Segment(30, 30));
        track->insertSegment(new Segment(30, 30));
        SegmentTrack* track2 = static_cast<SegmentTrack*>(seq2->track("Track"));
        track2->insertSegment(new Segment(10, 10));
        track2->insertSegment(new Segment(25, 20));

        AssertionSubscriberMock assertionSubscriber;
        SegmentTrackTest testsuite(&dfile, theader);
        testsuite.addAssertionSubscriber(&assertionSubscriber);

        testsuite.singleStamp(30);
        testsuite.singleStamp(30);
        testsuite.singleStamp(30);
        testsuite.advanceCursorSequence(dfile.sequencesBegin() + 1);
        REQUIRE(assertionSubscriber.totalAssertions() == 3);
        REQUIRE(assertionSubscriber.assertionAt(0)->result() == SegmentAssertion::MATCH);
        REQUIRE(assertionSubscriber.assertionAt(1)->result() == SegmentAssertion::MATCH);
        REQUIRE(assertionSubscriber.assertionAt(2)->result() == SegmentAssertion::MATCH);

        assertionSubscriber.removeAssertions();
        testsuite.singleStamp(10);
        testsuite.advanceCursorPosition(60);
        REQUIRE(assertionSubscriber.totalAssertions() == 2);
        REQUIRE(assertionSubscriber.assertionAt(0)->result() == SegmentAssertion::MATCH);
        REQUIRE(assertionSubscriber.assertionAt(1)->result() == SegmentAssertion::UNMARKED);
    }

    SECTION("Multi Sequnce - Overlap - Exception"){
        DataFile dfile;
        TrackHeader* theader = dfile.appendTrack("Segment", "Track");
        Sequence* seq = new Sequence("test1", "StandardVideoDecoder", Sequence::Video, 100);
        Sequence* seq2 = new Sequence("test2", "StandardVideoDecoder", Sequence::Video, 100);
        dfile.appendSequence(seq);
        dfile.appendSequence(seq2);

        SegmentTrack* track  = static_cast<SegmentTrack*>(seq->track("Track"));
        REQUIRE_THROWS_AS(track->insertSegment(new Segment(20, 81)), tg::Exception);
    }

}

}// namespace
