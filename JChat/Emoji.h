#pragma once

#include <QObject>
#include <QDebug>
#include <QVariantMap>
#include <QRegularExpression>
#include <QtXml/QtXml>

#include <QtQml/QJSEngine>

#include <QxOrm.h>


namespace JChat {


	//!https://github.com/emojione/emojione
	class Emoji : public QObject, public qx::QxSingleton<Emoji>
	{
		Q_OBJECT

	public:
		Emoji(QObject *parent = nullptr);
		~Emoji();

		QString unifyUnicode(QString const& unicode);

		QString toShort(QString const& unicode);

		QString toImage(QString const& unicode);

		QString unicodeToImage(QString const& unicode);

		QString shortnameToImage(QString const& shortname);

		QString shortnameToUnicode(QString const& shortname);

		QString convert(QString const& uc);

		QString imageToUnicode(QString const& html);

		QString fileName(QString const& unicode);

		std::vector<std::pair<QString, QVariantMap> > const& getPeople() const
		{
			return _people;
		}
	private:
		void htmlToText(QString& result, QDomNode const& n);
	private:
		QJSEngine				_js;

		std::vector<QString>	_categorys;

		std::map<int, std::pair<QString, QVariantMap> >	_allEmojis;

		std::vector<std::pair<QString, QVariantMap> >		_people;

		std::map<QString, std::size_t> _peopleMap;

	};

} // namespace JChat
