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

#ifndef TGSEQUENCE_H
#define TGSEQUENCE_H

#include "tgglobal.h"
#include "tgtrackheader.h"
#include "tgtrack.h"

namespace tg {

class Sequence{

public:
    enum Type{
        Video,
        Image
    };

    typedef std::vector<Track*>::iterator       TrackIterator;
    typedef std::vector<Track*>::const_iterator TrackConstIterator;

public:
    Sequence()
        : m_path("")
        , m_decoder("")
        , m_type(Video)
        , m_length(0)
    {}
    Sequence(const std::string& path, const std::string& decoder, Type type, VideoTime length)
        : m_path(path)
        , m_decoder(decoder)
        , m_type(type)
        , m_length(length)
    {}
    ~Sequence()
    {}

    const std::string path() const;
    VideoTime length() const;
    size_t totalTracks() const;

    Type type() const;
    static std::string typeToString(Sequence::Type type);
    static Sequence::Type typeFromString(const std::string& str);

    const std::string& decoder() const;
    void setDecoder(const std::string& decoder);

    Track* track(const std::string& name);
    Track* track(const TrackHeader* theader);

    const Track* track(const std::string& name) const;
    const Track* track(const TrackHeader* theader) const;

    Track* appendTrack(TrackHeader* header);
    void removeTrack(TrackHeader* header);
    void clearTracks();

    TrackIterator tracksBegin();
    TrackConstIterator tracksBegin() const;
    TrackIterator tracksEnd();
    TrackConstIterator tracksEnd() const;

private:
    // prevent copy
    Sequence(const Sequence&);
    Sequence& operator = (const Sequence&);

    // fields
    std::string m_path;
    std::string m_decoder;
    Type        m_type;
    VideoTime   m_length;

    std::vector<Track*> m_tracks;

};

inline const std::string Sequence::path() const{
    return m_path;
}

inline Sequence::Type Sequence::type() const{
    return m_type;
}

inline std::string Sequence::typeToString(Type type){
    switch(type){
    case Sequence::Video:
        return "Video";
    case Sequence::Image:
        return "Image";
    }
    return "";
}

inline Sequence::Type Sequence::typeFromString(const std::string &str){
    if ( str == "Video" )
        return Sequence::Video;
    else if ( str == "Image" )
        return Sequence::Image;
    return Sequence::Video;
}

inline VideoTime Sequence::length() const{
    return m_length;
}

inline size_t Sequence::totalTracks() const{
    return m_tracks.size();
}

inline const std::string &Sequence::decoder() const{
    return m_decoder;
}

inline void Sequence::setDecoder(const std::string &decoder){
    if ( m_decoder != decoder )
        m_decoder = decoder;
}

inline Track *Sequence::track(const std::string &name){
    for ( TrackIterator it = tracksBegin(); it != tracksEnd(); ++it )
        if ( (*it)->header()->name() == name )
            return *it;
    return 0;
}

inline Track *Sequence::track(const TrackHeader *theader){
    for ( TrackIterator it = tracksBegin(); it != tracksEnd(); ++it )
        if ( (*it)->header() == theader )
            return *it;
    return 0;
}

inline const Track *Sequence::track(const std::string &name) const{
    for ( TrackConstIterator it = tracksBegin(); it != tracksEnd(); ++it )
        if ( (*it)->header()->name() == name )
            return *it;
    return 0;
}

inline const Track *Sequence::track(const TrackHeader *theader) const{
    for ( TrackConstIterator it = tracksBegin(); it != tracksEnd(); ++it )
        if ( (*it)->header() == theader )
            return *it;
    return 0;
}

inline Track* Sequence::appendTrack(TrackHeader *header){
    Track* track = header->make(m_length);
    m_tracks.push_back(track);
    return track;
}

inline void Sequence::removeTrack(TrackHeader *header){
    for ( TrackIterator it = tracksBegin(); it != tracksEnd(); ++it ){
        if ( (*it)->header() == header){
            delete *it;
            m_tracks.erase(it);
            return;
        }
    }
}

inline void Sequence::clearTracks(){
    for ( TrackIterator it = tracksBegin(); it != tracksEnd(); ++it )
        delete *it;
    m_tracks.clear();
}

inline Sequence::TrackIterator Sequence::tracksBegin(){
    return m_tracks.begin();
}

inline Sequence::TrackConstIterator Sequence::tracksBegin() const{
    return m_tracks.begin();
}

inline Sequence::TrackIterator Sequence::tracksEnd(){
    return m_tracks.end();
}

inline Sequence::TrackConstIterator Sequence::tracksEnd() const{
    return m_tracks.end();
}

} // namespace

#endif // TGSEQUENCE_H
