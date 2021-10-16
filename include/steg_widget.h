#ifndef STEG_WIDGET_H
#define STEG_WIDGET_H

#include <QMainWindow>

#include "steg_model.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Steg_Widget; }
QT_END_NAMESPACE

class Steg_Widget : public QMainWindow
{
    Q_OBJECT

    public:
        Steg_Widget(QWidget *parent = nullptr);
        ~Steg_Widget();

    private:
        Ui::Steg_Widget *ui;

        StegModel smodel;
        QString ext_filter;

        void set_up_widget();
        void clear_fields();

    private slots:
        void model_show_size(DataType dt, uint64_t size);
        void model_enable_button(ActionType at, bool enable);
        void model_set_progBar(uint64_t val);
        void model_increment_progBar();
        void model_show_message(std::string str);
        void model_set_filter(std::string str);

        void on_pushButton_imagePath_clicked();
        void on_pushButton_dataPath_clicked();
        void on_pushButton_encode_clicked();
        void on_pushButton_decode_clicked();

        void on_tabWidget_currentChanged(int index);
        void checkBox_bits_stateChanged(int arg);
        void checkBox_channels_stateChanged(std::string arg);
        void on_spinBox_col_gaps_valueChanged(int arg1);
        void on_spinBox_row_gaps_valueChanged(int arg1);
        void on_pushButton_imagePath_2_clicked();
};
#endif // STEG_WIDGET_H
