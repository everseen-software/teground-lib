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

#include "opencv2/core/core.hpp"

using namespace tg;

namespace tgsequence_test{

TEST_CASE("Teground Sequence Test", "[sequencetestcase]"){

    SECTION("Track Insertion and Removal"){
        tg::DataFile dfile;

        Sequence* seq1 = new Sequence("sequence1", "StandardVideoDecoder", Sequence::Video, 1000);
        Sequence* seq2 = new Sequence("sequence2", "StandardVideoDecoder", Sequence::Video, 2000);

        dfile.appendSequence(seq1);
        dfile.appendSequence(seq2);
        REQUIRE(dfile.sequenceCount() == 2);

        TrackHeader* track1Head = dfile.appendTrack("Segment", "Track1");
        TrackHeader* track2Head = dfile.appendTrack("Segment", "Track2");

        REQUIRE(seq1->totalTracks() == 2);
        REQUIRE(seq2->totalTracks() == 2);
        REQUIRE(seq1->track("Track1") != 0);
        REQUIRE(seq2->track("Track1") != 0);

        dfile.removeTrack(track1Head);

        REQUIRE(seq1->track("Track1") == 0);
        REQUIRE(seq2->track("Track1") == 0);

        REQUIRE(seq1->track("Track2") != 0);
        REQUIRE(seq2->track("Track2") != 0);
        REQUIRE(seq1->track("Track2")->length() == 1000);
        REQUIRE(seq2->track("Track2")->length() == 2000);
    }

    SECTION("Track association and sequence removal"){
        tg::DataFile dfile;

        TrackHeader* track1Head = dfile.appendTrack("Segment", "Track1");
        TrackHeader* track2Head = dfile.appendTrack("Segment", "Track2");

        Sequence* seq1 = new Sequence("sequence1", "StandardVideoDecoder", Sequence::Video, 1000);
        Sequence* seq2 = new Sequence("sequence2", "StandardVideoDecoder", Sequence::Video, 2000);

        dfile.appendSequence(seq1);
        dfile.appendSequence(seq2);
        REQUIRE(dfile.sequenceCount() == 2);

        REQUIRE(seq1->totalTracks() == 2);
        REQUIRE(seq2->totalTracks() == 2);
        REQUIRE(seq1->track("Track1") != 0);
        REQUIRE(seq2->track("Track1") != 0);

        dfile.removeSequence(seq1);
        REQUIRE(dfile.sequenceCount() == 1);
    }

}

}// namespace
