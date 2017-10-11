#include <windows.h>
#include <DbgHelp.h>
#pragma comment(lib,"DbgHelp")

#include <QTranslator>
#include <QLibraryInfo>
#include <QtWidgets/QApplication>
#include <QtMultimedia/QMediaPlayer>

#include "LoginWidget.h"


static LONG WINAPI crashHandler(EXCEPTION_POINTERS * ExceptionInfo)
{
	MINIDUMP_EXCEPTION_INFORMATION  M;
	HANDLE  hDump_File;
	wchar_t Dump_Path[512];

	M.ThreadId = GetCurrentThreadId();
	M.ExceptionPointers = ExceptionInfo;  //got by GetExceptionInformation()
	M.ClientPointers = 0;

	GetModuleFileNameW(nullptr, Dump_Path, sizeof(Dump_Path));
	lstrcatW(Dump_Path, L".dmp");

	hDump_File = CreateFileW(Dump_Path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hDump_File, (MINIDUMP_TYPE)(MiniDumpWithFullMemory),
					  ExceptionInfo ? &M : nullptr, nullptr, nullptr);

	CloseHandle(hDump_File);

	return EXCEPTION_CONTINUE_SEARCH;
}

int main(int argc, char *argv[])
{
	SetUnhandledExceptionFilter(crashHandler);
	qputenv("QT_SCALE_FACTOR", "1");

	QApplication a(argc, argv);
	a.setApplicationDisplayName("JChat");
	a.setQuitOnLastWindowClosed(false);
	a.setWindowIcon(QIcon(":/image/resource/icon.png"));

	QTranslator qtTranslator;
	if(qtTranslator.load(QLocale::system(),
						 "qt", "_",
						 QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
	{
		a.installTranslator(&qtTranslator);
	}

	QTranslator qtBaseTranslator;
	if(qtBaseTranslator.load("qtbase_" + QLocale::system().name(),
							 QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
	{
		a.installTranslator(&qtBaseTranslator);
	}




	QMediaPlayer* player = new QMediaPlayer();
	a.setProperty("QMediaPlayer", QVariant::fromValue(player));
	auto* w = new JChat::LoginWidget();
	w->show();
	return a.exec();
}
