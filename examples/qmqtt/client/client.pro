TEMPLATE = app
TARGET = qmqtt_example
QT = core network qmqtt

CONFIG += console
SOURCES += \
    example.cpp

target.path = $$[QT_INSTALL_EXAMPLES]/qmqtt/client
INSTALLS += target

message('====path===='$$target.path)
