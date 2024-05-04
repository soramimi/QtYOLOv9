#include "ImageView.h"

#include <QPainter>

/**
 * @brief コンストラクタ
 */
ImageView::ImageView(QWidget *parent)
	: QWidget{parent}
{
	
}

/**
 * @brief 画像を設定
 */
void ImageView::setImage(const QImage &image)
{
	image_ = image;
	update(); // 再描画
}

/**
 * @brief 描画
 */
void ImageView::paintEvent(QPaintEvent *)
{
	QPainter painter(this);
	int x = (width() - image_.width()) / 2;
	int y = (height() - image_.height()) / 2;
	painter.drawImage(QPoint(x, y), image_);
}
