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
#include "tgtestsuite.h"
#include "tgsegment.h"
#include "tgsegmenttrack.h"
#include "tgsegmenttracktest.h"

#include "opencv2/core/core.hpp"

using namespace tg;

namespace tgtracktestdraw_test{

bool isFilledWith(const cv::Mat& mat, const cv::Scalar& color){
    for ( int i = 0; i < mat.rows; ++i ){
        const uchar* p = mat.ptr<uchar>(i);
        for ( int j = 0; j < mat.cols; ++j ){
            for ( int c = 0; c < mat.channels(); ++c ){
                if ( p[j * mat.channels() + c] != (int)(color[c]) ){
                    printf("%d", p[j * mat.channels() + c]);
                    return false;
                }
            }
        }
    }
    return true;
}

TEST_CASE("Teground tracktest Draw Test", "[tracktestdrawtestcase]"){

    SECTION("Label And Segment Tracks Draw Test"){
        DataFile dfile;
        TrackHeader* theader = dfile.appendTrack("Segment", "Track");
        Sequence* seq  = new Sequence("test1", "StandardVideoDecoder", Sequence::Video, 200);
        Sequence* seq2 = new Sequence("test2", "StandardVideoDecoder", Sequence::Video, 200);
        dfile.appendSequence(seq);
        dfile.appendSequence(seq2);

        SegmentTrack* track  = static_cast<SegmentTrack*>(seq->track("Track"));
        track->insertSegment(new Segment(20, 50));
        track->insertSegment(new Segment(80, 100));
        SegmentTrack* track2 = static_cast<SegmentTrack*>(seq2->track("Track"));
        track2->insertSegment(new Segment(10, 10));
        track2->insertSegment(new Segment(25, 20));

        cv::Mat drawSurface;

        SegmentTrackTest* tracktest = new SegmentTrackTest(&dfile, theader);
        tracktest->singleStamp(10);
        tracktest->singleStamp(50);

        tracktest->advanceCursorSequence(dfile.sequencesBegin() + 1);
        tracktest->singleStamp(15);

        TestSuite testsuite(&dfile, "Test");
        testsuite.addTest(tracktest);

        testsuite.draw(drawSurface, dfile.sequencesBegin(), 160, 100, 10, 30);
    }

    SECTION("Segment Track Draw Test"){
        DataFile dfile;
        TrackHeader* theader = dfile.appendTrack("Segment", "Track");
        Sequence* seq  = new Sequence("test1", "StandardVideoDecoder", Sequence::Video, 200);
        Sequence* seq2 = new Sequence("test2", "StandardVideoDecoder", Sequence::Video, 200);
        dfile.appendSequence(seq);
        dfile.appendSequence(seq2);

        SegmentTrack* track  = static_cast<SegmentTrack*>(seq->track("Track"));
        track->insertSegment(new Segment(20, 50));
        track->insertSegment(new Segment(80, 100));
        track->insertSegment(new Segment(30, 30));
        SegmentTrack* track2 = static_cast<SegmentTrack*>(seq2->track("Track"));
        track2->insertSegment(new Segment(10, 10));
        track2->insertSegment(new Segment(25, 20));

        cv::Mat drawSurface;

        SegmentTrackTest tracktest(&dfile, theader);
        tracktest.draw(drawSurface, dfile.sequencesBegin(), 0, 100, 10, 30);

        REQUIRE(isFilledWith(
            drawSurface(cv::Rect(TrackTest::DRAW_HEADER_WIDTH + 10 * 20, 0, 10 * 30, 30)),
            cv::Scalar(84, 84, 84)
        ));

        tracktest.singleStamp(10);
        tracktest.singleStamp(50);

        tracktest.draw(drawSurface, dfile.sequencesBegin(), 0, 100, 10, 30);

        REQUIRE(isFilledWith(
            drawSurface(cv::Rect(TrackTest::DRAW_HEADER_WIDTH + 10 * 10, 0, 10, 30)),
            cv::Scalar(30, 30, 120)
        ));

        REQUIRE(isFilledWith(
            drawSurface(cv::Rect(TrackTest::DRAW_HEADER_WIDTH + 10 * 50, 0, 10, 30)),
            cv::Scalar(30, 120, 30)
        ));
    }

}

} // namespace
