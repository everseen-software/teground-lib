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

#ifndef TGTRACKHEADER_H
#define TGTRACKHEADER_H

#include "tgglobal.h"

namespace tg{

class Track;
class TrackHeader{

    // Factory Functions
    // ------------------

public:
    typedef Track* (*TrackMakeFunction)(TrackHeader*, VideoTime);

private:
    class TrackFactory{
    public:
        TrackFactory(const std::string pType, TrackMakeFunction pMakeFunction)
            : type(pType)
            , makeFunction(pMakeFunction)
        {}

        std::string       type;
        TrackMakeFunction makeFunction;

        template<class T> static Track* make(TrackHeader* header, VideoTime length){
            return new T(header, length);
        }
    };

    static std::vector<TrackFactory>& registeredTypes() {
        static std::vector<TrackFactory> types;
        return types;
    }

    // TrackHeader Implementation
    // --------------------------

public:
    TrackHeader(const std::string& type, const std::string& name = "Track");
    ~TrackHeader();

    void setName(const std::string &name);
    const std::string& name() const;
    const std::string& type() const;

    Track* make(VideoTime length);

    static bool hasType(const std::string& type);

    template<class T> static void registerType(const std::string& typeName){
        if ( !hasType(typeName) )
            registeredTypes().push_back(TrackFactory(typeName, &TrackFactory::make<T>));
    }


    void write(cv::FileStorage& fs) const;
    void read(const cv::FileNode& node);

private:
    TrackHeader(const TrackHeader& other);
    TrackHeader& operator = (const TrackHeader& other);

    std::string m_name;
    std::string m_type;
    TrackMakeFunction m_fp;
};

inline bool TrackHeader::hasType(const std::string &type){
    for ( std::vector<TrackFactory>::iterator it = registeredTypes().begin(); it != registeredTypes().end(); ++it )
        if ( it->type == type)
            return true;
    return false;
}

inline void TrackHeader::write(cv::FileStorage &fs) const{
    fs << "{";
    fs << "Name" << m_name;
    fs << "Type" << m_type;
    fs << "}";
}

inline void TrackHeader::read(const cv::FileNode &node){
    m_name = (std::string)(node["Name"]);
}

inline TrackHeader::TrackHeader(const std::string &type, const std::string &name)
    : m_name(name)
    , m_type(type)
    , m_fp(0)
{
    for ( std::vector<TrackFactory>::iterator it = registeredTypes().begin(); it != registeredTypes().end(); ++it ){
        if ( it->type == type){
            m_fp = it->makeFunction;
            break;
        }
    }
}

inline TrackHeader::~TrackHeader(){
}

inline void TrackHeader::setName(const std::string& name){
    m_name = name;
}

inline const std::string&  TrackHeader::name() const{
    return m_name;
}

inline const std::string &TrackHeader::type() const{
    return m_type;
}

inline Track *TrackHeader::make(VideoTime length){
    if ( !m_fp )
        return 0;
    return m_fp(this, length);
}


static inline void write(cv::FileStorage& fs, const std::string&, const TrackHeader& x){
    x.write(fs);
}

static inline void read(const cv::FileNode& node, TrackHeader& x, const TrackHeader&){
    if(!node.empty())
        x.read(node);
}

} // namespace


#endif // TGTRACKHEADER_H
