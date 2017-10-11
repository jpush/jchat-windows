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

				model->appendRow(item);

				if(++i == 140)
				{
					break;
				}
			}
			return model;
		}();

		return _model;
	}


	EmojiPicker::EmojiPicker(QWidget *parent)
		: QWidget(parent, Qt::Tool | Qt::FramelessWindowHint)
	{
		ui.setupUi(this);
		this->setFocusProxy(ui.listView);

		ui.listView->setModel(emojiModel());

		connect(ui.listView, &QListView::clicked, this, [=](QModelIndex const& index)
		{
			if(!index.isValid())
			{
				return;
			}

			auto info = index.data(Qt::UserRole + 1).toMap();
			Q_EMIT emojiSelected(info);
		});
	}

	EmojiPicker::~EmojiPicker()
	{
	}

} // namespace JChat
