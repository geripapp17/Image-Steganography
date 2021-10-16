#include "../include/steg_model.h"

#include <bitset>

StegModel::~StegModel() {

    if(FileType::IMAGE == file_type) {
        delete img;
    }
    else if(FileType::VIDEO == file_type) {
        delete vid;
    }
}

bool StegModel::encode(const std::string out_path) {

    if(data_size <= container_capacity()) {

        set_progBar(data_size);

        char* buffer;
        const uint64_t buf_len = set_buffer(buffer);

        uint8_t rest = 8;
        uint64_t buf_ind = 0;
        bool end = false;

        cv::Mat* img_steg;
        cv::VideoWriter* vid_out;
        uint32_t frame_ind = 0;

        if(FileType::VIDEO == file_type) {
            try {
                vid_out = new cv::VideoWriter(out_path, cv::VideoWriter::fourcc('F', 'F', 'V', '1'), fps, frame_size, true);
            }  catch (...) {
                show_message("ERROR - Problem with FFMPEG Codec (FOURCC - FFV1)");
                return false;
            };

            img_steg = new cv::Mat(frame_size, CV_8UC3);
        }
        else {
            img_steg = new cv::Mat(img->clone());
        }


        while(frame_ind < frames_count) {

            if(FileType::VIDEO == file_type) {
                *vid >> *img_steg;
                if((*img_steg).empty()) { break; }
            }

            uint16_t row = 0;
            while(!end && row < img_steg->rows) {
                uint16_t col = 0;
                while(!end && col < img_steg->cols) {
                    uint8_t cnl = CHANNEL_B;
                    while(cnl <= CHANNEL_R) {

                        if(channels & cnl) {
                            insert_data(img_steg, buffer, buf_len, buf_ind, col, row, cnl, rest, end);
                        }

                        cnl = cnl << 1;
                        cnl &= 0b11111110;
                    }

                    col += (col_gaps + 1);
                }

                row += (row_gaps + 1);
            }
            ++frame_ind;

            if(FileType::VIDEO == file_type) {
                vid_out->write(*img_steg);
            }

            increment_progBar();
        }

        if(FileType::VIDEO == file_type) {
            vid->set(cv::CAP_PROP_POS_FRAMES, 0);
            delete vid_out;
        }
        else {
            cv::imwrite(out_path, *img_steg);
        }

        delete img_steg;
        delete buffer;

        show_message("Successfully encoded.");

        return true;
    }
    else {
        show_message("Not enough space in container.");
        return false;
    }

}

uint64_t StegModel::set_buffer(char*& buffer) {

    StegData sdata(data_path);
    uint64_t data_len = sdata.get_data_length();

    uint64_t buf_size = strlen(STEGNAME) + EXTENSION_SIZE + sizeof(uint64_t) + data_len + 1;
    buffer = new char[buf_size];
    buffer[buf_size - 1] = '\0';

    memcpy(buffer, STEGNAME, strlen(STEGNAME));
    memcpy(buffer + strlen(STEGNAME), sdata.get_extension(), EXTENSION_SIZE);
    memcpy(buffer + strlen(STEGNAME) + EXTENSION_SIZE, &data_len, sizeof(uint64_t));
    memcpy(buffer + strlen(STEGNAME) + EXTENSION_SIZE + sizeof(uint64_t), sdata.get_data(), data_len);

    return buf_size;
}

void StegModel::insert_data(cv::Mat* img_steg, const char* buffer, const uint64_t& buf_len, uint64_t& buf_ind, const uint16_t col, const uint16_t row, const uint8_t cnl, uint8_t& rest, bool& end) {

    img_steg->at<cv::Vec3b>(row, col)[cnl / 2] &= (~bits);

    uint8_t b = EIGHTHBIT;
    for(int i = 0; i < 8; ++i) {

        if(bits & b) {
            uint8_t val = buffer[buf_ind];
            val = (val << (8 - rest)) >> i;
            val &= b;

            img_steg->at<cv::Vec3b>(row, col)[cnl / 2] |= val;

            --rest;
            if(0 == rest) {
                ++buf_ind;
                rest = 8;

                increment_progBar();
                if(buf_ind >= buf_len - 1) { end = true; break; }
            }
        }

        b = b >> 1;
        b &= 0b01111111;
    }
}

bool StegModel::decode(const std::string out_path) {

    StegData sdata;

    if(FileType::IMAGE == file_type) {
        frames_count = 1;
    }
    else {
        img = new cv::Mat(frame_size, CV_8UC3);
    }

    // Extract STEGNAME
    char* buffer = new char[strlen(STEGNAME) + 1];
    buffer[strlen(STEGNAME)] = '\0';
    extract(buffer, strlen(STEGNAME), 0, 0, 0, CHANNEL_B, 0);

    if(strcmp(buffer, STEGNAME)) {
        show_message("Problem occured while decoding.");

        if(FileType::VIDEO == file_type) {
            delete img;
        }

        delete[] buffer;
        return false;
    }
    delete[] buffer;

    // Extract extension
    uint16_t col = (strlen(STEGNAME) * 8 / bits_cnt / channels_cnt);
    col += (col * col_gaps);
    col %= img->cols;
    uint16_t row = (strlen(STEGNAME) * 8 / bits_cnt / channels_cnt);
    row += (row * row_gaps);
    row /= img->cols;

    uint32_t frame_ind = 0;
    if(FileType::VIDEO == file_type) {
        frame_ind = row / frame_size.height;
    }

    uint8_t cnl = get_channel(((strlen(STEGNAME) * 8 / bits_cnt)) % channels_cnt);
    int rest = 0 - ((strlen(STEGNAME) * 8) % bits_cnt);

    buffer = new char[EXTENSION_SIZE];
    extract(buffer, EXTENSION_SIZE, frame_ind, col, row, cnl, rest);
    sdata.set_extension(buffer);


    // Extract message length
    col = ((strlen(STEGNAME) + EXTENSION_SIZE) * 8 / bits_cnt / channels_cnt);
    col += (col * col_gaps);
    col %= img->cols;
    row = ((strlen(STEGNAME) + EXTENSION_SIZE) * 8 / bits_cnt / channels_cnt);
    row += (row * row_gaps);
    row /= img->cols;

    frame_ind = 0;
    if(FileType::VIDEO == file_type) {
        frame_ind = row / frame_size.height;
    }

    cnl = get_channel((((strlen(STEGNAME) + EXTENSION_SIZE) * 8 / bits_cnt)) % channels_cnt);
    rest = 0 - (((strlen(STEGNAME) + EXTENSION_SIZE) * 8) % bits_cnt);

    buffer = new char[sizeof(uint64_t)];
    extract(buffer, sizeof(uint64_t), frame_ind, col, row, cnl, rest);
    uint64_t len;
    memcpy(&len, buffer, sizeof(uint64_t));
    sdata.set_length(len);

    set_progBar(len);

    // Extract message
    buffer = new char[len];
    col = ((strlen(STEGNAME) + EXTENSION_SIZE + sizeof(uint64_t)) * 8 / bits_cnt / channels_cnt);
    col += (col * col_gaps);
    col %= img->cols;
    row = ((strlen(STEGNAME) + EXTENSION_SIZE + sizeof(uint64_t)) * 8 / bits_cnt / channels_cnt);
    row += (row * row_gaps);
    row /= img->cols;

    frame_ind = 0;
    if(FileType::VIDEO == file_type) {
        frame_ind = row / frame_size.height;
    }

    cnl = get_channel(((strlen(STEGNAME) + EXTENSION_SIZE + sizeof(uint64_t)) * 8 / bits_cnt) % channels_cnt);
    rest = 0 - (((strlen(STEGNAME) + EXTENSION_SIZE + sizeof(uint64_t)) * 8) % bits_cnt);

    extract(buffer, len, frame_ind, col, row, cnl, rest);
    sdata.set_data(buffer);


    // Save extracted message
    save_data(sdata, out_path);

    if(FileType::VIDEO == file_type) {
        delete img;
    }

    show_message("Successfully decoded.");

    return true;
}

void StegModel::extract(char* buffer, const uint64_t& buf_len, uint32_t frame_ind, uint16_t col, uint16_t row, uint8_t cnl, int rest) {

    uint64_t buf_ind = 0;
    bool end = false;

    uint32_t f_ind = frame_ind;
    uint32_t ind = 0;
    if(0 != frame_ind) {
        --f_ind;
    }
    while(ind < f_ind) {
        *vid >> *img;
        ++ind;
    }

    while(!end && frame_ind < frames_count) {

        if(FileType::VIDEO == file_type) {

            *vid >> *img;
            if((*img).empty()) {
                break;
            }
        }

        row = 0;
        while(!end && row < img->rows) {
            col = (col >= img->cols ? 0 : col);
            while(!end && col < img->cols) {
                while(!end && cnl <= CHANNEL_R) {

                    if(channels & cnl) {
                        extract_data(buffer, buf_len, buf_ind, col, row, cnl, rest, end);
                    }

                    cnl = cnl << 1;
                    cnl &= 0b11111110;
                }

                cnl = CHANNEL_B;
                col += (col_gaps + 1);
            }

            row += (row_gaps + 1);
        }

        ++frame_ind;
    }

    if(FileType::VIDEO == file_type) {
        vid->set(cv::CAP_PROP_POS_FRAMES, 0);
    }
}

void StegModel::extract_data(char* buffer, const uint64_t& buf_len, uint64_t& buf_ind, const uint16_t col, const uint16_t row, const uint8_t cnl, int& rest, bool& end) {

    uint8_t channel_data = img->at<cv::Vec3b>(row, col)[cnl / 2];

    uint8_t b = EIGHTHBIT;
    for(int i = 0; i < 8; ++i) {

        if(bits & b) {
            uint8_t val = channel_data;
            val = (val >> (8 - i - 1));
            val &= 0b00000001;

            buffer[buf_ind] = buffer[buf_ind] << 1;
            buffer[buf_ind] |= val;

            ++rest;
            if(8 == rest) {
                increment_progBar();

                ++buf_ind;
                rest = 0;

                if(buf_ind >= buf_len) { end = true; break; }
            }
        }


        b = b >> 1;
        b &= 0b01111111;
    }
}

uint8_t StegModel::get_channel(uint8_t cnl) const {

    if(1 == channels_cnt) {
        if(channels & CHANNEL_B) {
            return CHANNEL_B;
        }
        else if(channels & CHANNEL_G) {
            return CHANNEL_G;
        }
        else if(channels & CHANNEL_R) {
            return CHANNEL_R;
        }
    }
    else if(2 == channels_cnt) {
        if(0 == cnl) {
            if(channels & CHANNEL_B) {
                return CHANNEL_B;
            }
            else if(channels & CHANNEL_G) {
                return CHANNEL_G;
            }
            else if(channels & CHANNEL_R) {
                return CHANNEL_R;
            }
        }
        else if(1 == cnl) {
            if(channels & CHANNEL_B) {
                if(channels & CHANNEL_G) {
                    return CHANNEL_G;
                }
                else if(channels & CHANNEL_R) {
                    return CHANNEL_R;
                }
            }
            else if(channels & CHANNEL_G) {
                if(channels & CHANNEL_R) {
                    return CHANNEL_R;
                }
            }
        }
    }
    else {
        if(0 == cnl) {
            return CHANNEL_B;
        }
        else if(1 == cnl) {
            return CHANNEL_G;
        }
        else {
            return CHANNEL_R;
        }
    }
}



void StegModel::choose_container(const std::string path_) {

    if(FileType::IMAGE == file_type) {
        delete img;
        file_type = FileType::NONE;
    }
    else if(FileType::VIDEO == file_type) {
        delete vid;
        file_type = FileType::NONE;
    }

    path = path_;

    if("" != path) {
        std::string ext = getFileExtension(path);

        if("mp4" == ext || "avi" == ext || "mov" == ext) {
            set_filter("AVI (*.avi)");
            file_type = FileType::VIDEO;
            vid = new cv::VideoCapture(path);
            frame_size = { (int)vid->get(cv::CAP_PROP_FRAME_WIDTH), (int)vid->get(cv::CAP_PROP_FRAME_HEIGHT) };
            fps = vid->get(cv::CAP_PROP_FPS);
            frames_count = get_frames_count();
        }
        else {
            set_filter("PNG (*.png)");
            file_type = FileType::IMAGE;
            img = new cv::Mat(cv::imread(path, cv::IMREAD_COLOR));
            frames_count = 1;
        }
    }
    else {
        file_type = FileType::NONE;
    }

    display_data();
}

void StegModel::choose_data(const std::string path) {

    data_path = path;
    data_size = 0;

    if("" != data_path) {

        std::ifstream fin(data_path, std::ios::binary);

        fin.seekg(0, std::ios::end);
        data_size = fin.tellg();

        fin.close();
    }

    display_data();
}

void StegModel::display_data() {

    if(ActionType::ENCODE == action_type) {

        uint64_t size = container_capacity();

        show_size(DataType::CONTAINER, size);
        show_size(DataType::MESSAGE, data_size);

        if("" != data_path && data_size && size >= data_size) {
            enable_button(ActionType::ENCODE, true);
        }
        else if("" != data_path && (!data_size || (size < data_size))) {
            show_message("Not enough space in container.");
            enable_button(ActionType::ENCODE, false);
        }
    }
    else if(ActionType::DECODE == action_type) {

        uint64_t size = container_capacity();

        if(FileType::NONE != file_type && 0 != size) {
            enable_button(ActionType::DECODE, true);
        }
        else {
            enable_button(ActionType::DECODE, false);
        }

    }

}



uint64_t StegModel::container_capacity() const {

    if(FileType::IMAGE == file_type && img) {

        uint8_t header_size = (strlen(STEGNAME) + EXTENSION_SIZE + sizeof(uint64_t));

        uint16_t r = std::ceil((double)img->rows / (row_gaps + 1));
        uint16_t c = std::ceil((double)img->cols / (col_gaps + 1));

        uint64_t capacity = r * c * channels_cnt * bits_cnt;

        return (0 == capacity) ? 0 : (capacity / 8 - header_size);
    }
    else if(FileType::VIDEO == file_type && vid) {

        uint8_t header_size = (strlen(STEGNAME) + EXTENSION_SIZE + sizeof(uint64_t));

        uint16_t c = std::ceil((double)frame_size.width / (col_gaps + 1));
        uint16_t r = std::ceil((double)frame_size.height / (row_gaps + 1));

        uint64_t capacity_frame = r * c * channels_cnt * bits_cnt;

        return (0 == capacity_frame) ? 0 : (frames_count * capacity_frame / 8 - header_size);
    }

    return 0;
}

uint32_t StegModel::get_frames_count() {

    uint32_t frames_count = 0;

    try {
        frames_count = vid->get(cv::CAP_PROP_FRAME_COUNT);
    }
    catch (...) {

        while(true) {
            cv::Mat frame;
            *vid >> frame;

            if(frame.empty()) {
                break;
            }

            ++frames_count;
        }
        vid->set(cv::CAP_PROP_POS_FRAMES, 0);
    }

    return frames_count;
}

void StegModel::save_data(const StegData& sdata, const std::string& out_path) const {

    std::stringstream ss;
    ss << out_path << "/out." << sdata.get_extension();

    std::ofstream fout(ss.str(), std::ios_base::binary);
    fout.write(sdata.get_data(), sdata.get_data_length());
    fout.close();
}



void StegModel::set_action(const ActionType at) {

    action_type = at;

    if(FileType::IMAGE == file_type) {
        delete img;
    }
    else if(FileType::VIDEO == file_type) {
        delete vid;
    }


    bits = 0b00000001;
    bits_cnt = 1;
    channels = 0b111;
    channels_cnt = 3;
    col_gaps = 0;
    row_gaps = 0;

    path = "";
    file_type = FileType::NONE;

    frame_size = { 0, 0 };
    fps = 0;
    frames_count = 0;

    data_path = "";
    data_size = 0;
}

void StegModel::set_bits_value(const uint8_t bit_ind) {

    switch(bit_ind) {
        case 1:
            if(bits & FIRSTBIT) {
                bits ^= FIRSTBIT;
                --bits_cnt;
            }
            else {
                bits |= FIRSTBIT;
                ++bits_cnt;
            }
        break;

        case 2:
            if(bits & SECONDBIT) {
                bits ^= SECONDBIT;
                --bits_cnt;
            }
            else {
                bits |= SECONDBIT;
                ++bits_cnt;
            }
        break;

        case 3:
            if(bits & THIRDBIT) {
                bits ^= THIRDBIT;
                --bits_cnt;
            }
            else {
                bits |= THIRDBIT;
                ++bits_cnt;
            }
        break;

        case 4:
            if(bits & FOURTHBIT) {
                bits ^= FOURTHBIT;
                --bits_cnt;
            }
            else {
                bits |= FOURTHBIT;
                ++bits_cnt;
            }
        break;

        case 5:
            if(bits & FIFTHBIT) {
                bits ^= FIFTHBIT;
                --bits_cnt;
            }
            else {
                bits |= FIFTHBIT;
                ++bits_cnt;
            }
        break;

        case 6:
            if(bits & SIXTHBIT) {
                bits ^= SIXTHBIT;
                --bits_cnt;
            }
            else {
                bits |= SIXTHBIT;
                ++bits_cnt;
            }
        break;

        case 7:
            if(bits & SEVENTHBIT) {
                bits ^= SEVENTHBIT;
                --bits_cnt;
            }
            else {
                bits |= SEVENTHBIT;
                ++bits_cnt;
            }
        break;

        case 8:
            if(bits & EIGHTHBIT) {
                bits ^= EIGHTHBIT;
                --bits_cnt;
            }
            else {
                bits |= EIGHTHBIT;
                ++bits_cnt;
            }
        break;
    }

    display_data();

}

void StegModel::set_channel_value(const std::string str) {

    switch(str[0]) {
        case 'R':
            if(channels & CHANNEL_R) {
                channels ^= CHANNEL_R;
                --channels_cnt;
            }
            else {
                channels |= CHANNEL_R;
                ++channels_cnt;
            }
        break;

        case 'G':
            if(channels & CHANNEL_G) {
                channels ^= CHANNEL_G;
                --channels_cnt;
            }
            else {
                channels |= CHANNEL_G;
                ++channels_cnt;
            }
        break;

        case 'B':
            if(channels & CHANNEL_B) {
                channels ^= CHANNEL_B;
                --channels_cnt;
            }
            else {
                channels |= CHANNEL_B;
                ++channels_cnt;
            }
        break;
    }

    display_data();

}

void StegModel::set_columns_gaps(const uint16_t val) {

    col_gaps = val;

    if(ActionType::ENCODE == action_type) {
        display_data();
    }
}

void StegModel::set_rows_gaps(const uint16_t val) {

    row_gaps = val;

    if(ActionType::ENCODE == action_type) {
        display_data();
    }
}

