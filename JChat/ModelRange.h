#pragma once
#include <queue>
#include <iterator>
#include <QVector>
#include <QObject>
#include <QtGui/QStandardItemModel>

namespace JChat{

	struct DFStandardItemRange
	{
		QStandardItem* root;

		struct DFStandardItemIterator
		{
			using difference_type = std::ptrdiff_t;
			using value_type = DFStandardItemIterator;
			using pointer = DFStandardItemIterator*;

			using reference = DFStandardItemIterator&;
			using iterator_category = std::forward_iterator_tag;

			QStandardItem*	root;

			QStandardItem*	current;

			int				level;

			QVector<QStandardItem*> columns;

			DFStandardItemIterator& operator++()
			{
				if(auto child = current->child(0))
				{
					columns.resize(current->columnCount());
					int i = 0;
					for(auto&& col : columns)
					{
						col = current->child(0, i++);
					}

					current = child;
					level++;
				}
				else
				{
					for(;;)
					{
						auto row = current->row();
						auto parent = current->parent();

						if(!parent)
						{
							parent = current->model()->invisibleRootItem();
						}
						auto next = parent->child(row + 1);
						if(next)
						{
							columns.resize(parent->columnCount());
							int i = 0;
							for(auto&& col : columns)
							{
								col = parent->child(row + 1, i++);
							}
							current = next;
							break;
						}
						else
						{
							current = parent;
							level--;
							if(current == root)
							{
								break;
							}
						}
					}
				}
				return *this;
			}

			QStandardItem* operator->() const
			{
				return current;
			}

			DFStandardItemIterator& operator*()
			{
				return *this;
			}


			QStandardItem* operator[](std::size_t col) const
			{
				return columns[col];
			}

			bool operator!=(DFStandardItemIterator const& it) const
			{
				return current != it.current;
			}
		};

		DFStandardItemIterator begin() const
		{
			Q_ASSERT(root);
			if(auto first = root->child(0))
			{
				DFStandardItemIterator iter{ root, first , 0 };
				iter.columns.resize(root->columnCount());

				int i = 0;
				for(auto&& col : iter.columns)
				{
					col = root->child(0, i++);
				}

				return iter;
			}
			else
			{
				return{ root, root,-1 };
			}
		}

		DFStandardItemIterator end() const
		{
			return{ root,root,-1 };
		}
	};

	inline DFStandardItemRange traverseDepthFirst(QStandardItem* root)
	{
		return{ root };
	}

	inline DFStandardItemRange traverseDepthFirst(const QStandardItemModel* model)
	{
		return traverseDepthFirst(model->invisibleRootItem());
	}

	//////////////////////////////////////////////////////////////////////////
	struct BFStandardItemRange
	{
		QStandardItem* root;

		struct BFStandardItemIterator
		{
		private:
			struct Item
			{
				QStandardItem*	item;
				int				level;
			};
			std::queue< Item >				_queue;

		public:
			QVector<QStandardItem*>			columns;
			QStandardItem*					current = nullptr;
			int								level = 0;

			BFStandardItemIterator& operator++()
			{
				if(!_queue.empty())
				{
					auto currentItem = _queue.front();
					_queue.pop();

					current = currentItem.item;
					level = currentItem.level;

					auto parent = current->parent();
					if(!parent)
					{
						parent = current->model()->invisibleRootItem();
					}

					auto row = current->row();
					columns.resize(parent->columnCount());
					int i = 0;
					for(auto&& col : columns)
					{
						col = parent->child(row, i++);
					}

					{
						int c = current->rowCount();
						auto l = level + 1;
						for(int r = 0; r < c; r++)
						{
							_queue.emplace(Item{ current->child(r,0),l });
						}
					}
				}
				else
				{
					current = nullptr;
				}

				return *this;
			}

			QStandardItem* operator->() const
			{
				return current;
			}

			BFStandardItemIterator& operator*()
			{
				return *this;
			}


			QStandardItem* operator[](std::size_t col) const
			{
				return columns[col];
			}

			bool operator!=(BFStandardItemIterator const& iter) const
			{
				return current != iter.current;
			}

			friend BFStandardItemRange;
		};


		BFStandardItemIterator begin() const
		{
			Q_ASSERT(root);
			BFStandardItemIterator iter{};

			if(auto c = root->rowCount())
			{
				for(int r = 0; r < c; r++)
				{
					iter._queue.emplace(BFStandardItemIterator::Item{ root->child(r,0),0 });
				}
				++iter;
			}
			return iter;
		}

		BFStandardItemIterator end() const
		{
			BFStandardItemIterator iter{};
			return iter;
		}

	};

	inline BFStandardItemRange traverseBreadthFirst(QStandardItem* root)
	{
		return{ root };
	}
	inline BFStandardItemRange traverseBreadthFirst(const QStandardItemModel* model)
	{
		return traverseBreadthFirst(model->invisibleRootItem());
	}
}

namespace JChat{

	struct DFModelIndexRange
	{
		QModelIndex					root;
		const QAbstractItemModel *	model;

		struct DFModelIndexIterator
		{
			const QAbstractItemModel* model;
			QModelIndex		root;
			QModelIndex		current;
			int				level;

			QVector<QModelIndex> columns;

			DFModelIndexIterator& operator++()
			{
				auto child = current.child(0, 0);
				if(child.isValid())
				{
					columns.resize(model->columnCount(current));
					int i = 0;
					for(auto&& col : columns)
					{
						col = current.child(0, i++);
					}
					current = child;
					level++;
				}
				else
				{
					for(;;)
					{
						auto parent = current.parent();
						auto next = current.sibling(current.row() + 1, 0);
						if(next.isValid())
						{
							columns.resize(model->columnCount(parent));
							int i = 0;
							for(auto&& col : columns)
							{
								col = next.sibling(next.row(), i++);
							}
							current = next;
							break;
						}
						else
						{
							current = parent;
							level--;
							if(current == root)
							{
								break;
							}
						}
					}
				}
				return *this;
			}

			QModelIndex* operator->()
			{
				return &current;
			}

			DFModelIndexIterator& operator*()
			{
				return *this;
			}


			QModelIndex* operator[](std::size_t col)
			{
				return &columns[col];
			}

			bool operator!=(DFModelIndexIterator const& it) const
			{
				return current != it.current;
			}
		};

		DFModelIndexIterator begin() const
		{
			if(root.isValid())
			{
				auto first = root.child(0, 0);
				if(first.isValid())
				{
					DFModelIndexIterator iter{ model, root, first , 0 };
					iter.columns.resize(model->columnCount(root));
					int i = 0;
					for(auto&& col : iter.columns)
					{
						col = first.sibling(first.row(), i++);
					}
					return iter;
				}
				else
				{
					return{ model, root, root,-1 };
				}
			}
			else
			{
				auto first = model->index(0, 0);
				if(first.isValid())
				{
					DFModelIndexIterator iter{ model, root, first , 0 };
					iter.columns.resize(model->columnCount(root));
					int i = 0;
					for(auto&& col : iter.columns)
					{
						col = first.sibling(first.row(), i++);
					}
					return iter;
				}
				else
				{
					return{ model, root, root,-1 };
				}
			}
		}

		DFModelIndexIterator end() const
		{
			return{ model, root, root,-1 };
		}
	};

	inline DFModelIndexRange traverseDepthFirst(QModelIndex const& root)
	{
		Q_ASSERT(root.isValid());
		return{ root,root.model() };
	}

	inline DFModelIndexRange traverseDepthFirst(const QAbstractItemModel* model)
	{
		return{ QModelIndex(),model };
	}


	struct BFModelIndexRange
	{
		QModelIndex					root;
		const QAbstractItemModel *	model;

		struct BFModelIndexIterator
		{
		private:
			struct Item
			{
				QModelIndex		index;
				int				level;
			};
			std::queue< Item >				_queue;
		public:
			const QAbstractItemModel *		model;

			QVector<QModelIndex>			columns;
			QModelIndex						current;
			int								level = 0;

			BFModelIndexIterator& operator++()
			{
				if(!_queue.empty())
				{
					auto currentItem = _queue.front();
					_queue.pop();

					current = currentItem.index;
					level = currentItem.level;

					auto parent = current.parent();

					columns.resize(model->columnCount(parent));
					int i = 0;
					for(auto&& col : columns)
					{
						col = current.sibling(current.row(), i++);
					}

					{
						int c = model->rowCount(current);
						auto l = level + 1;
						for(int r = 0; r < c; r++)
						{
							_queue.emplace(Item{ current.child(r,0),l });
						}
					}
				}
				else
				{
					current = QModelIndex();
				}

				return *this;
			}

			QModelIndex* operator->()
			{
				return &current;
			}

			BFModelIndexIterator& operator*()
			{
				return *this;
			}

			QModelIndex* operator[](std::size_t col)
			{
				return &columns[col];
			}

			bool operator!=(BFModelIndexIterator const& iter) const
			{
				return current != iter.current;
			}

			friend BFModelIndexRange;
		};


		BFModelIndexIterator begin() const
		{
			BFModelIndexIterator iter{};
			iter.model = model;

			if(root.isValid())
			{
				if(auto c = model->rowCount(root))
				{
					for(int r = 0; r < c; r++)
					{
						iter._queue.emplace(BFModelIndexIterator::Item{ root.child(r,0),0 });
					}
					++iter;
				}
			}
			else
			{
				if(auto c = model->rowCount(root))
				{
					for(int r = 0; r < c; r++)
					{
						iter._queue.emplace(BFModelIndexIterator::Item{ model->index(r,0),0 });
					}
					++iter;
				}
			}
			return iter;
		}

		BFModelIndexIterator end() const
		{
			BFModelIndexIterator iter{};
			return iter;
		}
	};

	inline BFModelIndexRange traverseBreadthFirst(QModelIndex const& root)
	{
		Q_ASSERT(root.isValid());
		return{ root,root.model() };
	}

	inline BFModelIndexRange traverseBreadthFirst(const QAbstractItemModel* model)
	{
		return{ QModelIndex(),model };
	}

	namespace _qx_range
	{
		template<class T>
		struct static_const{ static const T value; };
		template<class T>
		const T static_const<T>::value;

		struct DepthFirstTag{};
		struct BreadthFirstTag{};


		inline DFStandardItemRange operator|(QStandardItem* root, _qx_range::DepthFirstTag)
		{
			return traverseDepthFirst(root);
		}

		inline BFStandardItemRange operator|(QStandardItem* root, _qx_range::BreadthFirstTag)
		{
			return traverseBreadthFirst(root);
		}

		inline DFStandardItemRange operator|(const QStandardItemModel* model, _qx_range::DepthFirstTag)
		{
			return traverseDepthFirst(model);
		}

		inline BFStandardItemRange operator|(const QStandardItemModel* model, _qx_range::BreadthFirstTag)
		{
			return traverseBreadthFirst(model);
		}
		//////////////////////////////////////////////////////////////////////////
		inline DFModelIndexRange operator|(QModelIndex const& root, _qx_range::DepthFirstTag)
		{
			return traverseDepthFirst(root);
		}

		inline BFModelIndexRange operator|(QModelIndex const& root, _qx_range::BreadthFirstTag)
		{
			return traverseBreadthFirst(root);
		}
		//////////////////////////////////////////////////////////////////////////
		inline DFModelIndexRange operator|(const QAbstractItemModel* model, _qx_range::DepthFirstTag)
		{
			return traverseDepthFirst(model);
		}

		inline BFModelIndexRange operator|(const QAbstractItemModel* model, _qx_range::BreadthFirstTag)
		{
			return traverseBreadthFirst(model);
		}
	}

	namespace
	{
		const auto& depthFirst = _qx_range::static_const<_qx_range::DepthFirstTag>::value;
		const auto& breadthFirst = _qx_range::static_const<_qx_range::BreadthFirstTag>::value;
	}

}