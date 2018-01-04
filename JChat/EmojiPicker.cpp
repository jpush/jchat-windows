#include "EmojiPicker.h"

#include <QStandardItemModel>

#include "Emoji.h"

namespace JChat {

	static QStandardItemModel* emojiModel()
	{
		static QStandardItemModel* _model = []
		{
			auto model = new QStandardItemModel();

			auto e = JChat::Emoji::getSingleton();
			int i = 0;
			for(auto&& p : e->getPeople())
			{
				auto filename = e->fileName(e->convert(p.second["code_points"].toMap()["output"].toString()));

				auto item = new QStandardItem();
				item->setIcon(QIcon(filename));
				item->setData(p.second);
				item->setDragEnabled(false);
				item->setDropEnabled(false);
				model->appendRow(item);

				//if(++i == 300)break;
			}
			return model;
		}();

		return _model;
	}


	static QStandardItemModel* guangModel()
	{
		static QStandardItemModel* _model = []
		{
			static std::map<QString, QString> names = {

				{ ":/guang/resource/guang/01.png",u8"可爱" },
				{ ":/guang/resource/guang/02.png",u8"微笑" },
				{ ":/guang/resource/guang/03.png",u8"笑哭" },
				{ ":/guang/resource/guang/04.png",u8"害羞" },
				{ ":/guang/resource/guang/05.png",u8"两眼冒光" },
				{ ":/guang/resource/guang/06.png",u8"愤怒" },
				{ ":/guang/resource/guang/07.png",u8"痴汉" },
				{ ":/guang/resource/guang/08.png",u8"惊吓" },
				{ ":/guang/resource/guang/09.png",u8"无语" },
				{ ":/guang/resource/guang/10.png",u8"大哭" },
				{ ":/guang/resource/guang/11.png",u8"汗" },
				{ ":/guang/resource/guang/12.png",u8"晕" },
				{ ":/guang/resource/guang/13.png",u8"爽" },
				{ ":/guang/resource/guang/14.png",u8"睡觉" },
				{ ":/guang/resource/guang/15.png",u8"幸福" },
				{ ":/guang/resource/guang/16.png",u8"得意" },
				{ ":/guang/resource/guang/17.png",u8"么么哒" },
				{ ":/guang/resource/guang/18.png",u8"不理睬" },
				{ ":/guang/resource/guang/19.png",u8"阴笑" },
				{ ":/guang/resource/guang/20.png",u8"赞一个" },
				{ ":/guang/resource/guang/21.png",u8"再见" },
				{ ":/guang/resource/guang/22.png",u8"谢谢红包" },
				{ ":/guang/resource/guang/23.png",u8"冻住" },
				{ ":/guang/resource/guang/24.png",u8"石化" }

			};

			auto model = new QStandardItemModel();

			for(auto&& p : names)
			{
				auto item = new QStandardItem();
				item->setIcon(QIcon(p.first));
				item->setText(p.second);
				item->setData(p.first);
				item->setDragEnabled(false);
				item->setDropEnabled(false);
				model->appendRow(item);

			}
			return model;
		}();
		return _model;
	}



	EmojiPicker::EmojiPicker(QWidget *parent)
		: QWidget(parent, Qt::WindowType::Popup)
	{
		ui.setupUi(this);
		this->setFocusProxy(ui.listView);

		ui.listView->setModel(emojiModel());

		ui.listView_2->setModel(guangModel());

		connect(ui.listView, &QListView::clicked, this, [=](QModelIndex const& index)
		{
			if(!index.isValid())
			{
				return;
			}
			auto e = JChat::Emoji::getSingleton();

			auto info = index.data(Qt::UserRole + 1).toMap();

			auto emojiHtml = e->toImage(e->convert(info["code_points"].toMap().value("output").toString()));

			Q_EMIT emojiSelected(emojiHtml);
		});


		connect(ui.listView_2, &QListView::clicked, this, [=](QModelIndex const& index)
		{
			if(!index.isValid())
			{
				return;
			}

			auto file = index.data(Qt::UserRole + 1).toString();
			Q_EMIT largeEmojiSelected(file);
		});
	}

	EmojiPicker::~EmojiPicker()
	{
	}

} // namespace JChat
