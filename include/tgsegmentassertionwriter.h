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

#ifndef TGSEGMENTASSERTIONWRITER_H
#define TGSEGMENTASSERTIONWRITER_H

#include "tgglobal.h"
#include "tgSegmentTrackTest.h"
#include <iostream>
#include <iomanip>

namespace tg{

class SegmentAssertionConsoleWriter : public SegmentAssertionSubscriber{

public:
    SegmentAssertionConsoleWriter(TrackHeader* track): m_track(track), m_sequence(0){}
    ~SegmentAssertionConsoleWriter(){}

    virtual void onSequenceSet(Sequence* sequence);
    virtual void onAssertionInsert(SegmentAssertion* assertion);

private:
    TrackHeader* m_track;
    Sequence*    m_sequence;
};

inline void SegmentAssertionConsoleWriter::onSequenceSet(Sequence *sequence){
    std::cout <<
        "\n------------------------------------------------------------\n" <<
        sequence->path() <<
        "\n------------------------------------------------------------\n" <<
    std::endl;

    m_sequence = sequence;
}

inline void SegmentAssertionConsoleWriter::onAssertionInsert(SegmentAssertion *assertion){
    std::string typeString = "";
    switch( assertion->type() ){
    case SegmentAssertion::SINGLE_STAMP:
    case SegmentAssertion::MULTI_STAMP: typeString = "Stamp"; break;
    case SegmentAssertion::SINGLE_OVERLAP:
    case SegmentAssertion::MULTI_OVERLAP: typeString = "Segment"; break;
    case SegmentAssertion::UNMARKED_SEGMENT: typeString = "Unmarked"; break;
    }

    std::string resultString = "";
    switch( assertion->result() ){
    case SegmentAssertion::MATCH:    resultString = "[MATCH   ]"; break;
    case SegmentAssertion::MISS:     resultString = "[MISS    ]"; break;
    case SegmentAssertion::UNMARKED: resultString = "[UNMARKED]"; break;
    }

    std::cout <<
        resultString <<
         " POS[" <<
            m_track->name() << ":" <<
            std::setfill('0') << std::setw(5) << assertion->position() << ":" <<
            assertion->length() <<
         "]" <<
         " TYPE[" << typeString << "]";

    if ( assertion->hasSegment() )
        std::cout << " SEGMENT[" << assertion->segment()->position() << ", " << assertion->segment()->length() << "]";

    if ( assertion->hasInfo() )
        std::cout << " INFO[" << assertion->info() << "]";

    if ( assertion->hasFile() )
        std::cout << " FILE[" << assertion->file() << ":" << assertion->lineNumber() << "]";

    std::cout << std::endl;
}

}// namespace

#endif // TGSEGMENTASSERTIONWRITER_H
