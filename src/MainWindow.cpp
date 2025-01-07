#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "YOLOv9Detector.h"
#include <QDebug>
#include <QElapsedTimer>
#include <QFileDialog>
#include <QMessageBox>
#include <QPainter>

/**
 * @brief MainWindow のプライベート実装
 */
struct MainWindow::Private {
	YOLOv9Detector detector;
	QImage image;
};

/**
 * @brief コンストラクタ
 */
MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
	, m(new Private)
{
	ui->setupUi(this);
}

/**
 * @brief デストラクタ
 */
MainWindow::~MainWindow()
{
	delete ui;
	delete m;
}

/**
 * @brief YOLOv9モデルが読み込まれているかどうか
 */
bool MainWindow::isModelReady() const
{
	return m->detector;
}

/**
 * @brief YOLOv9モデルを読み込む
 */
bool MainWindow::loadModel()
{
	// 実行ファイルと同じ場所に yolov9.onnx が必要
	return m->detector.loadModel("yolov9.sim.onnx");
}

/**
 * @brief ファイルを開く
 */
void MainWindow::on_action_file_open_triggered()
{
	// 開くファイルを選択
	QString path = QFileDialog::getOpenFileName(this, "Open Image");
	if (path.isEmpty()) return;
	
	// 画像を読み込み
	QImage image = QImage(path);
	if (image.isNull()) {
		QMessageBox::critical(this, "Error", "Failed to load image");
		return;
	}
	
	m->image = image;
	
	// YOLOモデルの準備がまだなら読み込む
	if (!isModelReady()) {
		if (!loadModel()) {
			QMessageBox::critical(this, "Error", "Failed to load model");
			close();
			return;
		}
	}

	QElapsedTimer e;
	e.start();
	
	// 物体検出
	auto bboxes = m->detector.inference(m->image); // inference() は std::optional を返す
	if (bboxes) {
		// 検出結果が有効ならバウンディングボックスを描画する
		QPainter painter(&m->image);
		for (auto const &bbox : *bboxes) {
			// バウンディングボックスのラベル
			QString label = QString::number(bbox.index); // とりあえずクラスIDを文字列にする
			
			// TODO: クラスIDをラベルに変換する処理を追加
			// ラベルは data/coco.yaml に定義されている
			
			QString text = QString("[%1] %2").arg(label).arg(bbox.score, 0, 'f', 2);
			
			// バウンディングボックスを描画
			{
				painter.setPen(Qt::cyan); // 矩形の色
				painter.setBrush(Qt::NoBrush); // 塗りつぶさない
				painter.drawRect(bbox.rect); // 矩形を描画
			}
			// ラベルを描画
			{
				painter.setPen(Qt::black); // 文字の色
				QRect r = painter.fontMetrics().boundingRect(text).translated(bbox.rect.topLeft());
				painter.fillRect(r, Qt::cyan); // 背景を塗る;
				painter.drawText(bbox.rect.topLeft(), text); // ラベルを描画
			}
		}
	}
	
	qDebug() << "Elapsed time:" << e.elapsed() << "msec";

	// 画像を表示
	ui->centralwidget->setImage(m->image);
}

