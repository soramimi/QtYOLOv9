#include "YOLOv9Detector.h"
#include <onnxruntime/onnxruntime_cxx_api.h>
#include <QDebug>
#include <QImage>
#include <memory.h>

/**
 * @brief YOLOv9Detectorのプライベート実装
 */
struct YOLOv9Detector::Private {
	
	bool use_cuda = false; // CUDAを使用する場合はtrueにする
	
	std::unique_ptr<Ort::Env> env;
	std::unique_ptr<Ort::MemoryInfo> memory_info;
	
	Ort::SessionOptions session_options;
	std::unique_ptr<Ort::Session> session;
	
	size_t num_input_nodes = 0;
	size_t num_output_nodes = 0;
	std::vector<char const *> input_node_names;
	std::vector<char const *> output_node_names;
	std::vector<std::string> input_node_name_strings;
	std::vector<std::string> output_node_name_strings;
};

/**
 * @brief YOLOv9Detectorのコンストラクタ
 */
YOLOv9Detector::YOLOv9Detector()
	: m(new Private)
{
	// ONNX Runtime環境の初期化
	m->env = std::make_unique<Ort::Env>(ORT_LOGGING_LEVEL_WARNING, "test");
	
	m->memory_info = std::make_unique<Ort::MemoryInfo>(Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU));
	
	if (m->use_cuda) { // CUDAを使用する場合
		OrtCUDAProviderOptions cuda_options;
		m->session_options.AppendExecutionProvider_CUDA(cuda_options);
		// CUDAを使用する場合、推論の実行は高速ですが、
		// 初回だけCUDAの初期化に時間がかかります。
	} else {
		// CPUのみを使用する場合、
		// 一回の推論に1〜2秒程度かかります。
	}
	
	// 推論セッションの設定
	m->session_options.SetIntraOpNumThreads(1);
}

/**
 * @brief YOLOv9Detectorのデストラクタ
 */
YOLOv9Detector::~YOLOv9Detector()
{
	delete m;
}

/**
 * @brief YOLOv9Detectorが有効かどうか
 * @return 有効ならtrue
 */
YOLOv9Detector::operator bool() const
{
	return m->session != nullptr;
}

/**
 * @brief ONNXモデルを読み込む
 * @param model_path モデルファイルのパス
 * @return 成功した場合はtrue
 */
bool YOLOv9Detector::loadModel(const char *model_path)
{
	try {
		// セッションの生成
		m->session = std::make_unique<Ort::Session>(*m->env, model_path, m->session_options);
		
		// 入出力情報の取得
		m->num_input_nodes = m->session->GetInputCount();
		m->num_output_nodes = m->session->GetOutputCount();
		// qDebug() << "Number of input nodes:" << m->num_input_nodes;
		// qDebug() << "Number of output nodes:" << m->num_output_nodes;
		
		Ort::AllocatorWithDefaultOptions allocator;
		
 		// 入力ノード名の取得		
		m->input_node_names.resize(m->num_input_nodes);
		m->input_node_name_strings.resize(m->num_input_nodes);
		for (size_t i = 0; i < m->num_input_nodes; i++) {
			m->input_node_name_strings[i] = m->session->GetInputNameAllocated(i, allocator).get();
			m->input_node_names[i] = m->input_node_name_strings[i].c_str();
			// qDebug() << "input:" << m->input_node_names[i];
		}
		
		// 出力ノード名の取得
		m->output_node_names.resize(m->num_output_nodes);
		m->output_node_name_strings.resize(m->num_output_nodes);
		for (size_t i = 0; i < m->num_output_nodes; i++) {
			m->output_node_name_strings[i] = m->session->GetOutputNameAllocated(i, allocator).get();
			m->output_node_names[i] = m->output_node_name_strings[i].c_str();
			// qDebug() << "output:" << m->output_node_names[i];
		}
		
		return true;
		
	} catch (const Ort::Exception &e) {
		qDebug() << "Error:" << e.what();
	}
	return false;
}

/**
 * @brief 画像から物体検出を行う
 * @param image 画像
 * @return 物体検出結果
 */
std::optional<std::vector<YOLOv9Detector::BoundingBox>> YOLOv9Detector::inference(const QImage &image)
{
	const int N = 1; // batch size
	const int C = 3; // number of channels
	const int W = 640; // width
	const int H = 640; // height
	
	// 入力データの準備
	std::vector<float> input_tensor_values(N * C * H * W);
	
	// QImageを入力テンソルに変換
	{
		// 画素値を[0, 1]に正規化し、Packed RGB形式からPlanar BGR形式に変換
		QImage img = image.convertToFormat(QImage::Format_RGB888).scaled(W, H, Qt::IgnoreAspectRatio, Qt::FastTransformation);
		float *B = input_tensor_values.data() + H * W * 0;
		float *G = input_tensor_values.data() + H * W * 1;
		float *R = input_tensor_values.data() + H * W * 2;
		for (int y = 0; y < H; y++) {
			uint8_t const *src = img.scanLine(y);
			for (int x = 0; x < W; x++) {
				R[W * y + x] = src[3 * x + 0] / 255.0f;
				G[W * y + x] = src[3 * x + 1] / 255.0f;
				B[W * y + x] = src[3 * x + 2] / 255.0f;
			}
		}
	}
	std::vector<int64_t> input_tensor_shape = {1, 3, H, W};
	Ort::Value input_tensor = Ort::Value::CreateTensor<float>(*m->memory_info, input_tensor_values.data(), input_tensor_values.size(), input_tensor_shape.data(), input_tensor_shape.size());
	
	// 推論の実行
	auto output_tensors = m->session->Run(Ort::RunOptions{nullptr}, m->input_node_names.data(), &input_tensor, 1, m->output_node_names.data(), m->num_output_nodes);
	
	// 結果を生成
	std::vector<BoundingBox> bboxes;
	
	auto info = output_tensors.front().GetTensorTypeAndShapeInfo();
	std::vector<int64_t> shape = info.GetShape(); // e.g. {1, 84, 8400}
	
	Q_ASSERT(shape.size() == 3); // 3D tensor
	Q_ASSERT(shape[0] == N); // batch size
	Q_ASSERT(shape[1] > 4); // at least 5 values
	const int values = shape[1]; // maybe 84 if coco dataset
	const int classes = values - 4; // 80 classes
	const int count = shape[2];
	
	float const *output_tensor = output_tensors[0].GetTensorData<float>();
	
	for (int i = 0; i < count; i++) {
		auto Value = [&](int index){
			return output_tensor[count * index + i];
		};
		// x, y, w, h, class0, class1, ...
		float x = Value(0) * image.width() / W;
		float y = Value(1) * image.height() / H;
		float w = Value(2) * image.width() / W;
		float h = Value(3) * image.height() / H;
		x -= w / 2;
		y -= h / 2;
		for (int j = 0; j < classes; j++) { // クラス数
			BoundingBox bbox;
			bbox.score = Value(4 + j);
			if (bbox.score > 0.5) {
				bbox.index = j;
				bbox.rect = QRect(x, y, w, h);
				bboxes.push_back(bbox);
			}
		}
	}
	
	return std::move(bboxes);
}
