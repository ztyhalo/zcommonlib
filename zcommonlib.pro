# CONFIG -= qt

TEMPLATE = lib
# DEFINES += COMMONLIB_LIBRARY
TARGET = zcommonlib
# CONFIG += c++17
QT       -= gui
QT       += core xml network
# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
QMAKE_CXXFLAGS += -std=c++11
TOPDIR = $$PWD/../../public/
include($$TOPDIR/app.pri)

# DESTDIR = $$QBUILD

INCLUDEPATH = .\
              bufmodel\
              date\
              epoll\
              mutex\
              prodata\
              prodata/sem\
              reflect\
              msgpro\
              sigslot\
              socket\
              tcp\
              timer\
              udp\
              file\
              zprint


SOURCES += \
    bufmodel/ZBufModel.cpp \
    date/com_date.cpp \
    epoll/e_poll.cpp \
    epoll/pthclass.cpp \
    file/zfile.cpp \
    mutex/mutex_class.cpp \
    prodata/lsystemsem.cpp \
    prodata/sem/syssem.cpp \
    prodata/zlockerclass.cpp \
    prodata/zqtsharemem.cpp \
    prodata/zsharememrw.cpp \
    prodata/zsysshm.cpp \
    prodata/zsysshmbase.cpp \
    prodata/zsystemsem.cpp \
    reflect/tbundle.cpp \
    socket/socket.cpp \
    tcp/tcp_class.cpp \
    tcp/tcp_client.cpp \
    timer/timers.cpp \
    udp/udp.cpp \
    zprint/cprintfclass.cpp \
    zprint/netprint.cpp \
    zprint/printfclass.cpp \
    zprint/zprint.cpp

HEADERS += \
    bufmodel/TwoBufPthT.h \
    bufmodel/ZBufModel.h \
    date/com_date.h \
    epoll/e_poll.h \
    epoll/pthclass.h \
    file/zfile.h \
    msgpro/msgprocess.h \
    mutex/mutex_class.h \
    prodata/clist.h \
    prodata/diffqtsharedataT.h \
    prodata/lsystemsem.h \
    prodata/pro_data.h \
    prodata/ptdataapp.h \
    prodata/qtsemshare.h \
    prodata/qtsharemem.h \
    prodata/sem/syssem.h \
    prodata/semshare.h \
    prodata/sharedateprocessT.h \
    prodata/sharemem.h \
    prodata/zbufT.h \
    prodata/zlockerclass.h \
    prodata/zmap.h \
    prodata/zmsg.h \
    prodata/zqtsharemem.h \
    prodata/zsharememrw.h \
    prodata/zsysshm.h \
    prodata/zsysshmbase.h \
    prodata/zsystemsem.h \
    reflect/reflect.h \
    reflect/tbundle.h \
    reflect/xmlprocess.h \
    sigslot/sigslot.h \
    socket/socket.h \
    tcp/tcp_class.h \
    tcp/tcp_client.h \
    timer/timers.h \
    udp/udp.h \
    zprint/cprintfclass.h \
    zprint/netprint.h \
    zprint/printfclass.h \
    zprint/printfconf.h \
    zprint/version.h \
    zprint/zprint.h

LIBS += -lssl -lcrypto
# Default rules for deployment.
DEPENDPATH += $${QBUILD}
