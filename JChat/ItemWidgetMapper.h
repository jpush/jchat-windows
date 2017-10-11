#pragma once

#include <map>
#include <type_traits>
#include <QObject>
#include <QPointer>
#include <QAbstractItemModel>
#include <QAbstractItemView>

namespace JChat
{
	class ItemWidgetMapper : public QObject
	{
		Q_OBJECT
	public:
		ItemWidgetMapper(QObject *parent = nullptr);

		void setView(QAbstractItemView* view);

		template<class T, class = std::enable_if_t< std::is_base_of_v<QWidget, T> && QtPrivate::HasQ_OBJECT_Macro<T>::Value  >  >
		void setItemWidgetClass()
		{
			setItemWidgetMetaObject(T::staticMetaObject);
		}


		void addMapping(int itemRole, const QByteArray & widgetPropertyName);

		void addMapping(int itemRole, std::function<void(QWidget*, QVariant const&)> writeFn);

		void removeMapping(int itemRole);


		Q_SIGNAL void itemWidgetCreated(QWidget* widget, QModelIndex const& index);

	protected:

		virtual bool eventFilter(QObject *watched, QEvent *event) override;
	private:
		void				setModel(QAbstractItemModel *model);
		QAbstractItemModel *model() const{ return _model; }

		void setItemWidgetMetaObject(QMetaObject const& metaObject);

		QWidget*			createWidget();

		void populate(QModelIndex const& index, QWidget*, const QVector<int> &roles = {});

	private:
		QAbstractItemModel*			_model = nullptr;

		QPointer<QAbstractItemView> _view;

		QMetaObject const*			_metaObject = nullptr;

		using Fn = std::function<void(QWidget*, QVariant const&)>;
		std::multimap<int, Fn> _roleFnMap;
	};

}

