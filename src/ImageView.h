#ifndef IMAGEVIEW_H
#define IMAGEVIEW_H

#include <QWidget>

/**
 * @brief 画像表示ウィジェット
 */
class ImageView : public QWidget {
	Q_OBJECT
private:
	QImage image_;
protected:
	void paintEvent(QPaintEvent *);
public:
	explicit ImageView(QWidget *parent = nullptr);
	void setImage(const QImage& image);
};

#endif // IMAGEVIEW_H
