#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

/**
 * @brief メインウィンドウ
 */
class MainWindow : public QMainWindow {
	Q_OBJECT
private:
	Ui::MainWindow *ui;
	struct Private;
	Private *m;
	
	bool isModelReady() const;
	bool loadModel();
public:
	explicit MainWindow(QWidget *parent = nullptr);
	~MainWindow();
private slots:
	void on_action_file_open_triggered();
};

#endif // MAINWINDOW_H
