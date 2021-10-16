    #ifndef STEGDATA_H
#define STEGDATA_H

#include <fstream>
#include <string.h>

#define STEGNAME        "G-Steg"
#define EXTENSION_SIZE  5


class StegData {
    private:
        char* data;
        char* ext;
        uint64_t data_len;

    public:
        StegData(const std::string path = "");
        ~StegData();


        inline const char* get_extension()   const { return ext; }
        inline const char* get_data()        const { return data; }
        inline uint64_t  get_data_length()   const { return data_len; }

        inline void set_data(char* d)              { data = d; }
        inline void set_extension(char* e)         { ext = e; }
        inline void set_length(const uint64_t len) { data_len = len; }
};

std::string getFileExtension(const std::string path);


#endif // STEGDATA_H
