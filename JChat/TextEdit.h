#pragma once

#include <optional>
#include <QTextEdit>
#include <QCompleter>
#include <Jmcpp/Types.h>

namespace JChat {

	class TextEdit : public QTextEdit
	{
		Q_OBJECT

	public:
		TextEdit(QWidget *parent);
		~TextEdit();


		QString		getText(std::optional<Jmcpp::UserIdList> & userList);

		QCompleter* getCompleter() const { return _completer; }

		void		setCompleter(QCompleter*  val);

		QString		textUnderCursor() const;

	protected:
		
		QString leftChar(QTextCursor);

		void	updatePop();
		
		void	insert(QModelIndex const& index);
		
		virtual void keyPressEvent(QKeyEvent *event) override;


		virtual void insertFromMimeData(const QMimeData *source) override;

	private:
		QCompleter*		_completer = nullptr;
		int				_lastPostion = -1;
	
	};

} // namespace JChat
