#pragma once

#include <QLabel>

namespace JChat {

	class ElidedLabel : public QLabel
	{
		Q_OBJECT

	public:
		ElidedLabel(QWidget *parent = Q_NULLPTR);
		~ElidedLabel();


		void setElideMode(Qt::TextElideMode elideMode) {
			elideMode_ = elideMode;
			updateGeometry();
		}

		Qt::TextElideMode elideMode() const { return elideMode_; }

		Q_SLOT void setText(const QString & txt);

	private:
		void resizeEvent(QResizeEvent* e);
		void paintEvent(QPaintEvent* e);
		void cacheElidedText(int w);

		Qt::TextElideMode elideMode_ = Qt::ElideRight;
		QString cachedElidedText;
	};

} // namespace JChat
