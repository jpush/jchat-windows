
#include "ItemWidgetMapper.h"
#include <QResizeEvent>
#include <QLayout>



JChat::ItemWidgetMapper::ItemWidgetMapper(QObject *parent) : QObject(parent)
{

}

void
JChat::ItemWidgetMapper::setView(QAbstractItemView* view)
{
	if(_view == view)
	{
		if(_model != view->model())
		{
			setModel(view->model());
		}
		return;
	}

	if(_view)
	{
		_view->viewport()->removeEventFilter(this);
		setModel(nullptr);
	}

	_view = view;
	if(_view)
	{
		_view->viewport()->installEventFilter(this);

		auto model = _view->model();

		Q_ASSERT(model);
		setModel(model);
	}
}


void
JChat::ItemWidgetMapper::setModel(QAbstractItemModel *model)
{
	if(_model == model){ return; }

	if(_model)
	{
		disconnect(_model, 0, this, 0);
	}

	_model = model;
	if(!_model)
	{
		return;
	}

	connect(_model, &QAbstractItemModel::dataChanged, this,
			[=](const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
	{
		auto parent = topLeft.parent();
		if(_view->rootIndex() != parent)
		{
			return;
		}

		auto model = topLeft.model();
		for(auto row = topLeft.row(); row <= bottomRight.row(); row++)
		{
			for(auto col = topLeft.column(); col <= bottomRight.column(); col++)
			{
				auto index = model->index(row, col, parent);
				populate(index, 0, roles);
			}
		}
	});

	connect(_model, &QAbstractItemModel::rowsInserted, this,
			[=](const QModelIndex &parent, int first, int last)
	{
		if(parent != _view->rootIndex())
		{
			return;
		}

		for(; first <= last; first++)
		{
			auto index = _model->index(first, 0, parent);
			auto widget = createWidget();
			_view->setIndexWidget(index, widget);
			_model->setData(index, QSize(0, widget->height()), Qt::ItemDataRole::SizeHintRole);

			Q_EMIT itemWidgetCreated(widget, index);

			populate(index, widget);
		}
	});

	connect(_model, &QAbstractItemModel::rowsAboutToBeRemoved, this,
			[=](const QModelIndex &parent, int first, int last)
	{


	});

	connect(_model, &QAbstractItemModel::destroyed, this, [=]()
	{
		_model = nullptr;
	});


	auto rootIndex = _view->rootIndex();

	for(auto first = 0, count = _model->rowCount(rootIndex); first < count; first++)
	{
		auto index = _model->index(first, 0, rootIndex);
		auto widget = createWidget();
		_view->setIndexWidget(index, widget);
		_model->setData(index, QSize(0, widget->height()), Qt::ItemDataRole::SizeHintRole);

		Q_EMIT itemWidgetCreated(widget, index);

		populate(index, widget);
	}
}



void
JChat::ItemWidgetMapper::addMapping(int itemRole, const QByteArray & widgetPropertyName)
{
	_roleFnMap.emplace(itemRole, [widgetPropertyName](QWidget* widget, QVariant const& data)
	{
		widget->setProperty(widgetPropertyName.constData(), data);
	});
}

void
JChat::ItemWidgetMapper::addMapping(int itemRole, std::function<void(QWidget*, QVariant const&)> writeFn)
{
	_roleFnMap.emplace(itemRole, std::move(writeFn));
}

void
JChat::ItemWidgetMapper::removeMapping(int itemRole)
{
	_roleFnMap.erase(itemRole);
}

bool
JChat::ItemWidgetMapper::eventFilter(QObject *watched, QEvent *event)
{
	if(watched == _view->viewport())
	{
		if(event->type() == QEvent::Resize)
		{
			auto ev = static_cast<QResizeEvent*> (event);

			auto width = ev->size().width();

			for(auto row = 0, n = _model->rowCount(_view->rootIndex()); row < n; row++)
			{
				auto index = _model->index(row, 0, _view->rootIndex());
				if(auto widget = _view->indexWidget(index))
				{
					_model->setData(index, QSize(width, widget->height()), Qt::SizeHintRole);
				}
			}
		}
	}

	return false;
}

void
JChat::ItemWidgetMapper::setItemWidgetMetaObject(QMetaObject const& metaObject)
{
	_metaObject = &metaObject;
}

QWidget*
JChat::ItemWidgetMapper::createWidget()
{
	auto object = _metaObject->newInstance();
	auto widget = qobject_cast<QWidget*>(object);
	Q_ASSERT_X(widget, "ItemWidgetMapper::createWidget", "createWidget failed");
	return widget;
}

void
JChat::ItemWidgetMapper::populate(QModelIndex const& index, QWidget* widget, const QVector<int> &roles)
{
	if(!widget)
	{
		widget = _view->indexWidget(index);
	}

	if(!widget)
	{
		return;
	}


	if(roles.empty())
	{
		for(auto&&[role, fn] : _roleFnMap)
		{
			auto data = _model->data(index, role);
			fn(widget, data);
		}
	}
	else
	{
		for(auto&& role : roles)
		{
			for(auto[first, second] = _roleFnMap.equal_range(role); first != second; ++first)
			{
				auto data = _model->data(index, role);
				first->second(widget, data);
			}
		}
	}

}
