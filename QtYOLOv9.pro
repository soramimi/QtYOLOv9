
TARGET = QtYOLOv9
TEMPLATE = app
QT += core gui widgets

# ビルド結果はbinディレクトリに出力されます
DESTDIR = $$PWD/bin
# 実行ファイルと同じ場所にモデルファイル（yolov9.onnx）をコピーしておきます

INCLUDEPATH += $$PWD/src
LIBS += -lonnxruntime

SOURCES += \
	src/ImageView.cpp \
	src/MainWindow.cpp \
	src/YOLOv9Detector.cpp \
	src/main.cpp

FORMS += \
	src/MainWindow.ui

HEADERS += \
	src/ImageView.h \
	src/MainWindow.h \
	src/YOLOv9Detector.h
