#-------------------------------------------------
#
# Project created by QtCreator 2018-01-03T16:15:15
#
#-------------------------------------------------

QT += core gui widgets network sql multimedia multimediawidgets svg xml quick

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
	QMAKE_CXXFLAGS+= /std:c++17 /await
	LIBS+=advapi32.lib gdi32.lib user32.lib

	#need boost 1.66+
	INCLUDEPATH+=E:/__library__/boost_1_66_0_v141_x86
	LIBS+=-LE:/__library__/boost_1_66_0_v141_x86/lib

	DEFINES += _QX_STATIC_BUILD

	INCLUDEPATH+=$$PWD/../jmessage-sdk/jmessage-sdk-pc-win-1.2.3/include
	LIBS+=-L$$PWD/../jmessage-sdk/jmessage-sdk-pc-win-1.2.3/x86/lib

	CONFIG(debug, debug|release):LIBS+=-ljmcppd
	CONFIG(release, debug|release)::LIBS+=-ljmcpp

	CONFIG(debug, debug|release):LIBS+= cpprest141_2_10d.lib
	CONFIG(release, debug|release): LIBS+= cpprest141_2_10.lib


	DESTDIR = $$PWD/../build_v141_x86/bin
}
else{
	QT += macextras

	DEFINES+=_LIBCPP_ENABLE_CXX17_REMOVED_FEATURES _LIBCPP_DISABLE_AVAILABILITY

	QMAKE_CXXFLAGS+= -fcoroutines-ts

	QMAKE_RPATHDIR += @executable_path/../Frameworks

	INCLUDEPATH+= /usr/local/include
	LIBS+= -L/usr/local/lib
	LIBS+= -lcpprest -lboost_filesystem-mt

	DESTDIR+=$$PWD/../../JChatMacBin

	INCLUDEPATH+=$$PWD/../jmessage-sdk/jmessage-sdk-pc-mac-1.2.3/Jmcpp.framework/Headers
	LIBS+= -F$$PWD/../jmessage-sdk/jmessage-sdk-pc-mac-1.2.3 -framework Jmcpp

}

CONFIG(debug, debug|release):	LIBS += -L$$OUT_PWD/../QxOrm/ -L$$OUT_PWD/../QxOrm/debug/ -lQxOrmd
CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../QxOrm/ -L$$OUT_PWD/../QxOrm/release/ -lQxOrm

INCLUDEPATH += $$PWD/../QxOrm/include
DEPENDPATH +=  $$PWD/../QxOrm/include
