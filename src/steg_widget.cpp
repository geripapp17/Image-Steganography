#include "include/steg_widget.h"
#include "ui_steg_widget.h"

#include <QFileDialog>
#include <QDir>



Steg_Widget::Steg_Widget(QWidget *parent) : QMainWindow(parent), ui(new Ui::Steg_Widget) {

    ui->setupUi(this);
    this->setFixedSize(this->width(),this->height());
    ui->tabWidget->setCurrentIndex(0);

    set_up_widget();

    connect(&smodel, SIGNAL(show_size(DataType, uint64_t)),
            this,   SLOT(model_show_size(DataType, uint64_t)));

    connect(&smodel, SIGNAL(enable_button(ActionType, bool)),
            this,   SLOT(model_enable_button(ActionType, bool)));

    connect(&smodel, SIGNAL(set_progBar(uint64_t)),
            this,   SLOT(model_set_progBar(uint64_t)));

    connect(&smodel, SIGNAL(increment_progBar()),
            this,   SLOT(model_increment_progBar()));

    connect(&smodel, SIGNAL(show_message(std::string)),
            this,   SLOT(model_show_message(std::string)));

    connect(&smodel, SIGNAL(set_filter(std::string)),
            this,   SLOT(model_set_filter(std::string)));
}

Steg_Widget::~Steg_Widget() {

    delete ui;
}

void Steg_Widget::set_up_widget() {

    for(int i = 1; i <= 8; ++i) {
        QCheckBox* check_box = Steg_Widget::findChild<QCheckBox*>("checkBox_bit_" + QString::number(i));
        int value = check_box->text().toInt();
        connect(check_box, &QCheckBox::stateChanged, [this, value] { checkBox_bits_stateChanged(value); });

        check_box = Steg_Widget::findChild<QCheckBox*>("checkBox_bit_" + QString::number(i) + "_2");
        connect(check_box, &QCheckBox::stateChanged, [this, value] { checkBox_bits_stateChanged(value); });
    }


    connect(ui->checkBox_channel_R, &QCheckBox::stateChanged, [this] { checkBox_channels_stateChanged("R"); });

    connect(ui->checkBox_channel_G, &QCheckBox::stateChanged, [this] { checkBox_channels_stateChanged("G"); });

    connect(ui->checkBox_channel_B, &QCheckBox::stateChanged, [this] { checkBox_channels_stateChanged("B"); });

    connect(ui->checkBox_channel_R_2, &QCheckBox::stateChanged, [this] { checkBox_channels_stateChanged("R"); });

    connect(ui->checkBox_channel_G_2, &QCheckBox::stateChanged, [this] { checkBox_channels_stateChanged("G"); });

    connect(ui->checkBox_channel_B_2, &QCheckBox::stateChanged, [this] { checkBox_channels_stateChanged("B"); });



    QSpinBox* spin_box = Steg_Widget::findChild<QSpinBox*>("spinBox_col_gaps_2");
    connect(ui->spinBox_col_gaps_2, QOverload<int>::of(&QSpinBox::valueChanged), [this, spin_box] { on_spinBox_col_gaps_valueChanged(spin_box->value()); });

    spin_box = Steg_Widget::findChild<QSpinBox*>("spinBox_row_gaps_2");
    connect(ui->spinBox_row_gaps_2, QOverload<int>::of(&QSpinBox::valueChanged), [this, spin_box] { on_spinBox_row_gaps_valueChanged(spin_box->value()); });
}

void Steg_Widget::on_pushButton_imagePath_clicked() {

    QString filters = "JPEG (*.jpg *.jpeg) ;; PNG (*.png) ;; BMP (*.bmp) ;; MP4 (*.mp4) ;; AVI (*.avi) ;; MOV (*.mov)";
    QString path = QFileDialog::getOpenFileName(this, "Choose Container", QDir::homePath(), filters);

    smodel.choose_container(path.toStdString());

    ui->lineEdit_imagePath_encode->setText(path);
}

void Steg_Widget::on_pushButton_imagePath_2_clicked() {

    QString filters = "PNG (*.png) ;; AVI (*.avi)";
    QString path = QFileDialog::getOpenFileName(this, "Choose Data", QDir::homePath(), filters);

    smodel.choose_container(path.toStdString());

    ui->lineEdit_imagePath_decode->setText(path);
}

void Steg_Widget::on_pushButton_dataPath_clicked() {

    QString path = QFileDialog::getOpenFileName(this, "Choose Data", QDir::homePath());

    smodel.choose_data(path.toStdString());

    ui->lineEdit_dataPath_encode->setText(path);
}

void Steg_Widget::on_pushButton_encode_clicked() {

    QString path = QFileDialog::getSaveFileName(this, "Save", QDir::homePath(), ext_filter);

    if("" != path) {
        smodel.encode(path.toStdString());
        ui->progressBar->setValue(0);
    }
}

void Steg_Widget::on_pushButton_decode_clicked() {

    QString path = QFileDialog::getExistingDirectory(this, "Choose Output Location",
                                                     QDir::homePath(),
                                                     QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if("" != path) {
        smodel.decode(path.toStdString());
    }
}

void Steg_Widget::on_tabWidget_currentChanged(int index) {

    clear_fields();
    smodel.set_action((0 == index) ? ActionType::ENCODE : ActionType::DECODE);
}

void Steg_Widget::clear_fields() {

    ui->pushButton_encode->setEnabled(false);
    ui->pushButton_decode->setEnabled(false);


    ui->checkBox_bit_1->setChecked(true);
    ui->checkBox_bit_1_2->setChecked(true);

    for(int i = 2; i <= 8; ++i) {
        QCheckBox* check_box = Steg_Widget::findChild<QCheckBox*>("checkBox_bit_" + QString::number(i));
        check_box->setChecked(false);

        check_box = Steg_Widget::findChild<QCheckBox*>("checkBox_bit_" + QString::number(i) + "_2");
        check_box->setChecked(false);
    }


    ui->checkBox_channel_B->setChecked(true);
    ui->checkBox_channel_B_2->setChecked(true);
    ui->checkBox_channel_G->setChecked(true);
    ui->checkBox_channel_G_2->setChecked(true);
    ui->checkBox_channel_R->setChecked(true);
    ui->checkBox_channel_R_2->setChecked(true);


    ui->spinBox_col_gaps->setValue(0);
    ui->spinBox_col_gaps_2->setValue(0);
    ui->spinBox_row_gaps->setValue(0);
    ui->spinBox_row_gaps_2->setValue(0);


    ui->label_container_capacity->setText("0 bytes");
    ui->label_data_size->setText("0 bytes");
    ui->lineEdit_imagePath_encode->setText("");
    ui->lineEdit_imagePath_decode->setText("");
    ui->lineEdit_dataPath_encode->setText("");

    ui->progressBar->setValue(0);
}

void Steg_Widget::model_show_size(DataType dt, std::uint64_t size) {

    std::stringstream ss;
    ss << size << " bytes";

    if(DataType::CONTAINER == dt) {
        ui->label_container_capacity->setText(QString::fromStdString(ss.str()));
    }
    else {
        ui->label_data_size->setText(QString::fromStdString(ss.str()));
    }
}

void Steg_Widget::model_enable_button(ActionType at, bool enable) {

    if(ActionType::ENCODE == at) { ui->pushButton_encode->setEnabled(enable); }
    else if(ActionType::DECODE == at) { ui->pushButton_decode->setEnabled(enable); }
}

void Steg_Widget::model_set_progBar(uint64_t val) {

    ui->progressBar->setMaximum(val);
}

void Steg_Widget::model_increment_progBar() {

    uint32_t val = ui->progressBar->value();
    ui->progressBar->setValue(++val);
}

void Steg_Widget::model_show_message(std::string str) {

    ui->textEdit->append(QString::fromStdString(str));

    ui->progressBar->setValue(0);
}

void Steg_Widget::model_set_filter(std::string str) {

    ext_filter = QString::fromStdString(str);
}

void Steg_Widget::checkBox_bits_stateChanged(int arg) {

    smodel.set_bits_value(arg);
}

void Steg_Widget::checkBox_channels_stateChanged(std::string arg) {

    smodel.set_channel_value(arg);
}

void Steg_Widget::on_spinBox_col_gaps_valueChanged(int arg) {

    smodel.set_columns_gaps(arg);
}

void Steg_Widget::on_spinBox_row_gaps_valueChanged(int arg) {

    smodel.set_rows_gaps(arg);
}


