#ifndef GLOBAL_H
#define GLOBAL_H

#include <QDir>
#include <QSettings>
#include <QString>
#include <list>

struct Config{
	QString version;
	QString appName					= "ESP Http Firmware Updater";
	QString file					= "";
	QString host					= "";
};

namespace app {
	extern Config conf;

	bool parsArgs(int argc, char *argv[]);
	void setLog(const uint8_t logLevel, const QString &mess);
	void loadSettings();
	void saveSettings();
}

#endif // GLOBAL_H
