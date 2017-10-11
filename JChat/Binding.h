#pragma once

#include <QObject>
#include <QPointer>

namespace JChat {

	class Binding : public QObject
	{
		Q_OBJECT
	public:
		Binding( QObject* source, QByteArray const& propSource, QObject* target, QByteArray const& propTarget, QObject *parent = nullptr);

		~Binding();
	protected:
		virtual bool eventFilter(QObject *watched, QEvent *event) override;
		Q_SLOT void setTargetProperty0();
	private:
		QPointer<QObject> target;
		QPointer<QObject> source;

		QByteArray propTarget;
		QByteArray propSource;
	};

} // namespace JChat
