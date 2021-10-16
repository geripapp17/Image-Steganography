#include "../include/steg_data.h"

#include <bitset>

StegData::StegData(const std::string path) {

    if("" != path) {

        std::ifstream fin(path, std::ios::binary);
        if (!fin.good()) {
            //std::cout << "Could not read data" << std::endl;
            return;
        }

        ext = new char[EXTENSION_SIZE];
        memset(ext, 0, EXTENSION_SIZE);
        strcpy_s(ext, EXTENSION_SIZE, getFileExtension(path).c_str());

        fin.seekg(0, std::ios::end);
        data_len = fin.tellg();

        data = new char[data_len + 1];
        data[data_len] = '\0';

        fin.seekg(0, std::ios_base::beg);
        fin.read(data, data_len);
        fin.close();
    }
    else {
        ext = nullptr;
        data = nullptr;
        data_len = 0;
    }
}

StegData::~StegData() {

    if(ext) {
        ext = nullptr;
    }
    if(data) {
        data = nullptr;
    }
}

std::string getFileExtension(const std::string path) {

    size_t i = path.rfind('.', path.length());
    if (i != std::string::npos) {
        return(path.substr(i + 1, path.length() - i));
    }

    return("");
}

