#pragma once


#include <QSettings>

class Settings : public QSettings
{
	Q_OBJECT
public:
	Settings(QString const& filename, QObject *parent = 0)
		: QSettings(filename, QSettings::IniFormat, parent)
	{

	}

	using QSettings::value;

	QVariant value(const QString &key, const QVariant &defaultValue /*= QVariant()*/)
	{
		auto v = QSettings::value(key);
		if(v.isValid()) {
			return v;
		}
		else {
			QSettings::setValue(key, defaultValue);
			return defaultValue;
		}
	}

	template <class T>
	T value(QString const &key, const std::remove_reference_t<T> &defaultValue = std::remove_reference_t<T>())
	{
		auto v = QSettings::value(key);
		if(v.isValid()) {
			return qvariant_cast<T>(v);
		}
		else {
			QSettings::setValue(key, QVariant::fromValue(defaultValue));
			return defaultValue;
		}
	}
};
