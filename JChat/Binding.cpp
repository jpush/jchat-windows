#include "Binding.h"
#include <QMetaObject>
#include <QMetaProperty>
#include <QDynamicPropertyChangeEvent>


JChat::Binding::Binding(QObject* source, QByteArray const& propSource, QObject* target, QByteArray const& propTarget, QObject *parent)
	:QObject(parent ? parent : source)
	, target(target)
	, propTarget(propTarget)
	, source(source)
	, propSource(propSource)
{
	auto idx = source->metaObject()->indexOfProperty(propSource);
	if(idx != -1)
	{
		auto metaProp = source->metaObject()->property(idx);
		if(!metaProp.hasNotifySignal())
		{
			Q_ASSERT_X(false, "Binding", "hasNotifySignal==false");
			return;
		}

		auto method = metaProp.notifySignal();

		{
			auto signature = QMetaObject::normalizedSignature("setTargetProperty0()");
			auto idx = staticMetaObject.indexOfMethod(signature);
			if(idx == -1)
			{
				return;
			}
			connect(source, method, this, staticMetaObject.method(idx));
		}

		auto value = source->property(propSource.constData());
		target->setProperty(propTarget.constData(), value);

		return;
	}
	else
	{
		source->installEventFilter(this);
	}
}

JChat::Binding::~Binding()
{

}

bool
JChat::Binding::eventFilter(QObject *watched, QEvent *event)
{
	if(watched == source && event->type() == QEvent::DynamicPropertyChange)
	{
		auto e = (QDynamicPropertyChangeEvent*)event;
		if(e->propertyName() == propSource)
		{
			setTargetProperty0();
		}
	}

	return false;
}

Q_SLOT void
JChat::Binding::setTargetProperty0()
{
	if(target)
		target->setProperty(propTarget.constData(), source->property(propSource.constData()));
}

