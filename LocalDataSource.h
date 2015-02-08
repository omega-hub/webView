#ifndef __LOCAL_DATA_SOURCE__
#define __LOCAL_DATA_SOURCE__

#include <Awesomium/DataSource.h>
#include <Awesomium/WebString.h>

///////////////////////////////////////////////////////////////////////////////
class LocalDataSource: public Awesomium::DataSource
{
public:
    virtual void OnRequest(int request_id, const Awesomium::WebString &path);
};
#endif