#ifndef YOLOV9DETECTOR_H
#define YOLOV9DETECTOR_H

#include <optional>
#include <vector>
#include <QRect>

class QImage;

/**
 * @brief YOLOv9による物体検出
 */
class YOLOv9Detector {
public:
	struct BoundingBox {
		int index;
		float score;
		QRect rect;
	};
private:
	struct Private;
	Private *m;
public:
	YOLOv9Detector();
	~YOLOv9Detector();
	operator bool () const;
	bool loadModel(const char *model_path);
	std::optional<std::vector<BoundingBox>> inference(QImage const &image);
};

#endif // YOLOV9DETECTOR_H
