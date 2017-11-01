#include "ImageCut.h"

#include <QFileDialog>
#include <QEventLoop>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QScrollBar>
#include <QDebug>
namespace JChat {

	ImageCut::ImageCut(QWidget *parent)
		: QWidget(parent, Qt::Window)
	{
		ui.setupUi(this);
		setWindowModality(Qt::ApplicationModal);

		ui.graphicsView->setScene(&_scene);
		ui.graphicsView->viewport()->installEventFilter(this);
	}

	ImageCut::~ImageCut()
	{
	}

	Q_SLOT void ImageCut::on_btnOpenFile_clicked()
	{
		auto filePath = QFileDialog::getOpenFileName(this, tr("Open File"), "",
													 tr("Images (*.png *.jpg *.jpeg *.bmp *.gif)"));
		if(filePath.isEmpty())
		{
			return;
		}

		setImage(QImage(filePath));
	}

	Q_SLOT void ImageCut::on_horizontalSlider_valueChanged(int value)
	{
		auto t = QTransform().scale(_initScale + value*_scaleStep, _initScale + value*_scaleStep);
		ui.graphicsView->setTransform(t);
	}


	void ImageCut::setImage(QImage const& img)
	{
		_scene.clear();
		_item = nullptr;

		_item = new QGraphicsPixmapItem(QPixmap::fromImage(img));
		_scene.addItem(_item);

		ui.graphicsView->fitInView(_item, Qt::AspectRatioMode::KeepAspectRatioByExpanding);

		auto scale = ui.graphicsView->transform().m11();
		_initScale = ui.graphicsView->transform().m22();

		QSignalBlocker blocker(ui.horizontalSlider);
		ui.horizontalSlider->setValue(0);
		ui.horizontalSlider->setEnabled(isEnableScale());

		ui.graphicsView->setDragMode(isEnableScale() ? QGraphicsView::ScrollHandDrag : QGraphicsView::NoDrag);

		_scaleStep = (1.2 - _initScale) / 100;
	}

	QImage ImageCut::getImage() const
	{
		auto rect = ui.graphicsView->mapToScene(ui.graphicsView->rect()).boundingRect();
		if(_item)
		{
			QRect rectPixmap = _item->mapRectFromScene(rect).toRect();
			auto pixmap = _item->pixmap();

			QImage img{ ui.graphicsView->size() ,QImage::Format_ARGB32 };

			QPainter p(&img);
			p.setRenderHint(QPainter::SmoothPixmapTransform);
			p.drawPixmap(img.rect(), pixmap, rectPixmap);
			return img;
		}

		return {};
	}

	bool ImageCut::eventFilter(QObject *watched, QEvent *event)
	{
		if(watched == ui.graphicsView->viewport())
		{
			switch(event->type())
			{
				case QEvent::Wheel:
				{
					if(!isEnableScale())
					{
						return true;
					}
					auto e = static_cast<QWheelEvent*>(event);
					auto d = e->angleDelta().y();
					if(d > 0)
					{
						ui.horizontalSlider->setValue(ui.horizontalSlider->value() + 5);
					}
					else
					{
						ui.horizontalSlider->setValue(ui.horizontalSlider->value() - 5);
					}
					return true;
				}break;
				default:
					break;
			}
		}
		return false;
	}

	QImage ImageCut::cutImage(QWidget* parent)
	{
		QEventLoop el;
		ImageCut widget(parent);
		widget.show();

		connect(widget.ui.btnOK, &QPushButton::clicked, [&]
		{
			el.exit(0);
		});

		connect(widget.ui.btnCancel, &QPushButton::clicked, [&]
		{
			el.exit(1);
		});
		connect(&widget, &ImageCut::closed, [&]
		{
			el.exit(1);
		});

		return el.exec() ? QImage{} : widget.getImage();
	}


} // namespace JChat
