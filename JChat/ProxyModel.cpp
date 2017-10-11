#include "proxymodel.h"

namespace JChat
{
	ProxyModel::ProxyModel(QObject *parent)
		: QSortFilterProxyModel(parent)
	{

	}

	ProxyModel::~ProxyModel()
	{

	}

	QVariant ProxyModel::data(const QModelIndex &index, int role /*= Qt::DisplayRole*/) const
	{
		auto ret = QSortFilterProxyModel::data(index, role);

		auto iter = _roleDataFn.find(role);
		if(iter != _roleDataFn.end() && iter->second)
		{
			iter->second(index, ret);
		}
		return ret;
	}

	QVariant ProxyModel::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/) const
	{
		auto ret = QSortFilterProxyModel::headerData(section, orientation, role);

		auto iter = _roleHeaderDataFn.find(role);
		if(iter != _roleHeaderDataFn.end() && iter->second)
		{
			iter->second(section, orientation, ret);
		}

		return ret;
	}

	Qt::ItemFlags ProxyModel::flags(const QModelIndex &index) const
	{
		auto ret = QSortFilterProxyModel::flags(index);
		if(_flagFn)
		{
			_flagFn(index, ret);
		}
		return ret;
	}


	bool ProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
	{
		auto ret = QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
		if(_filterFn)
		{
			_filterFn(source_row, source_parent, ret);
		}
		return ret;
	}

	bool ProxyModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
	{
		auto ret = QSortFilterProxyModel::lessThan(source_left, source_right);
		if(_compFn)
		{
			_compFn(source_left, source_right, ret);
		}
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	void ProxyModel::setFilterFn(FilterFn filterFn)
	{
		_filterFn = std::move(filterFn);
		invalidateFilter();
	}

	void ProxyModel::setDataFnForRole(int role, DataFn dataFn)
	{
		_roleDataFn[role] = std::move(dataFn);
	}

	void ProxyModel::setHeaderDataFnForRole(int role, HeaderDataFn dataFn)
	{
		_roleHeaderDataFn[role] = std::move(dataFn);
	}

	void ProxyModel::setFlagsFn(FlagFn flagFn)
	{
		_flagFn = std::move(flagFn);
	}

	void ProxyModel::setCompareFn(CompareFn compFn)
	{
		_compFn = std::move(compFn);
	}


}