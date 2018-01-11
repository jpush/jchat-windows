#-------------------------------------------------
#
# Project created by QtCreator 2018-01-03T16:15:15
#
#-------------------------------------------------

QT       += core gui widgets network sql multimedia multimediawidgets svg xml quick macextras

TARGET = JChat
TEMPLATE = app
CONFIG+=app_bundle

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

RC_FILE=Resource.rc

RESOURCES = Resource.qrc
RESOURCES += ResourceEmoji.qrc

FORMS += $$files(*.ui, false)

HEADERS += $$files(*.h, false)
HEADERS += $$files(*.hpp, false)

SOURCES += $$files(*.cpp, false)

CONFIG+=c++1z warn_off

CONFIG(debug, debug|release):TARGET=JChatd
CONFIG(release, debug|release):TARGET=JChat

win32{
	QMAKE_CXXFLAGS+= /std:c++latest /await

	LIBS+=Advapi32.lib gdi32.lib User32.lib

	INCLUDEPATH+=D:\__projects__\JChat\packages\jmessage-cpp.1.2.0-beta180102-3\build\native\include
	LIBS+=-LD:\__projects__\JChat\packages\jmessage-cpp.1.2.0-beta180102-3\build\native\lib\x86

	CONFIG(debug, debug|release):LIBS+=-ljmcppd
	CONFIG(release, debug|release)::LIBS+=-ljmcpp

	#boost
	INCLUDEPATH+=E:\__library__\boost_1_66_0_v141_x86
	LIBS+=-LE:/__library__/boost_1_66_0_v141_x86/lib

	#cpprestsdk
	INCLUDEPATH+=C:\Users\jiguang\.nuget\packages\cpprestsdk.v140.windesktop.msvcstl.dyn.rt-dyn\2.9.1\build\native\include
	LIBS+=-LC:\Users\jiguang\.nuget\packages\cpprestsdk.v140.windesktop.msvcstl.dyn.rt-dyn\2.9.1\lib\native\v140\windesktop\msvcstl\dyn\rt-dyn\x86

	CONFIG(debug, debug|release):LIBS+=Debug/cpprest140d_2_9.lib
	CONFIG(release, debug|release): LIBS+=Release/cpprest140_2_9.lib

	#ssl
	INCLUDEPATH+=D:\__projects__\vcpkg\installed\x86-windows-s\include

	CONFIG(debug, debug|release):LIBS+=-LD:\__projects__\vcpkg\installed\x86-windows-s\debug\lib -llibeay32 -lssleay32
	CONFIG(release, debug|release)::LIBS+= -LD:\__projects__\vcpkg\installed\x86-windows-s\lib -llibeay32 -lssleay32

}
else{
	DEFINES+=_LIBCPP_ENABLE_CXX17_REMOVED_FEATURES _LIBCPP_DISABLE_AVAILABILITY

	QMAKE_CXX= /usr/local/opt/llvm/bin/clang++
	QMAKE_LINK= $$QMAKE_CXX

	QMAKE_CXXFLAGS+= -nostdinc++ -I/usr/local/opt/llvm/include -I/usr/local/opt/llvm/include/c++/v1
	QMAKE_CXXFLAGS+= -fcoroutines-ts
	QMAKE_LFLAGS+= -L/usr/local/opt/llvm/lib -Wl,-rpath,@executable_path/../Frameworks  #-Wl,-rpath,/usr/local/opt/llvm/lib

	INCLUDEPATH+= /usr/local/opt/openssl/include
	INCLUDEPATH+= /usr/local/include
	LIBS+= -L/usr/local/lib
	LIBS+= -lc++experimental -lcpprest -lboost_system -lboost_filesystem   -lboost_thread -lboost_chrono


	DESTDIR+=/users/never/__projects__/_out


	CONFIG(debug, debug|release){
		INCLUDEPATH+= /users/never/__projects__/_out/Jmcppd.framework/Headers
		LIBS+= -F/users/never/__projects__/_out -framework Jmcppd
	}
	CONFIG(release, debug|release){
		INCLUDEPATH+=/users/never/__projects__/_out/Jmcpp.framework/Headers
		LIBS+= -F/users/never/__projects__/_out -framework Jmcpp
	}

	QMAKE_RPATHDIR+=/users/never/__projects__/_out

}

CONFIG(debug, debug|release):	LIBS += -L$$OUT_PWD/../QxOrm/ -L$$OUT_PWD/../QxOrm/debug/ -lQxOrmd
CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../QxOrm/ -L$$OUT_PWD/../QxOrm/release/ -lQxOrm

INCLUDEPATH += $$PWD/../QxOrm/include
DEPENDPATH +=  $$PWD/../QxOrm/include
