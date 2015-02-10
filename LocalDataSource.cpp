#include <omega.h>
#include "LocalDataSource.h"
#include <Awesomium/STLHelpers.h>

#include <fstream>

using namespace omega;

// Needed to fix build on linux:
// http://answers.awesomium.com/questions/4205/linux-undefined-reference-to-typeinfo-for-awesomiu.html
#ifdef __linux__
    Awesomium::DataSource::~DataSource()
    {
    }
#endif

///////////////////////////////////////////////////////////////////////////////
void LocalDataSource::OnRequest(int request_id, const Awesomium::ResourceRequest& request, const Awesomium::WebString &url)
{
    DataManager* dm = SystemManager::instance()->getDataManager();
    
    String path = ToString(url);

    // Find extension to deduct mime type
    String filename;
    String ext;
    String mime;
    StringUtils::splitBaseFilename(path, filename, ext);

    if(ext == "html" || ext == "htm") mime = "text/html";
    else if(ext == "css") mime = "text/css";
    else if(ext == "jpeg" || ext == "jpg") mime = "image/jpeg";
    else if(ext == "gif") mime = "image/gif";
    else if(ext == "png") mime = "image/png";
    else if(ext == "js") mime = "application/javascript";

    String fullpath;
    if(DataManager::findFile(path, fullpath))
    {
        std::ifstream file(fullpath.c_str(), std::ios::binary);
        file.seekg(0, std::ios::end);
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<char> buffer(size);
        if(file.read(buffer.data(), size))
        {
            SendResponse(request_id, size, (unsigned char*)buffer.data(), Awesomium::WSLit(mime.c_str()));
        }
    }
    
}
