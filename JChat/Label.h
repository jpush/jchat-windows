#pragma once

#include <QLabel>

namespace JChat {

	class Label : public QLabel
	{
		Q_OBJECT

	public:
		Label(QWidget *parent = nullptr);
		~Label();

		Q_SIGNAL void clicked() const;
	protected:
		bool event(QEvent *e) override;
	};

} // namespace JChat
