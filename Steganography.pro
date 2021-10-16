QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += D:/Apps/OpenCV/release/install/include


LIBS += D:/Apps/OpenCV/release/bin/libopencv_core450.dll \
        D:/Apps/OpenCV/release/bin/libopencv_highgui450.dll \
        D:/Apps/OpenCV/release/bin/libopencv_imgcodecs450.dll \
        D:/Apps/OpenCV/release/bin/libopencv_imgproc450.dll \
        D:/Apps/OpenCV/release/bin/libopencv_videoio450.dll

SOURCES += src/main.cpp \
           src/steg_data.cpp \
           src/steg_model.cpp \
           src/steg_widget.cpp

HEADERS += include/steg_data.h \
           include/steg_model.h \
           include/steg_widget.h

FORMS += steg_widget.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
