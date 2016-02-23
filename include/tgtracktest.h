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

#ifndef TGTRACKTESTSUITE
#define TGTRACKTESTSUITE

#include "tgglobal.h"
#include "tgdatafile.h"

namespace tg{

class TrackTest{

    // Const Definitions
    // -----------------

public:
    static const int DRAW_HEADER_WIDTH = 100;


    // Factory Functions
    // -----------------

public:
    typedef TrackTest* (*TrackTestMakeFunction)(const DataFile* data, const TrackHeader* track);

private:
    class TrackTestFactory{
    public:
        TrackTestFactory(const std::string pType, TrackTestMakeFunction pMakeFunction)
            : type(pType)
            , makeFunction(pMakeFunction)
        {}

        std::string           type;
        TrackTestMakeFunction makeFunction;

        template<class T> static TrackTest* make(const DataFile* data, const TrackHeader* track){
            return new T(data, track);
        }
    };

    static std::vector<TrackTestFactory>& registeredTypes() {
        static std::vector<TrackTestFactory> types;
        return types;
    }

public:
    template<class T> static void registerSubtype(const std::string& typeName){
        if ( !hasSubtype(typeName) )
            registeredTypes().push_back(TrackTestFactory(typeName, &TrackTestFactory::make<T>));
    }

    static bool hasSubtype(const std::string& type){
        for ( std::vector<TrackTestFactory>::iterator it = registeredTypes().begin();
              it != registeredTypes().end();
              ++it
        ){
            if ( it->type == type)
                return true;
        }
        return false;
    }

    static TrackTest* makeSubtype(const std::string& typeName, const TrackHeader* header, const DataFile* data){
        for ( std::vector<TrackTestFactory>::iterator it = registeredTypes().begin();
              it != registeredTypes().end();
              ++it
        ){
            if ( it->type == typeName )
                return it->makeFunction(data, header);
        }
        return 0;
    }

    // Members
    // -------

public:
    TrackTest(const DataFile* data, const TrackHeader* track);
    virtual ~TrackTest();

    const TrackHeader* trackHeader() const;
    const DataFile* data() const;

    virtual void read(const cv::FileNode& node) = 0;
    virtual void write(cv::FileStorage& fs) const = 0;
    virtual bool isEnd() const = 0;

    virtual void draw(
        cv::Mat& dst,
        DataFile::SequenceIterator seqIt,
        VideoTime framePosition,
        VideoTime numberOfFrames = 100,
        int pixelsPerFrame = 10,
        int trackHeight = 30
    ) = 0;

private:
    const TrackHeader* m_trackHeader;
    const DataFile*    m_data;
};

inline TrackTest::TrackTest(const DataFile *data, const TrackHeader *track)
    : m_trackHeader(track)
    , m_data(data)
{
}

inline TrackTest::~TrackTest(){
}

inline const TrackHeader *TrackTest::trackHeader() const{
    return m_trackHeader;
}

inline const DataFile *TrackTest::data() const{
    return m_data;
}


}// namespace


#endif // TGTRACKTESTSUITE
