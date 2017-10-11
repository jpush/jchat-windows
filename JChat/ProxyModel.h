#pragma once
#include <functional>
#include <map>
#include <QSortFilterProxyModel>

namespace JChat
{
	class ProxyModel : public QSortFilterProxyModel
	{
		Q_OBJECT
	public:
		ProxyModel(QObject *parent = nullptr);
		~ProxyModel();

		QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
		QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
		Qt::ItemFlags flags(const QModelIndex &index) const override;


		using FilterFn = std::function<void(int source_row, const QModelIndex &source_parent, bool& ret) >;
		void setFilterFn(FilterFn filterFn);


		using DataFn = std::function<void(const QModelIndex &index, QVariant& data) >;
		void setDataFnForRole(int role, DataFn dataFn);


		using HeaderDataFn = std::function<void(int section, Qt::Orientation orientation, QVariant& data)>;
		void setHeaderDataFnForRole(int role, HeaderDataFn dataFn);


		using FlagFn = std::function<void(const QModelIndex &index, Qt::ItemFlags& flags)>;
		void setFlagsFn(FlagFn flagFn);

		using CompareFn = std::function< void(const QModelIndex &source_left, const QModelIndex &source_right, bool& ret)>;
		void setCompareFn(CompareFn compFn);


		using QSortFilterProxyModel::invalidateFilter;

	protected:
		virtual bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
		virtual bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;

		std::map<int, DataFn>			_roleDataFn;
		std::map<int, HeaderDataFn>		_roleHeaderDataFn;
		FilterFn						_filterFn;
		FlagFn							_flagFn;
		CompareFn						_compFn;
	};
}


