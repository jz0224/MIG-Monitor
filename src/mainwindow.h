#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "camerathread.h"
#include <regex>
#include <direct.h>
#include <QMainWindow>
#include <QDialog>
#include <QPixmap>
#include <QLabel>
#include <QImage>
#include <QComboBox>
#include <QMessageBox>
#include <QTreeWidget>
#include <QFileDialog>
#include <QVector>
#include <vector>
#include <QtCharts>
#include <QTimer>
#include <math.h>
#include <memory>

#include "processthread.h"
#include "controlthread.h"

using namespace QtCharts;

namespace Ui {
	class MainWindow;
	class SaveSetDialog;
	class ConfigureDialog;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
	void ReadSettings();
	void WriteSettings();

	Ui::MainWindow *ui;

    QList<int> data;
    int graph_count = 0;

    QComboBox *synchro_box;
    QComboBox *polarity_box;

	QTreeWidgetItem *temItem = nullptr;
	int temColumn;

	CameraThread* mpCameraThread;
    ProcessThread* mpProcessThread;
	ControlThread* mpControlThread;
	CameraParameters cameraParams;

	std::string g_SaveFolder;

	QLineSeries* i_series;
	QChart* i_chart;
	QValueAxis* i_axisX;
	QValueAxis* i_axisY;

	QLineSeries* v_series;
	QChart* v_chart;
	QValueAxis* v_axisX;
	QValueAxis* v_axisY;

	ImageChannelPtr mpImageChannel;

signals:
	void UpdateProcessParam(ImageProcessParameters);

private slots:
    void openEditor(QTreeWidgetItem *item, int column);
    void closeEditor();
    void updateFrame(QImage);

    void on_actionConnect_triggered();
    void on_actionDisconnect_triggered();
    void on_actionSetSavePath_triggered();
    void on_actionUploadParameters_triggered();
    void on_actionStart_triggered();
    void on_actionStop_triggered();
    void on_actionRecord_toggled(bool);
	void on_processButton_toggled(bool);
	void on_actionBoard_triggered();

    void receiveSaveSet(QString, size_t);

	void ReceiveAnalogData(QVector<double>);

	void Slotfinish();

	void receiveWidth(double);
	void receiveLength(double);
	void displayROI(QImage);
	void ConnectLostHandler();
};

class SaveSetDialog : public QDialog
{
	Q_OBJECT

public:
	explicit SaveSetDialog(QWidget *parent = 0);
	~SaveSetDialog();
	

signals:
	void sendSaveSet(QString, size_t);

private slots:
	void on_BrowserBtn_clicked();
	void on_ConfirmBtn_clicked();

private:
	Ui::SaveSetDialog *ui;
	QString mSavePath;
	size_t mSaveFreq;
};

#endif // MAINWINDOW_H