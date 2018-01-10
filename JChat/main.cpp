
#ifdef _MSC_VER
#include <windows.h>
#include <DbgHelp.h>
#pragma comment(lib,"DbgHelp")
#endif // _MSC_VER


#include <QFontDatabase>
#include <QTranslator>
#include <QLibraryInfo>
#include <QtWidgets/QApplication>
#include <QtMultimedia/QMediaPlayer>

#include "LoginWidget.h"

#ifdef _MSC_VER
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

namespace console_colours
{
	const WORD BOLD = FOREGROUND_INTENSITY;
	const WORD RED = FOREGROUND_RED;
	const WORD CYAN = FOREGROUND_GREEN | FOREGROUND_BLUE;
	const WORD WHITE = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
	const WORD YELLOW = FOREGROUND_RED | FOREGROUND_GREEN;

	struct Reset
	{
		HANDLE out_handle; WORD old;
		~Reset()
		{
			SetConsoleTextAttribute(out_handle, old);
		}
	};

	Reset set_console_attribs(HANDLE out_handle_, WORD attribs)
	{
		CONSOLE_SCREEN_BUFFER_INFO orig_buffer_info;
		GetConsoleScreenBufferInfo(out_handle_, &orig_buffer_info);
		WORD back_color = orig_buffer_info.wAttributes;

		back_color &= ~(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);

		SetConsoleTextAttribute(out_handle_, attribs | back_color);
		return { out_handle_, orig_buffer_info.wAttributes };
	}
}


static void loggerFn(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
	static std::mutex lock;

	std::unique_lock<std::mutex> locker(lock);

	QByteArray localMsg = msg.toLocal8Bit();
	auto h = GetStdHandle(STD_OUTPUT_HANDLE);
	switch(type) {
		case QtDebugMsg:
		{
			auto reset = console_colours::set_console_attribs(h, console_colours::CYAN);
			std::clog << localMsg.constData() << std::endl;
		}break;
		case QtInfoMsg:
			std::clog << localMsg.constData() << std::endl;
			break;
		case QtWarningMsg:
		{
			auto reset = console_colours::set_console_attribs(h, console_colours::YELLOW);
			std::clog << localMsg.constData() << std::endl;
		}break;
		case QtCriticalMsg:
		{
			auto reset = console_colours::set_console_attribs(h, console_colours::RED | console_colours::BOLD);
			std::clog << localMsg.constData() << std::endl;
		}break;
		case QtFatalMsg:
		{
			auto reset = console_colours::set_console_attribs(h, console_colours::WHITE | BACKGROUND_RED);
			std::clog << localMsg.constData() << std::endl;
			abort();
		}break;
	}
}
#endif // _MSC_VER


int main(int argc, char *argv[])
{
#ifdef _MSC_VER
	SetUnhandledExceptionFilter(crashHandler);
	qInstallMessageHandler(loggerFn);
#endif

	qputenv("QT_SCALE_FACTOR", "1");

	QApplication a(argc, argv);

	a.setApplicationName("JChat");
	a.setApplicationDisplayName("JChat");
	a.setQuitOnLastWindowClosed(false);
	a.setWindowIcon(QIcon(":/image/resource/icon.png"));

#ifdef Q_OS_MACOS
	QDir dir=a.applicationDirPath();
	dir.cdUp();
	auto fontFile= dir.absoluteFilePath("Resources/msyh.ttc");
	//QFontDatabase::addApplicationFont(fontFile);
#endif

	QCoreApplication::setAttribute(Qt::AA_Use96Dpi);

	a.setFont(QFont("Microsoft YaHei",10));
	a.setStyleSheet(R"( QWidget{ font-family:"Microsoft YaHei";	} )");

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
