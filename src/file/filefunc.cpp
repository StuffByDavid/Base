#include "file/filefunc.hpp"


bool Base::fileExists(string filename)
{
	return boost::filesystem::exists(filename);
}

int Base::fileGetSize(string filename)
{
    return boost::filesystem::file_size(filename);
}

string Base::fileGetName(string filename)
{
    size_t pos = filename.find_last_of(SLASH);

    if (pos == string::npos)
        return filename;

    return filename.substr(pos + 1, filename.size() - pos - 1);
}

string Base::fileGetPath(string filename)
{
    size_t pos = filename.find_last_of(SLASH);

    if (pos == string::npos)
        return filename;

    return filename.substr(0, pos + 1);
}

string Base::fileGetDirectory(string filename)
{
    size_t pos = filename.find_last_of(SLASH);

    if (pos == string::npos)
        return filename;

    return filename.substr(0, pos);
}

string Base::fileGetExtension(string filename)
{
    string fn = fileGetName(filename);
    size_t pos = fn.find_last_of(DOT);

    if (pos == string::npos)
        return "";

    return fn.substr(pos, fn.size() - pos);
}

string Base::fileSetExtension(string filename, string ext)
{
    string fn = fileGetName(filename);
    string fp = fileGetPath(filename);
    size_t pos = fn.find_last_of(DOT);

    if (pos != string::npos)
        fn = fn.substr(0, pos);
    
    return fp + fn + ext;
}

string Base::fileGetContents(string filename)
{
    std::ifstream file(filename);
    string line, contents = "";
    
    while (getline(file, line))
        contents += line + '\n';
    
    file.close();
    
    return contents;
}

bool Base::directoryExists(string directory)
{
    struct stat info;
    
    if (stat(&directory[0], &info) != 0)
        return false;

    return (info.st_mode & S_IFDIR);
}