#ifndef __SXGE_UTIL_FILE_LOADER_HH__
#define __SXGE_UTIL_FILE_LOADER_HH__

#include <iostream>
#include <fstream>
#include <string>

namespace sxge {

class FileLoader {
public:
	FileLoader(std::string filename) : data(NULL) {
		size_t size = 0;
		std::ifstream fin(filename, std::ios::in);

		fin.seekg(0, std::ios::end);
		size = fin.tellg();
		fin.seekg(0, std::ios::beg);

		data = new char[size];	
		fin.read(data, size);
		fin.close();
	}
	
	virtual ~FileLoader() {
		delete[] data;
	}
	
	char* getData() {
		return data;
	}

protected:
	char *data;
};

} //namespace sxge

#endif //__SXGE_UTIL_FILE_LOADER_HH__
