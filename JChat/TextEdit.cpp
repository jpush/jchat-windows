#include "TextEdit.h"

#include <optional>
#include <QKeyEvent>
#include <QScrollBar>
#include <QListView>
#include <QTextDocument>
#include <QTextBlock>
#include <QTextDocumentFragment>
#include <QTextTable>
#include <QUuid>

#include <QDebug>
#include "Emoji.h"
#include "MemberModel.h"
#include "ChatWidget.h"
#include "SendImage.h"
#include "SendFile.h"

namespace JChat {

	namespace detail
	{
		enum CustomProperty
		{
			UuidProperty = QTextCharFormat::UserProperty + 1,
			UserIdProperty
		};

		std::optional<QString> getUuid(QTextCursor const& cursor)
		{
			auto fmt = cursor.charFormat();
			auto uuid = fmt.property(UuidProperty);
			if(uuid.isNull())
			{
				return {};
			}
			return uuid.toString();
		}

		std::pair<QTextCursor, QTextCursor> findBoundaryImpl(QTextCursor const& cursor)
		{
			std::pair<QTextCursor, QTextCursor> result;
			auto cursorTmp = cursor;
			auto uuid = getUuid(cursorTmp);
			Q_ASSERT(uuid);
			auto rightBoundary = cursor;
			auto rightBoundaryTmp = rightBoundary;
			while(true)
			{
				if(rightBoundaryTmp.movePosition(QTextCursor::NextCharacter))
				{
					auto id2 = getUuid(rightBoundaryTmp);
					rightBoundary = rightBoundaryTmp;

					if(uuid != id2)
					{
						break;
					}
				}
				else
				{
					// #hack  failed
					auto id2 = getUuid(rightBoundaryTmp);
					rightBoundary = rightBoundaryTmp;
					if(uuid != id2)
					{
						break;
					}
					else
					{
						continue;
					}

					//qDebug() << rightBoundaryTmp.position() << rightBoundary.position();
					Q_ASSERT(false);
					break;
				}
			}

			result.second = rightBoundary;
			while(true)
			{
				if(rightBoundary.movePosition(QTextCursor::PreviousCharacter))
				{
					auto id2 = getUuid(rightBoundary);
					if(uuid != id2)
					{
						[[maybe_unused]]
						auto r = rightBoundary.movePosition(QTextCursor::PreviousCharacter);
						Q_ASSERT(r);
						break;
					}
				}
				else
				{
					Q_ASSERT(false);
					break;
				}
			}

			result.first = rightBoundary;
			return result;
		}

		std::optional<std::pair<QTextCursor, QTextCursor>>
			getBoundary(QTextCursor const& cursor)
		{
			static_assert(std::is_move_constructible_v<std::optional<std::pair<QTextCursor, QTextCursor>>>);
			static_assert(std::is_copy_constructible_v<std::optional<std::pair<QTextCursor, QTextCursor>>>);
			auto cursorTmp = cursor;
			cursorTmp.clearSelection();
			auto uuid = getUuid(cursorTmp);
			if(uuid)
			{
				return findBoundaryImpl(cursor);
			}
			if(cursorTmp.movePosition(QTextCursor::NextCharacter))
			{
				auto uid2 = getUuid(cursorTmp);
				if(uid2)
				{
					return findBoundaryImpl(cursorTmp);
				}
				else
				{
				}
			}
			else
			{
			}
			return {};
		}
	}


	TextEdit::TextEdit(QWidget *parent)
		: QTextEdit(parent)
	{

		connect(this, &QTextEdit::textChanged, [=]
		{
			updatePop();
		});


		connect(this, &QTextEdit::cursorPositionChanged, this, [=]
		{
			auto cursor = textCursor();
			if(_lastPostion == -1)
			{
				_lastPostion = cursor.position();
			}
			else
			{
				auto delta = cursor.position() - _lastPostion;
				_lastPostion = cursor.position();
				if(cursor.hasSelection())
				{

				}
				else
				{
					auto boundary = detail::getBoundary(cursor);
					if(boundary)
					{
						QSignalBlocker blocker(this);
						auto p = delta >= 0 ? boundary->second : boundary->first;
						cursor.setPosition(p.position());
						setTextCursor(cursor);
						_lastPostion = p.position();
					}
					else
					{
						updatePop();
					}
				}
			}
		});

		connect(this, &QTextEdit::selectionChanged, this, [=]
		{
			auto cursor = textCursor();
			if(cursor.hasSelection())
			{
				auto start = cursor.selectionStart();
				auto end = cursor.selectionEnd();
				auto pos = cursor.position();

				auto startNew = start;
				auto endNew = end;
				cursor.clearSelection();

				bool update = false;

				auto cursorTmp = cursor;
				cursorTmp.setPosition(start);

				auto b = detail::getBoundary(cursorTmp);
				if(b)
				{
					startNew = b->first.position();
					update = true;
				}
				cursorTmp.setPosition(end);
				auto b2 = detail::getBoundary(cursorTmp);
				if(b2)
				{
					endNew = b2->second.position();
					update = true;
				}
				if(update)
				{
					if(end == pos)
					{
						cursor.setPosition(startNew);
						cursor.setPosition(endNew, QTextCursor::KeepAnchor);
						_lastPostion = endNew;
					}
					else if(start == pos)
					{
						cursor.setPosition(endNew);
						cursor.setPosition(startNew, QTextCursor::KeepAnchor);
						_lastPostion = startNew;
					}
					QSignalBlocker blocker(this);
					setTextCursor(cursor);
				}
			}
		});

	}

	TextEdit::~TextEdit()
	{
	}

	QString TextEdit::leftChar(QTextCursor tc)
	{
		tc.clearSelection();
		tc.movePosition(QTextCursor::MoveOperation::Left, QTextCursor::KeepAnchor);
		return tc.selectedText();
	}

	void TextEdit::updatePop()
	{
		if(!_completer)
		{
			return;
		}
		auto c = _completer;

		QTextCursor tc = textCursor();
		QString completionPrefix = textUnderCursor();

		auto text = leftChar(tc);
		bool isShortcut = text == "@";

		if(!isShortcut && completionPrefix.isEmpty())
		{
			c->popup()->hide();
			return;
		}

		if(completionPrefix.contains("@"))
		{
			c->popup()->hide();
			return;
		}

		//if(completionPrefix != c->completionPrefix())
		{
			c->setCompletionPrefix(completionPrefix);
			c->popup()->setCurrentIndex(c->completionModel()->index(0, 0));
		}

		tc.movePosition(QTextCursor::MoveOperation::WordLeft, QTextCursor::MoveAnchor);
		tc.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor);
		text = tc.selectedText();

		if(isShortcut || text == "@")
		{
			QRect cr = cursorRect();
			cr.setWidth(c->popup()->sizeHintForColumn(0)
						+ c->popup()->verticalScrollBar()->sizeHint().width());
			c->complete(cr);
		}
	}


	void TextEdit::insert(QModelIndex const& index)
	{
		auto completion = index.data(Qt::ItemDataRole::DisplayRole).toString();
		auto userId = index.data(MemberModel::UserIdRole);
		QString text = textUnderCursor();

		QTextCursor tc = textCursor();
		if(!text.isEmpty())
		{
			if(leftChar(tc) == "@")
			{

			}
			else
			{
				tc.movePosition(QTextCursor::MoveOperation::WordLeft, QTextCursor::MoveAnchor);
			}
		}

		tc.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor);
		tc.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, text.size() + 1);
		tc.removeSelectedText();

		{

			QTextCharFormat normal;
			tc.beginEditBlock();
			tc.insertText(" ", normal);
			QTextCharFormat fmt;
			fmt.setForeground(Qt::blue);
			fmt.setProperty(detail::UuidProperty, QUuid::createUuid().toString());
			fmt.setProperty(detail::UserIdProperty, userId);

			tc.insertText("@" + completion, fmt);
			tc.insertText(" ", normal);
			tc.endEditBlock();
		}

		setTextCursor(tc);
	}

	QString TextEdit::getText(std::optional<Jmcpp::UserIdList> & userList)
	{
		bool atAllUser = false;
		Jmcpp::UserIdList userIdList;

		auto e = JChat::Emoji::getSingleton();

		auto html = toHtml();
		auto doc = document();

		for(auto b = doc->begin(), e = doc->end(); b != e; b = b.next())
		{
			for(auto iter = b.begin(); iter != b.end(); ++iter)
			{
				auto frag = iter.fragment();
				auto fmt = frag.charFormat();

				auto uid = fmt.property(detail::UuidProperty);
				if(uid.isNull())
				{
					continue;
				}

				auto userId = fmt.property(detail::UserIdProperty);
				if(userId.isNull())
				{
					atAllUser = true;
				}
				else
				{
					userIdList.push_back(userId.value<Jmcpp::UserId>());
				}
			}
		}

		if(atAllUser)
		{
			userList.emplace();
		}
		else
		{
			if(!userIdList.empty())
			{
				userList = userIdList;
			}
		}
		auto text = e->imageToUnicode(html);
		return text;
	}

	void TextEdit::setCompleter(QCompleter* val)
	{
		if(val == _completer)
		{
			return;
		}
		if(_completer)
		{
			QObject::disconnect(_completer, 0, this, 0);
			if(_completer->parent() == this)
			{
				delete _completer;
				_completer = nullptr;
			}
		}
		_completer = val;

		if(!_completer)
			return;

		_completer->setWidget(this);
		_completer->setCompletionMode(QCompleter::PopupCompletion);
		_completer->setCaseSensitivity(Qt::CaseInsensitive);

		QObject::connect(_completer, (void (QCompleter::*)(const QModelIndex &index)) &QCompleter::activated,
						 this, &TextEdit::insert);

	}

	QString TextEdit::textUnderCursor() const
	{
		QTextCursor tc = textCursor();
		tc.select(QTextCursor::WordUnderCursor);
		return tc.selectedText();
	}

	void TextEdit::keyPressEvent(QKeyEvent *e)
	{
		if(!_completer)
		{
			QTextEdit::keyPressEvent(e);
			return;
		}

		auto c = _completer;
		if(c && c->popup()->isVisible())
		{
			// The following keys are forwarded by the completer to the widget
			switch(e->key()) {
				case Qt::Key_Enter:
				case Qt::Key_Return:
				case Qt::Key_Escape:
				case Qt::Key_Tab:
				case Qt::Key_Backtab:
					e->ignore();
					return; // let the completer do default behavior
				default:
					break;
			}
		}

		switch(e->key())
		{
			case Qt::Key::Key_Delete:
			{
				auto cursor = textCursor();
				if(!cursor.hasSelection())
				{
					auto tmp = cursor;
					if(tmp.movePosition(QTextCursor::NextCharacter))
					{
						auto b = detail::getBoundary(tmp);
						if(b)
						{
							cursor.setPosition(b->second.position(), QTextCursor::KeepAnchor);
							cursor.removeSelectedText();

							return;
						}
					}
				}
			}break;
			case Qt::Key::Key_Backspace:
			{
				auto cursor = textCursor();
				if(!cursor.hasSelection())
				{
					auto tmp = cursor;
					if(tmp.movePosition(QTextCursor::PreviousCharacter))
					{
						auto b = detail::getBoundary(tmp);
						if(b)
						{
							cursor.setPosition(b->first.position(), QTextCursor::KeepAnchor);
							cursor.removeSelectedText();
							return;
						}
					}
				}
			}break;
			default:
				break;
		}

		QTextEdit::keyPressEvent(e);
	}

	void TextEdit::insertFromMimeData(const QMimeData *source)
	{
		auto getChatWidget = [this]
		{
			ChatWidget* widget = nullptr;
			for(auto parent = parentWidget();
				parent && !(widget = qobject_cast<ChatWidget*>(parent));
				parent = parent->parentWidget());

			return widget;
		};

		qDebug() << source->formats();

		if(source->hasImage())
		{
			QImage image = qvariant_cast<QImage>(source->imageData());
			ChatWidget* widget = getChatWidget();
			if(!widget)
			{
				return;
			}

			if(SendImage::exec(image, this->topLevelWidget()) == 0)
			{
				widget->sendImage(image);
			}
		}
		else if(source->hasUrls())
		{
			if(source->urls().empty())
			{
				return;
			}
			auto url = source->urls().front();
			ChatWidget* widget = getChatWidget();
			if(!widget)
			{
				return;
			}
			auto filePath = url.toLocalFile();
			QImage image(filePath);
			if(image.isNull())
			{
				if(SendFile::exec(filePath, this->topLevelWidget()) == 0)
				{
					widget->sendFile(filePath);
				}
			}
			else
			{
				if(SendImage::exec(image, this->topLevelWidget()) == 0)
				{
					widget->sendImage(filePath);
				}
			}

		}
		else if(source->hasHtml())
		{
			auto text = source->html();
			this->insertHtml(text);
		}
		else if(source->hasText())
		{
			auto fmts = source->formats();
			auto text = source->text();
			auto e = JChat::Emoji::getSingleton();
			auto html = e->toImage(text);
			this->insertHtml(html);
		}
	}

} // namespace JChat
