#ifndef STEGMODEL_H
#define STEGMODEL_H

#include <QObject>
#include <sstream>

#include "opencv2/opencv.hpp"
#include "steg_data.h"


#define FIRSTBIT        0b00000001
#define SECONDBIT       0b00000010
#define THIRDBIT        0b00000100
#define FOURTHBIT       0b00001000
#define FIFTHBIT        0b00010000
#define SIXTHBIT        0b00100000
#define SEVENTHBIT      0b01000000
#define EIGHTHBIT       0b10000000

#define CHANNEL_R       0b100
#define CHANNEL_G       0b010
#define CHANNEL_B       0b001


enum class DataType     { CONTAINER, MESSAGE };
enum class ActionType   { ENCODE, DECODE };
enum class FileType     { IMAGE, VIDEO, NONE };

class StegModel : public QObject {

    Q_OBJECT

    public:
        StegModel()  = default;
        ~StegModel();

        bool encode(const std::string out_path);
        bool decode(const std::string out_path);
        void choose_container(const std::string path);
        void choose_data(const std::string path);

        void set_bits_value(const uint8_t bit_ind);
        void set_channel_value(const std::string ch);
        void set_action(const ActionType at);
        void set_columns_gaps(const uint16_t val);
        void set_rows_gaps(const uint16_t val);

        inline uint8_t get_bits()                   const { return bits; }
        inline uint8_t get_bits_cnt()               const { return bits_cnt; }
        inline uint8_t get_channels()               const { return channels; }
        inline uint8_t get_channels_cnt()           const { return channels_cnt; }
        inline uint16_t get_col_gaps()              const { return col_gaps; }
        inline uint16_t get_row_gaps()              const { return row_gaps; }
        inline std::string get_path()               const { return path; }
        inline FileType get_file_type()             const { return file_type; }
        inline cv::Size get_frame_size()            const { return frame_size; }
        inline uint16_t get_fps()                   const { return fps; }
        inline uint32_t get_frames_cnt()            const { return frames_count; }
        inline std::string get_data_path()          const { return data_path; }
        inline uint64_t get_data_size()             const { return data_size; }
        inline uint64_t get_container_capacity()    const { return container_capacity(); }


    private:
        ActionType action_type = ActionType::ENCODE;

        uint8_t bits                = 0b00000001;
        uint8_t bits_cnt            = 1;
        uint8_t channels            = 0b111;
        uint8_t channels_cnt        = 3;
        uint16_t col_gaps           = 0;
        uint16_t row_gaps           = 0;

        std::string path            = "";
        FileType file_type          = FileType::NONE;

        cv::Mat* img                = nullptr;

        cv::VideoCapture* vid       = nullptr;
        cv::Size frame_size         = { 0, 0 };
        uint16_t fps                = 0;
        uint32_t frames_count       = 0;

        std::string data_path       = "";
        uint64_t data_size          = 0;


        void display_data();

        void insert_data(cv::Mat* img_steg, const char* buffer, const uint64_t& buf_len, uint64_t& buf_ind, const uint16_t col, const uint16_t row, const uint8_t cnl, uint8_t& rest, bool& end);
        uint64_t set_buffer(char*&);

        void extract(char* buffer, const uint64_t& buf_len, uint32_t frame_ind, uint16_t col, uint16_t row, uint8_t cnl, int rest);
        void extract_data(char* buffer, const uint64_t& buf_len, uint64_t& buf_ind, const uint16_t col, const uint16_t row, const uint8_t cnl, int& rest, bool& end);

        uint64_t container_capacity() const;
        uint8_t get_channel(uint8_t cnl) const;
        uint32_t get_frames_count();
        void save_data(const StegData& sdata, const std::string& out_path) const;

    signals:
        void show_size(DataType dt, uint64_t size);
        void enable_button(ActionType at, bool enable);
        void set_progBar(uint64_t);
        void increment_progBar();
        void show_message(std::string str);
        void set_filter(std::string);
};


#endif // STEGMODEL_H
