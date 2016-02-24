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

#ifndef TGTESTSUITE_H
#define TGTESTSUITE_H

#include "tgglobal.h"
#include "tgdatafile.h"
#include "tgtracktest.h"
#include "tgsegmenttracktest.h"

namespace tg{

class TestSuite{

public:
    TestSuite(const DataFile* dataFile, const std::string& name);
    ~TestSuite();

    void addTest(TrackTest* testSuite);
    const DataFile* dataFile() const;

    void read(const cv::FileNode& node);
    void write(cv::FileStorage& fs) const;

    bool readFrom(const std::string& path);
    bool writeTo(const std::string& path);

    void clearTests();

    void draw(
        cv::Mat& dst,
        DataFile::SequenceIterator seqIt,
        VideoTime framePosition,
        VideoTime numberOfFrames = 100,
        int pixelsPerFrame = 10,
        int trackHeight = 30
    );

private:
    const DataFile* m_data;
    std::string m_name;
    std::vector<TrackTest*> m_tests;

};

inline TestSuite::TestSuite(const DataFile *dataFile, const std::string &name)
    : m_data(dataFile)
    , m_name(name)
{
    TrackTest::registerSubtype<SegmentTrackTest>("SegmentTrackTest");
}

inline TestSuite::~TestSuite(){
    clearTests();
}

inline void TestSuite::addTest(TrackTest *testSuite){
    m_tests.push_back(testSuite);
}

inline const DataFile *TestSuite::dataFile() const{
    return m_data;
}

inline void TestSuite::read(const cv::FileNode &node){
    clearTests();

    m_name = (std::string)node["Name"];

    cv::FileNode nodeTracks = node["TrackTests"];
    if ( nodeTracks.type() != cv::FileNode::SEQ )
        throw Exception("\'TrackTestSuites\' is not an iterable type.");

    for (cv::FileNodeIterator it = nodeTracks.begin(); it != nodeTracks.end(); ++it){
        const cv::FileNode& trackNode = *it;
        std::string testSubtype = (std::string)(trackNode["Type"]);
        if ( !TrackTest::hasSubtype(testSubtype) )
            throw Exception("Test SubType does not exist when parsing test file: " + testSubtype);

        size_t headerIndex = static_cast<size_t>(static_cast<double>(trackNode["Header"]));
        if ( headerIndex > dataFile()->trackCount() ){
            std::stringstream errorStream;
            errorStream << "Out of bounds header index: " << headerIndex;
            throw Exception(errorStream.str());
        }

        TrackTest* test = TrackTest::makeSubtype(testSubtype, dataFile()->trackAt(headerIndex), m_data);
        test->read(trackNode);
        addTest(test);
    }
}

inline void TestSuite::write(cv::FileStorage &fs) const{
    fs << "TeGround" << "{";

    fs << "{";
    fs << "Name" << m_name;
    fs << "TrackTests" << "[";
    for ( std::vector<TrackTest*>::const_iterator it = m_tests.begin(); it != m_tests.end(); ++it )
        (*it)->write(fs);
    fs << "]";
    fs << "}";

    fs << "}";
}

inline bool TestSuite::readFrom(const std::string &path){
    try{
        cv::FileStorage fs(path, cv::FileStorage::READ | cv::FileStorage::FORMAT_YAML);
        if ( !fs.isOpened() )
            return false;
        read(fs["TeGroundTestSuite"]);
        fs.release();
    } catch ( cv::Exception& e ){
        throw tg::Exception(e.what());
    }

    return true;
}

inline bool TestSuite::writeTo(const std::string &path){
    cv::FileStorage fs(path, cv::FileStorage::WRITE | cv::FileStorage::FORMAT_YAML);
    if ( !fs.isOpened() )
        return false;
    write(fs);
    fs.release();
    return true;
}

inline void TestSuite::clearTests(){
    for ( std::vector<TrackTest*>::const_iterator it = m_tests.begin(); it != m_tests.end(); ++it )
        delete *it;
    m_tests.clear();
}

void TestSuite::draw(
    cv::Mat &dst,
    DataFile::SequenceIterator seqIt,
    VideoTime framePosition,
    VideoTime numberOfFrames,
    int pixelsPerFrame,
    int trackHeight)
{
    if ( framePosition >= (*seqIt)->length() )
        return;

    // Create surface
    // --------------

    int dstWidth = TrackTest::DRAW_HEADER_WIDTH + pixelsPerFrame * (int)numberOfFrames;
    if ( trackHeight < 10 )
        trackHeight = 10;

    dst.create(
        cv::Size( dstWidth, trackHeight * ((int)m_tests.size() + 1)),
        CV_8UC3
    );
    dst.setTo(cv::Scalar(70, 70, 70));

    // Draw Heading
    // ------------

    cv::putText(
        dst,
        m_name.substr(0, 9),
        cv::Point(10, trackHeight / 2 + 10),
        cv::FONT_HERSHEY_SIMPLEX,
        0.44,
        cv::Scalar(150, 150, 150));

    // Draw tracks
    // -----------

    int drawPosition = trackHeight;
    for ( std::vector<TrackTest*>::iterator it = m_tests.begin(); it != m_tests.end(); ++it ){
        TrackTest* t = *it;
        cv::Mat trackRegion = dst(cv::Rect(0, drawPosition, dstWidth, trackHeight));
        t->draw(
            trackRegion,
            seqIt,
            framePosition,
            numberOfFrames,
            pixelsPerFrame,
            trackHeight
        );
        drawPosition += trackHeight;
    }

    // Draw markers and labels
    // -----------------------

    const int minHeaderLabelDistance = 100;

    int textDistance  = cv::max(minHeaderLabelDistance, pixelsPerFrame);
    int frameDivider  = textDistance / pixelsPerFrame;
    int labeledFrameDivider = 1;
    while( true ){
        if ( labeledFrameDivider >= frameDivider  )
            break;
        if ( labeledFrameDivider * 2 >= frameDivider ){
            labeledFrameDivider = labeledFrameDivider * 2;
            break;
        }
        if ( labeledFrameDivider * 5 >= frameDivider  ){
            labeledFrameDivider = labeledFrameDivider * 5;
            break;
        }

        labeledFrameDivider = labeledFrameDivider * 10;
    }

    int markedFrameDivider = (int)std::ceil(labeledFrameDivider / 4);

    VideoTime currentFrameNumber = framePosition;
    for ( VideoTime i = 0; i < numberOfFrames; ++i ){
        if ( currentFrameNumber % labeledFrameDivider == 0 &&
             currentFrameNumber + labeledFrameDivider <= (*seqIt)->length()
        ){
            int markerPosition = TrackTest::DRAW_HEADER_WIDTH + (int)i * pixelsPerFrame;
            int labelPosition  = currentFrameNumber == 0 ? markerPosition + 3 : markerPosition - 15;

            cv::line(
                dst,
                cv::Point(markerPosition, trackHeight - 5),
                cv::Point(markerPosition, trackHeight),
                cv::Scalar(120, 120, 120)
            );

            std::string label = currentFrameNumber == 0 ? "0" : cv::format("%.5d", currentFrameNumber);
            cv::putText(
                dst,
                label,
                cv::Point(labelPosition, trackHeight - 10),
                cv::FONT_HERSHEY_SIMPLEX,
                0.35,
                cv::Scalar(200, 200, 200));
        } else if ( currentFrameNumber == (*seqIt)->length() - 1){
            int labelPosition  = TrackTest::DRAW_HEADER_WIDTH + (int)i * pixelsPerFrame - 30;
            cv::putText(
                dst,
                cv::format("%.5d", currentFrameNumber),
                cv::Point(labelPosition, trackHeight - 10),
                cv::FONT_HERSHEY_SIMPLEX,
                0.35,
                cv::Scalar(150, 150, 150));
        }
        if ( currentFrameNumber % markedFrameDivider == 0 ){
            int markerPosition = TrackTest::DRAW_HEADER_WIDTH + (int)i * pixelsPerFrame;

            cv::line(
                dst,
                cv::Point(markerPosition, trackHeight - 2),
                cv::Point(markerPosition, trackHeight),
                cv::Scalar(120, 120, 120)
            );
        }
        currentFrameNumber++;
        if ( currentFrameNumber >= (*seqIt)->length() ){
            currentFrameNumber = 0;
            ++seqIt;
            int delimiterPosition = TrackTest::DRAW_HEADER_WIDTH + (int)i * pixelsPerFrame + pixelsPerFrame;
            cv::line(
                dst,
                cv::Point(delimiterPosition, 0 ),
                cv::Point(delimiterPosition, trackHeight * ((int)m_tests.size() + 1)),
                cv::Scalar(100, 100, 100)
            );
            if ( seqIt == m_data->sequencesEnd() )
                break;
        }
    }
}


} // namespace

#ifndef TG_DISABLE_MACROS

#define TG_TEST_SUITE(_var, _dataFile, _name) \
    tg::TestSuite _var(&_dataFile, _name)

#define TG_TEST_SUITE_ADD_TEST(_var, _trackTestOb) \
    _var.addTest(_trackTestOb)

#endif



#endif // TGTESTSUITE
