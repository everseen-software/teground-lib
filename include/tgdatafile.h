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

#ifndef TGDATAFILE_H
#define TGDATAFILE_H

#include "tgglobal.h"
#include "tgsequence.h"
#include "tgtrackheader.h"
#include "tgtrack.h"
#include "tgsegmenttrack.h"
#include <vector>

#include <iostream>

namespace tg{

class DataFile{

public:
    typedef std::vector<Sequence*>::iterator          SequenceIterator;
    typedef std::vector<Sequence*>::const_iterator    SequenceConstIterator;

    typedef std::vector<TrackHeader*>::iterator       TrackHeaderIterator;
    typedef std::vector<TrackHeader*>::const_iterator TrackHeaderConstIterator;

public:
    DataFile();
    ~DataFile();

    bool readFrom(const std::string& path);
    bool writeTo(const std::string& path);

    void read(const cv::FileNode& node);
    void write(cv::FileStorage& fs) const;

    // Track Handlers
    // --------------

    size_t trackCount() const;
    size_t trackIndex(const TrackHeader *track) const;
    TrackHeader *trackAt(size_t index);
    const TrackHeader *trackAt(size_t index) const;
    TrackHeader *appendTrack(const std::string& type, const std::string& name = "Track");
    void removeTrack(TrackHeader* track);
    void clearTracks();

    TrackHeaderIterator tracksBegin();
    TrackHeaderConstIterator tracksBegin() const;
    TrackHeaderIterator tracksEnd();
    TrackHeaderConstIterator tracksEnd() const;


    // Sequence Handlers
    // -----------------

    size_t sequenceCount() const;
    void appendSequence(Sequence* seq);
    void removeSequence(Sequence* seq);
    Sequence* sequenceAt(size_t index);
    const Sequence* sequenceAt(size_t index) const;
    Sequence* takeSequence(Sequence* seq);
    void moveSequence(Sequence* seq, size_t index);
    Sequence* sequenceFrom(const std::string& path);
    void clearSequences();

    SequenceIterator sequencesBegin();
    SequenceConstIterator sequencesBegin() const;
    SequenceIterator sequencesEnd();
    SequenceConstIterator sequencesEnd() const;

private:
    // prevent copy
    DataFile(const DataFile&);
    DataFile& operator =(const DataFile&);

    // fields
    std::vector<Sequence*>    m_sequences;
    std::vector<TrackHeader*> m_tracks;
};

inline DataFile::DataFile(){
    TrackHeader::registerType<SegmentTrack>("Segment");
}

inline DataFile::~DataFile(){
    clearSequences();
    clearTracks();
}

inline bool DataFile::readFrom(const std::string& path){
    try{
        cv::FileStorage fs(path, cv::FileStorage::READ | cv::FileStorage::FORMAT_YAML);
        if ( !fs.isOpened() )
            return false;
        read(fs["TeGround"]);
        fs.release();
    } catch ( cv::Exception& e ){
        throw tg::Exception(e.what());
    }

    return true;
}

inline bool DataFile::writeTo(const std::string& path){
    cv::FileStorage fs(path, cv::FileStorage::WRITE | cv::FileStorage::FORMAT_YAML);
    if ( !fs.isOpened() )
        return false;
    write(fs);
    fs.release();
    return true;
}

inline void DataFile::read(const cv::FileNode &node){

    // Clear State

    clearSequences();
    clearTracks();

    // Iterate Tracks

    cv::FileNode nodeTracks = node["Tracks"];
    if ( nodeTracks.type() != cv::FileNode::SEQ )
        throw Exception("\'Tracks\' is not an iterable type.");

    for (cv::FileNodeIterator it = nodeTracks.begin(); it != nodeTracks.end(); ++it){
        const cv::FileNode& trackNode = *it;
        std::string trackType  = (std::string)(trackNode["Type"]);
        if ( !TrackHeader::hasType(trackType) )
            throw Exception("Type does not exist when parsing file: " + trackType);

        TrackHeader* theader = new TrackHeader(trackType);
        theader->read(trackNode);
        m_tracks.push_back(theader);
    }

    // Iterate Sequences

    cv::FileNode nodeSequences = node["Sequences"];
    if ( nodeSequences.type() != cv::FileNode::SEQ )
        throw Exception("\'Tracks\' is not an iterable type.");

    for (cv::FileNodeIterator it = nodeSequences.begin(); it != nodeSequences.end(); ++it){
        const cv::FileNode& sequenceNode = *it;

        cv::FileNode sequenceNodeTracks = sequenceNode["Tracks"];
        if ( sequenceNodeTracks.type() != cv::FileNode::SEQ )
            throw Exception("\'Sequence.Tracks\' is not an iterable type.");

        Sequence* seq = new Sequence(
            static_cast<std::string>(sequenceNode["Path"]),
            static_cast<std::string>(sequenceNode["Decoder"]),
            Sequence::typeFromString(static_cast<std::string>(sequenceNode["Type"])),
            static_cast<VideoTime>(static_cast<double>(sequenceNode["Length"]))
        );

        for ( cv::FileNodeIterator ntrackIt = sequenceNodeTracks.begin();
              ntrackIt != sequenceNodeTracks.end();
              ++ntrackIt
        ){
            size_t trackIndex = static_cast<size_t>(static_cast<double>((*ntrackIt)["Header"]));
            if ( trackIndex >= m_tracks.size() ){
                std::stringstream errorStream;
                errorStream << "Out of bounds header index: " << trackIndex;
                throw Exception(errorStream.str());
            }
            Track* track = seq->appendTrack(m_tracks[trackIndex]);
            track->read(*ntrackIt);
        }

        m_sequences.push_back(seq);
    }
}

inline void DataFile::write(cv::FileStorage &fs) const{
    fs << "TeGround" << "{";

    fs << "Tracks" << "[";
    for ( TrackHeaderConstIterator it = tracksBegin(); it != tracksEnd(); ++it ){
        fs << *(*it);
    }
    fs << "]";

    fs << "Sequences" << "[";
    for ( SequenceConstIterator it = sequencesBegin(); it != sequencesEnd(); ++it ){
        const Sequence* seq = *it;

        fs << "{";

        fs << "Path" << seq->path();
        fs << "Type" << Sequence::typeToString(seq->type());
        fs << "Length" << (double)seq->length();
        fs << "Decoder" << seq->decoder();

        fs << "Tracks" << "[";
        for ( Sequence::TrackConstIterator trackIt = seq->tracksBegin(); trackIt != seq->tracksEnd(); ++trackIt ){
            size_t index = trackIndex((*trackIt)->header());
            if ( index == m_tracks.size() )
                throw Exception("Failed to find index for track: " + (*trackIt)->header()->name());
            (*trackIt)->write(fs, index);
        }
        fs << "]";

        fs << "}";
    }
    fs << "]";

    fs << "}";
}


inline size_t DataFile::trackCount() const{
    return m_tracks.size();
}

inline size_t DataFile::trackIndex(const TrackHeader *track) const{
    for ( size_t i = 0; i < m_tracks.size(); ++i ){
        if ( m_tracks[i] == track )
            return i;
    }
    return m_tracks.size();
}

inline TrackHeader *DataFile::trackAt(size_t index){
    return m_tracks.at(index);
}

inline const TrackHeader *DataFile::trackAt(size_t index) const{
    return m_tracks.at(index);
}

inline TrackHeader* DataFile::appendTrack(const std::string &type, const std::string &name){
    if ( !TrackHeader::hasType(type) )
        throw Exception("Type \'" + type + "\' does not exist. Failed to create track \'" + name + "\'");

    TrackHeader* theader = new TrackHeader(type, name);
    m_tracks.push_back(theader);

    for ( SequenceIterator it = sequencesBegin(); it != sequencesEnd(); ++it ){
        Sequence* seq = *it;
        seq->appendTrack(theader);
    }

    return theader;
}

inline void DataFile::removeTrack(TrackHeader *trackHeader){

    for ( SequenceIterator it = sequencesBegin(); it != sequencesEnd(); ++it ){
        Sequence* seq = *it;
        seq->removeTrack(trackHeader);
    }

    for ( TrackHeaderIterator it = tracksBegin(); it != tracksEnd(); ++it ){
        if ( *it == trackHeader ){
            m_tracks.erase(it);
            break;
        }
    }

    delete trackHeader;
}

inline void DataFile::clearTracks(){
    for ( SequenceIterator it = sequencesBegin(); it != sequencesEnd(); ++it ){
        Sequence* seq = *it;
        seq->clearTracks();
    }

    for ( TrackHeaderIterator it = tracksBegin(); it != tracksEnd(); ++it ){
        delete *it;
    }
    m_tracks.clear();
}

inline DataFile::TrackHeaderIterator DataFile::tracksBegin(){
    return m_tracks.begin();
}

inline DataFile::TrackHeaderConstIterator DataFile::tracksBegin() const{
    return m_tracks.begin();
}

inline DataFile::TrackHeaderIterator DataFile::tracksEnd(){
    return m_tracks.end();
}

inline DataFile::TrackHeaderConstIterator DataFile::tracksEnd() const{
    return m_tracks.end();
}

inline size_t DataFile::sequenceCount() const{
    return m_sequences.size();
}

inline void DataFile::appendSequence(Sequence *seq){
    seq->clearTracks();
    m_sequences.push_back(seq);

    for ( TrackHeaderIterator it = tracksBegin(); it != tracksEnd(); ++it ){
        seq->appendTrack(*it);
    }
}

inline void DataFile::removeSequence(Sequence *seq){
    for ( SequenceIterator it = sequencesBegin(); it != sequencesEnd(); ++it ){
        if ( *it == seq ){
            m_sequences.erase(it);
            delete seq;
            return;
        }
    }
}

inline Sequence *DataFile::sequenceAt(size_t index){
    return m_sequences.at(index);
}

inline const Sequence *DataFile::sequenceAt(size_t index) const{
    return m_sequences.at(index);
}

inline Sequence *DataFile::takeSequence(Sequence *seq){
    for ( SequenceIterator it = sequencesBegin(); it != sequencesEnd(); ++it ){
        if ( *it == seq ){
            m_sequences.erase(it);
            return seq;
        }
    }
    return seq;
}

inline void DataFile::moveSequence(Sequence *seq, size_t indexTo){
    for ( size_t i = 0; i < sequenceCount(); ++i ){
        if ( seq == m_sequences[i] ){
            if ( i == indexTo )
                return;
            m_sequences.erase(sequencesBegin() + i);
            m_sequences.insert(sequencesBegin() + indexTo, seq);
            return;
        }
    }
}

inline Sequence *DataFile::sequenceFrom(const std::string &path){
    for ( SequenceConstIterator it = sequencesBegin(); it != sequencesEnd(); ++it ){
        if ( (*it)->path() == path )
            return *it;
    }
    return 0;
}

inline void DataFile::clearSequences(){
    for ( DataFile::SequenceIterator it = sequencesBegin(); it != sequencesEnd(); ++it )
        delete *it;
    m_sequences.clear();
}

inline DataFile::SequenceIterator DataFile::sequencesBegin(){
    return m_sequences.begin();
}

inline DataFile::SequenceConstIterator DataFile::sequencesBegin() const{
    return m_sequences.begin();
}

inline DataFile::SequenceIterator DataFile::sequencesEnd(){
    return m_sequences.end();
}

inline DataFile::SequenceConstIterator DataFile::sequencesEnd() const{
    return m_sequences.end();
}

}// namespace tg

#endif
