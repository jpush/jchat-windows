#include "Emoji.h"

#include <QFile>
#include <QTextStream>

#include <QJsonObject>
#include <QJsonDocument>

#include <QDebug>

namespace JChat {

	Emoji::Emoji(QObject *parent)
		: QObject(parent)
		, QxSingleton("Emoji")
	{
		QString fileName = ":/js/emojione.js";

		QFile scriptFile(fileName);
		if(!scriptFile.open(QIODevice::ReadOnly))
		{
			return;
		}
		QTextStream stream(&scriptFile);
		QString contents = stream.readAll();
		scriptFile.close();

		_js.evaluate(contents, fileName);

		auto fn = _js.evaluate(R"(

		//emojione.greedyMatch=true;
		
		function(path)
		{
			emojione.imagePathPNG=path;
		}

		)");

		QJSValueList args;
		args << ":/emoji/emoji/EmojiOne_3.1.1_32x32_png/";
		fn.call(args);


		QFile j(":/js/emoji.json");

		if(!j.open(QIODevice::ReadOnly))
		{
			return;
		}

		auto doc = QJsonDocument::fromJson(j.readAll());

		auto emojis = doc.object().toVariantMap().toStdMap();


		for(auto&& emoji : emojis)
		{
			auto info = emoji.second.toMap();
			auto category = info["category"].toString();

			auto order = info["order"].toInt();

			_allEmojis.emplace(order, std::make_pair(emoji.first, info));
		}

		for(auto&& emoji : _allEmojis)
		{
			auto category = emoji.second.second["category"].toString();

			auto display = emoji.second.second["display"].toInt();

			auto keywords = emoji.second.second["keywords"].toStringList();

			for(auto&& keyword : keywords)
			{
				if(keyword.contains("skin tone"))
				{
					goto SKIP;
				}
			}
			if(category == "people" && display)
			{
				_people.emplace_back(emoji.second.first, emoji.second.second);

				_peopleMap.try_emplace(emoji.second.first, _people.size() - 1);
			}

		SKIP:;

		}

		_allEmojis.clear();
	}

	Emoji::~Emoji()
	{

	}

	QString Emoji::unifyUnicode(QString const& unicode)
	{
		auto value = _js.evaluate(R"( emojione.unifyUnicode	)");
		QJSValueList args;
		args << unicode;
		auto ret = value.call(args);

		return ret.toString();
	}

	QString Emoji::toShort(QString const& unicode)
	{
		auto value = _js.evaluate(R"( emojione.toShort	)");
		QJSValueList args;
		args << unicode;
		auto ret = value.call(args);

		return ret.toString();
	}

	QString Emoji::toImage(QString const& unicode)
	{
		auto value = _js.evaluate(R"( emojione.toImage	)");
		QJSValueList args;
		args << unicode;
		auto ret = value.call(args);

		return ret.toString();
	}

	QString Emoji::unicodeToImage(QString const& unicode)
	{
		auto value = _js.evaluate(R"( emojione.unicodeToImage  )");
		QJSValueList args;
		args << unicode;
		auto ret = value.call(args);

		return ret.toString();
	}

	QString Emoji::shortnameToImage(QString const& shortname)
	{
		auto value = _js.evaluate(R"( emojione.shortnameToImage	)");
		QJSValueList args;
		args << shortname;
		auto ret = value.call(args);
		return ret.toString();
	}


	QString Emoji::shortnameToUnicode(QString const& shortname)
	{
		auto value = _js.evaluate(R"( emojione.shortnameToUnicode	)");
		QJSValueList args;
		args << shortname;
		auto ret = value.call(args);
		return ret.toString();
	}


	QString Emoji::convert(QString const& uc)
	{
		auto fn = _js.evaluate(R"(
			emojione.convert;
		)");

		QJSValueList args;
		args << uc;
		auto str = fn.call(args).toString();

		return str;
	}


	void Emoji::htmlToText(QString& result, QDomNode const& n)
	{
		QDomText t = n.toText();
		if(!t.isNull())
		{
			result += t.data();
			return;
		}

		auto e = n.toElement();
		if(!e.isNull())
		{
			if(e.tagName() == "span")
			{
				result += e.text();
				return;
			}
			else if(e.tagName() == "img")
			{
				auto src = e.attribute("src");

				QFileInfo info(src);
				auto uc = info.baseName();

				if(auto iter = _peopleMap.find(uc); iter != _peopleMap.end())
				{
					auto index = iter->second;
					Q_ASSERT(index < _people.size());

					auto&& info = _people[index].second;

					auto output = info["code_points"].toMap()["output"].toString();

					auto unicode = convert(output);

					result += unicode;
				}

			}
			else if(e.tagName() == "a")
			{
				for(QDomNode n = e.firstChild(); !n.isNull(); n = n.nextSibling())
				{
					htmlToText(result, n);
				}
			}
		}
	}

	QString Emoji::imageToUnicode(QString const& html)
	{
		QDomDocument doc;
		QXmlInputSource source;
		source.setData(html);
		QXmlSimpleReader reader;

		if(!doc.setContent(&source, &reader))
		{
			Q_ASSERT(false);
		}

		QString result;
		auto root = doc.firstChildElement("html");
		auto body = root.firstChildElement("body");

		auto p = body.firstChildElement("p");

		bool first = true;
		while(!p.isNull())
		{
			if(!first)
			{
				result += '\n';
			}
			else
			{
				first = false;
			}

			for(QDomNode n = p.firstChild(); !n.isNull(); n = n.nextSibling())
			{
				htmlToText(result, n);
			}

			p = p.nextSiblingElement("p");
		}
		return result;
	}

	QString Emoji::fileName(QString const& unicode)
	{
		auto fn = _js.evaluate(R"(
			emojione.fileName;
		)");

		QJSValueList args;
		args << unicode;
		auto str = fn.call(args).toString();

		return str;
	}

} // namespace JChat
