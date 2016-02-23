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

#include "tgsegment.h"
#include "tgsegmenttrack.h"

#include "opencv2/core/core.hpp"

using namespace tg;

namespace tgsegmenttrack_test{

inline Segment* getSegment(SegmentTrack& t, size_t index){
    return *(t.begin() + index);
}

inline bool matchSegmentCoords(SegmentTrack& t, size_t index, VideoTime pos, VideoTime length = -1){
    Segment* segm = getSegment(t, index);
    return (segm->position() == pos && (length == -1 || segm->length() == length));
}

TEST_CASE("Teground Segment Test", "[segmenttracktestcase]"){

    SECTION("Ascending Insertion"){
        SegmentTrack t(0, 100);
        t.insertSegment(new Segment(1, 1));
        t.insertSegment(new Segment(2, 1));
        t.insertSegment(new Segment(3, 1));
        t.insertSegment(new Segment(4, 1));

        REQUIRE(t.totalSegments() == 4);
        REQUIRE(matchSegmentCoords(t, 0, 1));
        REQUIRE(matchSegmentCoords(t, 1, 2));
        REQUIRE(matchSegmentCoords(t, 2, 3));
        REQUIRE(matchSegmentCoords(t, 3, 4));
    }

    SECTION("Ascending Equal Insertion"){
        SegmentTrack t(0, 100);
        t.insertSegment(new Segment(1, 1));
        t.insertSegment(new Segment(1, 1));
        t.insertSegment(new Segment(3, 1));
        t.insertSegment(new Segment(3, 1));
        t.insertSegment(new Segment(3, 2));
        t.insertSegment(new Segment(4, 1));
        t.insertSegment(new Segment(4, 1));
        t.insertSegment(new Segment(4, 1));
        t.insertSegment(new Segment(4, 2));

        REQUIRE(t.totalSegments() == 9);
        REQUIRE(matchSegmentCoords(t, 0, 1, 1));
        REQUIRE(matchSegmentCoords(t, 1, 1, 1));
        REQUIRE(matchSegmentCoords(t, 2, 3, 1));
        REQUIRE(matchSegmentCoords(t, 3, 3, 1));
        REQUIRE(matchSegmentCoords(t, 4, 3, 2));
        REQUIRE(matchSegmentCoords(t, 5, 4, 1));
        REQUIRE(matchSegmentCoords(t, 6, 4, 1));
        REQUIRE(matchSegmentCoords(t, 7, 4, 1));
        REQUIRE(matchSegmentCoords(t, 8, 4, 2));
    }

    SECTION("Descending Insertion"){
        SegmentTrack t(0, 100);
        t.insertSegment(new Segment(4, 1));
        t.insertSegment(new Segment(3, 1));
        t.insertSegment(new Segment(2, 1));
        t.insertSegment(new Segment(1, 1));

        REQUIRE(t.totalSegments() == 4);
        REQUIRE(matchSegmentCoords(t, 0, 1));
        REQUIRE(matchSegmentCoords(t, 1, 2));
        REQUIRE(matchSegmentCoords(t, 2, 3));
        REQUIRE(matchSegmentCoords(t, 3, 4));
    }

    SECTION("Descending Equal Insertion"){
        SegmentTrack t(0, 100);
        t.insertSegment(new Segment(4, 2));
        t.insertSegment(new Segment(4, 1));
        t.insertSegment(new Segment(4, 1));
        t.insertSegment(new Segment(4, 1));
        t.insertSegment(new Segment(3, 2));
        t.insertSegment(new Segment(3, 1));
        t.insertSegment(new Segment(3, 1));
        t.insertSegment(new Segment(1, 1));
        t.insertSegment(new Segment(1, 1));

        REQUIRE(t.totalSegments() == 9);
        REQUIRE(matchSegmentCoords(t, 0, 1, 1));
        REQUIRE(matchSegmentCoords(t, 1, 1, 1));
        REQUIRE(matchSegmentCoords(t, 2, 3, 1));
        REQUIRE(matchSegmentCoords(t, 3, 3, 1));
        REQUIRE(matchSegmentCoords(t, 4, 3, 2));
        REQUIRE(matchSegmentCoords(t, 5, 4, 1));
        REQUIRE(matchSegmentCoords(t, 6, 4, 1));
        REQUIRE(matchSegmentCoords(t, 7, 4, 1));
        REQUIRE(matchSegmentCoords(t, 8, 4, 2));
    }

    SECTION("Alternate Edge Insertion"){
        SegmentTrack t(0, 100);
        t.insertSegment(new Segment(4, 2));
        t.insertSegment(new Segment(2, 1));
        t.insertSegment(new Segment(5, 2));
        t.insertSegment(new Segment(1, 1));
        t.insertSegment(new Segment(6, 1));

        REQUIRE(t.totalSegments() == 5);
        REQUIRE(matchSegmentCoords(t, 0, 1, 1));
        REQUIRE(matchSegmentCoords(t, 1, 2, 1));
        REQUIRE(matchSegmentCoords(t, 2, 4, 2));
        REQUIRE(matchSegmentCoords(t, 3, 5, 2));
        REQUIRE(matchSegmentCoords(t, 4, 6, 1));
    }

    SECTION("Alternate Equal Edge Insertion"){
        SegmentTrack t(0, 100);
        t.insertSegment(new Segment(2, 2));
        t.insertSegment(new Segment(2, 2));
        t.insertSegment(new Segment(2, 1));
        t.insertSegment(new Segment(3, 1));
        t.insertSegment(new Segment(2, 1));
        t.insertSegment(new Segment(3, 1));
        t.insertSegment(new Segment(1, 1));
        t.insertSegment(new Segment(3, 2));

        REQUIRE(t.totalSegments() == 8);
        REQUIRE(matchSegmentCoords(t, 0, 1, 1));
        REQUIRE(matchSegmentCoords(t, 1, 2, 1));
        REQUIRE(matchSegmentCoords(t, 2, 2, 1));
        REQUIRE(matchSegmentCoords(t, 3, 2, 2));
        REQUIRE(matchSegmentCoords(t, 4, 2, 2));
        REQUIRE(matchSegmentCoords(t, 5, 3, 1));
        REQUIRE(matchSegmentCoords(t, 6, 3, 1));
        REQUIRE(matchSegmentCoords(t, 7, 3, 2));
    }

    SECTION("Alternate Internal Insertion"){
        SegmentTrack t(0, 100);
        t.insertSegment(new Segment(4, 2));
        t.insertSegment(new Segment(1, 1));
        t.insertSegment(new Segment(3, 2));
        t.insertSegment(new Segment(2, 1));
        t.insertSegment(new Segment(3, 1));

        REQUIRE(t.totalSegments() == 5);
        REQUIRE(matchSegmentCoords(t, 0, 1, 1));
        REQUIRE(matchSegmentCoords(t, 1, 2, 1));
        REQUIRE(matchSegmentCoords(t, 2, 3, 1));
        REQUIRE(matchSegmentCoords(t, 3, 3, 2));
        REQUIRE(matchSegmentCoords(t, 4, 4, 2));
    }

    SECTION("Alternate Internal Equal Insertion"){
        SegmentTrack t(0, 100);
        t.insertSegment(new Segment(4, 2));
        t.insertSegment(new Segment(1, 1));
        t.insertSegment(new Segment(3, 2));
        t.insertSegment(new Segment(2, 1));
        t.insertSegment(new Segment(3, 1));
        t.insertSegment(new Segment(2, 2));
        t.insertSegment(new Segment(3, 1));
        t.insertSegment(new Segment(2, 1));
        t.insertSegment(new Segment(2, 2));

        REQUIRE(t.totalSegments() == 9);
        REQUIRE(matchSegmentCoords(t, 0, 1, 1));
        REQUIRE(matchSegmentCoords(t, 1, 2, 1));
        REQUIRE(matchSegmentCoords(t, 2, 2, 1));
        REQUIRE(matchSegmentCoords(t, 3, 2, 2));
        REQUIRE(matchSegmentCoords(t, 4, 2, 2));
        REQUIRE(matchSegmentCoords(t, 5, 3, 1));
        REQUIRE(matchSegmentCoords(t, 6, 3, 1));
        REQUIRE(matchSegmentCoords(t, 7, 3, 2));
        REQUIRE(matchSegmentCoords(t, 8, 4, 2));
    }

    SECTION("Set Segment Coordinates Without Changing Position"){
        SegmentTrack t(0, 100);
        t.insertSegment(new Segment(1, 5));
        t.insertSegment(new Segment(10, 5));
        t.insertSegment(new Segment(20, 5));

        t.assignSegmentCoords(t.begin() + 0, 4, 10);
        t.assignSegmentCoords(t.begin() + 1, 15, 50);
        t.assignSegmentCoords(t.begin() + 2, 30, 5);

        REQUIRE(t.totalSegments() == 3);
        REQUIRE(matchSegmentCoords(t, 0, 4, 10));
        REQUIRE(matchSegmentCoords(t, 1, 15, 50));
        REQUIRE(matchSegmentCoords(t, 2, 30, 5));
    }

    SECTION("Set Segment Coordinates"){
        SegmentTrack t(0, 100);
        t.insertSegment(new Segment(1, 5));
        t.insertSegment(new Segment(10, 5));
        t.insertSegment(new Segment(20, 5));

        REQUIRE(t.totalSegments() == 3);
        t.assignSegmentCoords(t.begin() + 0, 12, 10);
        REQUIRE(matchSegmentCoords(t, 0, 10, 5));

        t.assignSegmentCoords(t.begin() + 0, 15, 20);
        REQUIRE(matchSegmentCoords(t, 0, 12, 10));

        t.assignSegmentCoords(t.begin() + 2, 0, 15);
        REQUIRE(matchSegmentCoords(t, 0, 0, 15));
    }

    SECTION("Set Segment Coordinates To Equal"){
        SegmentTrack t(0, 100);
        t.insertSegment(new Segment(1, 5));
        t.insertSegment(new Segment(10, 5));
        t.insertSegment(new Segment(20, 5));

        REQUIRE(t.totalSegments() == 3);
        t.assignSegmentCoords(t.begin() + 0, 20, 5);
        REQUIRE(matchSegmentCoords(t, 0, 10, 5));

        t.assignSegmentCoords(t.begin() + 0, 30, 5);
        REQUIRE(matchSegmentCoords(t, 0, 20, 5));

        t.assignSegmentCoords(t.begin() + 2, 20, 5);

        REQUIRE(matchSegmentCoords(t, 0, 20, 5));
        REQUIRE(matchSegmentCoords(t, 1, 20, 5));
        REQUIRE(matchSegmentCoords(t, 2, 20, 5));
    }

}

}// namespace
