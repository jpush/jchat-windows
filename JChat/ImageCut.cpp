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

	Q_SLOT void ImageCut::on_rotateRight_clicked()
	{
		_rotate += 90;
		ui.graphicsView->rotate(90);
	}

	Q_SLOT void ImageCut::on_rotateLeft_clicked()
	{
		_rotate -= 90;
		ui.graphicsView->rotate(-90);
	}

	Q_SLOT void ImageCut::on_horizontalSlider_valueChanged(int value)
	{
		auto t = QTransform().rotate(_rotate).scale(_initScale + value*_scaleStep, _initScale + value*_scaleStep);
		ui.graphicsView->setTransform(t);
	}

	void ImageCut::fitInView(const QRectF &rect)
	{
		QRectF viewRect = ui.graphicsView->viewport()->rect();
		if(viewRect.isEmpty())
			return;

		QRectF sceneRect = ui.graphicsView->mapFromScene(rect).boundingRect();
		sceneRect.adjust(0, 0, -1, -1); // 
		if(sceneRect.isEmpty())
			return;

		auto xratio = viewRect.width() / sceneRect.width();
		auto yratio = viewRect.height() / sceneRect.height();

		xratio = yratio = qMax(xratio, yratio);

		ui.graphicsView->scale(xratio, yratio);
		ui.graphicsView->centerOn(rect.center());
	}

	void ImageCut::fitInView(QGraphicsItem* item)
	{
		QPainterPath path = item->isClipped() ? item->clipPath() : item->shape();
		path.translate(item->sceneTransform().dx(), item->sceneTransform().dy());
		fitInView(path.boundingRect());
	}

	void ImageCut::setImage(QImage const& img)
	{
		if(img.isNull())
		{
			return;
		}

		_scene.clear();
		_item = nullptr;
		_item = new QGraphicsPixmapItem(QPixmap::fromImage(img));
		_scene.addItem(_item);
		_scene.setSceneRect(_item->boundingRect());

		ui.graphicsView->resetTransform();
		fitInView(_item);

		_rotate = 0;
		auto scale = ui.graphicsView->transform().m11();
		_initScale = ui.graphicsView->transform().m22();

		QSignalBlocker blocker(ui.horizontalSlider);
		ui.horizontalSlider->setValue(0);
		ui.horizontalSlider->setEnabled(isEnableScale());

		ui.graphicsView->setDragMode(isEnableScale() ? QGraphicsView::ScrollHandDrag : QGraphicsView::NoDrag);

		_scaleStep = (1.0 - _initScale) / 100;
	}

	QImage ImageCut::getImage() const
	{
		auto rect = ui.graphicsView->mapToScene(ui.graphicsView->viewport()->rect()).boundingRect();
		if(_item)
		{
			auto rectPixmap = _item->mapRectFromScene(rect);
			auto pixmap = _item->pixmap();
			auto w = std::min(rectPixmap.width(), 801.);

			QImage img{ (int)w ,(int)w , QImage::Format_ARGB32 };
			QPainter p(&img);
			p.setRenderHint(QPainter::SmoothPixmapTransform);
			p.setWorldTransform(QTransform().translate(w / 2, w / 2).rotate(_rotate).translate(-w / 2, -w / 2));
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

	QImage ImageCut::cutImage(QWidget* parent, QImage const& img)
	{
		QEventLoop el;
		ImageCut widget(parent);
		widget.show();

		widget.setImage(img);

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
