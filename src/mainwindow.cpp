#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui_savesetdialog.h"
#include <QMetaType>
#include <memory>

#include "imagechannel.h"

Q_DECLARE_METATYPE(QVector<double>);

#define REPOSITORY_SIZE (100)

const double ratio = 10.0;

const QString settingFilePath = "./settings.ini";

SaveSetDialog::SaveSetDialog(QWidget *parent) :
	QDialog(parent), ui(new Ui::SaveSetDialog)
{
	ui->setupUi(this);
	QSettings config(settingFilePath, QSettings::IniFormat);
	config.beginGroup("Save_Path");
	mSavePath = config.value(QString("Save Path"), "D:/").toString();
	config.endGroup();
	ui->savepath_edit->setText(mSavePath);
}

SaveSetDialog::~SaveSetDialog()
{
	delete ui;

	QSettings config(settingFilePath, QSettings::IniFormat);

	config.beginGroup("Save_Path");
	config.setValue(QString("Save Path"), mSavePath);
	config.endGroup();
}

void SaveSetDialog::on_BrowserBtn_clicked()
{
	mSavePath = QFileDialog::getExistingDirectory(this, tr("Save Path"), "D:/");

	if (!mSavePath.isEmpty()) {
		ui->savepath_edit->setText(mSavePath);
	}
}

void SaveSetDialog::on_ConfirmBtn_clicked()
{
	mSaveFreq = ui->savefreq_edit->text().toInt();
	emit sendSaveSet(mSavePath, mSaveFreq);
    this->close();
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->display_wnd->setScaledContents(true);
	ui->roi_wnd->setScaledContents(true);

    synchro_box = new QComboBox;
    polarity_box = new QComboBox;
    synchro_box->addItem("Extern");
	synchro_box->addItem("Intern");
    polarity_box->addItem("Positive edge");
    polarity_box->addItem("Positive level");
    polarity_box->addItem("Negative edge");
    polarity_box->addItem("Negative level");
    ui->parameter_tree->setItemWidget(ui->parameter_tree->topLevelItem(4), 1, synchro_box);
    ui->parameter_tree->setItemWidget(ui->parameter_tree->topLevelItem(5), 1, polarity_box);

	ReadSettings();

    connect(ui->parameter_tree,SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(openEditor(QTreeWidgetItem*, int)));
    connect(ui->parameter_tree,SIGNAL(itemSelectionChanged()), this, SLOT(closeEditor()));
    ui->parameter_tree->setStyleSheet("QTreeWidget::item{height:30px}");

	ui->actionRecord->setCheckable(true);

	qRegisterMetaType<QVector<double>>("QVector<double>");

	i_series = new QLineSeries;
	i_series->setUseOpenGL(true);
	i_chart = new QChart;
	i_series->append(0, 0);
	i_chart->addSeries(i_series);
	i_chart->createDefaultAxes();

	i_axisX = new QValueAxis;
	i_axisX->setRange(0, 40);
	i_axisX->setGridLineVisible(true);
	i_axisX->setTickCount(5);     //标记的个数
	i_axisX->setMinorTickCount(1); //次标记的个数

	i_axisY = new QValueAxis;
	i_axisY->setRange(50, 300);
	i_axisY->setGridLineVisible(true);
	i_axisY->setTickCount(6);
	//i_axisY->setMinorTickCount(1);

	i_chart->setAxisX(i_axisX, i_series);
	i_chart->setAxisY(i_axisY, i_series);
	i_chart->setTitle("Current");
	i_chart->legend()->hide();
	i_chart->setMargins(QMargins(0, 0, 0, 0));
	i_chart->setTheme(QChart::ChartThemeDark);

	v_series = new QLineSeries;
	v_series->setUseOpenGL(true);
	v_chart = new QChart;
	v_series->append(0, 0);
	v_chart->addSeries(v_series);
	v_chart->createDefaultAxes();

	v_axisX = new QValueAxis;
	v_axisX->setRange(0, 40);
	v_axisX->setGridLineVisible(true);
	v_axisX->setTickCount(5);     //标记的个数
	v_axisX->setMinorTickCount(1); //次标记的个数

	v_axisY = new QValueAxis;
	v_axisY->setRange(0, 30);
	v_axisY->setGridLineVisible(true);
	v_axisY->setTickCount(5);
	//v_axisY->setMinorTickCount(1);

	v_chart->setAxisX(v_axisX, v_series);
	v_chart->setAxisY(v_axisY, v_series);
	v_chart->setTitle("Voltage");
	v_chart->legend()->hide();
	v_chart->setMargins(QMargins(0, 0, 0, 0));
	v_chart->setTheme(QChart::ChartThemeDark);

	ui->graphicsView->setChart(i_chart);
	ui->graphicsView->setRenderHint(QPainter::Antialiasing);

	ui->graphicsView_3->setChart(v_chart);
	ui->graphicsView_3->setRenderHint(QPainter::Antialiasing);

	ui->AI_Current->setPalette(Qt::green);
	ui->AI_Voltage->setPalette(Qt::green);

	ImageChannelPtr pImageChannel = std::make_shared<ImageChannel>(REPOSITORY_SIZE);
	mpImageChannel = std::move(pImageChannel);
}

MainWindow::~MainWindow()
{
	WriteSettings();
    delete ui;
}

void MainWindow::openEditor(QTreeWidgetItem *item, int column)
{
    if (column == 1)
    {
        ui->parameter_tree->openPersistentEditor(item, column);
        temItem = item;
        temColumn = column;
    }
}

void MainWindow::closeEditor()
{
    if (temItem != nullptr)
    {
        ui->parameter_tree->closePersistentEditor(temItem, temColumn);
    }
}

void MainWindow::updateFrame(QImage display_img)
{
    ui->display_wnd->setPixmap(QPixmap::fromImage(display_img));
}

void MainWindow::displayROI(QImage display_roi)
{
	ui->roi_wnd->setPixmap(QPixmap::fromImage(display_roi));
}

void MainWindow::on_actionConnect_triggered()
{
	mpCameraThread = new CameraThread(mpImageChannel);

	if (mpCameraThread->ConnectToDevice() < 0) {
		QMessageBox::warning(this, tr("Warning"), tr("Camera is not connected!"), QMessageBox::Ok);
		return;
	}
	ui->actionConnect->setEnabled(false);
	ui->actionDisconnect->setEnabled(true);
	ui->actionUploadParameters->setEnabled(true);
}

void MainWindow::on_actionDisconnect_triggered()
{
	mpCameraThread->Disconnect();
	delete mpCameraThread;
	mpCameraThread = nullptr;

	ui->actionConnect->setEnabled(true);
	ui->actionDisconnect->setEnabled(false);
	ui->actionStart->setEnabled(false);
	ui->actionStop->setEnabled(false);
	ui->actionSetSavePath->setEnabled(false);
	ui->actionRecord->setEnabled(false);
	ui->actionUploadParameters->setEnabled(false);
	ui->display_wnd->setEnabled(false);
	ui->roi_wnd->setEnabled(false);
}

void MainWindow::on_actionSetSavePath_triggered()
{
    SaveSetDialog *save_set_dialog = new SaveSetDialog();
    connect(save_set_dialog, SIGNAL(sendSaveSet(QString, size_t)), this, SLOT(receiveSaveSet(QString, size_t)));
	save_set_dialog->setWindowTitle("Save Path Set");
	save_set_dialog->exec();
	delete save_set_dialog;
}

void MainWindow::on_actionUploadParameters_triggered()
{
    QString width = ui->parameter_tree->topLevelItem(0)->text(1);
    QString height = ui->parameter_tree->topLevelItem(1)->text(1);
    QString frameRate = ui->parameter_tree->topLevelItem(2)->text(1);
    QString exposure = ui->parameter_tree->topLevelItem(3)->text(1);
    QString synchro = synchro_box->currentText();
    QString polarity = polarity_box->currentText();

    cameraParams.imageWidth = width.toStdString();
    cameraParams.imageHeight = height.toStdString();
    cameraParams.exposureTime = exposure.toStdString();
    cameraParams.frameFrequency = frameRate.toStdString();
    if (synchro == "Intern")
        cameraParams.synchroMode = std::string("0");
    else if (synchro == "Extern")
        cameraParams.synchroMode = std::string("1");

    if (polarity == "Positive edge")
        cameraParams.polarityMode = std::string("11");
    else if (polarity == "Positive level")
        cameraParams.polarityMode = std::string("10");
    else if (polarity == "Negative edge")
        cameraParams.polarityMode = std::string("01");
    else if (polarity == "Negative level")
        cameraParams.polarityMode = std::string("00");

	mpCameraThread->SetCameraParameter(cameraParams);
	std::cout << "parameter set" << std::endl;
	mpCameraThread->StartCapture();

	mpProcessThread = new ProcessThread(mpImageChannel);
	connect(mpProcessThread, SIGNAL(sendImg(QImage)), this, SLOT(updateFrame(QImage)));
	mpProcessThread->StartDisplay();

	ui->display_wnd->setEnabled(true);
	ui->actionSetSavePath->setEnabled(true);
	ui->actionStart->setEnabled(true);
}

void MainWindow::on_actionStart_triggered()
{
	//connect(mpProcessThread, SIGNAL(connectLost()), this, SLOT(ConnectLostHandler()));

	//判断相机是否在采像
	if (!mpCameraThread->IsCapturing())
	{
		mpCameraThread->StartCapture();
		mpProcessThread->StartDisplay();
	}

	//控制线程开始
	if (mpControlThread != nullptr && !mpControlThread->IsCollecting())
	{
		i_series->clear();
		v_series->clear();
		mpControlThread->StartCollect();
	}
		
	ui->actionStart->setEnabled(false);
	ui->actionStop->setEnabled(true);
	ui->actionSetSavePath->setEnabled(true);
    ui->actionUploadParameters->setEnabled(false);
	ui->actionDisconnect->setEnabled(false);
}

void MainWindow::on_actionStop_triggered()
{
    //on_actionRecord_toggled(false);
	
	mpProcessThread->StopAllTasks();
	mpCameraThread->StopCapture();
	if (mpControlThread != nullptr && mpControlThread->IsCollecting())
		mpControlThread->StopCollect();

	ui->actionStart->setEnabled(true);
	ui->actionStop->setEnabled(false);
    //ui->actionSetSavePath->setEnabled(false);
    ui->actionRecord->setEnabled(false);
    ui->actionUploadParameters->setEnabled(true);
	ui->actionDisconnect->setEnabled(true);
}

void MainWindow::receiveSaveSet(QString saveDir, size_t saveFreq)
{
	if (saveDir.isEmpty()) return;

	std::string tmp_dir = saveDir.toStdString();
	tmp_dir = std::regex_replace(tmp_dir, std::regex("/"), std::string("\\"));
	g_SaveFolder = tmp_dir;
	
	mpProcessThread->SetSaveFreq(saveFreq);

	ui->actionRecord->setEnabled(true);
}

void MainWindow::on_actionRecord_toggled(bool pressed)
{
    if (pressed == true)
    {
		time_t t = time(0);
		char tmp_time[64];
		strftime(tmp_time, sizeof(tmp_time), "%Y%m%d%H%M", localtime(&t));
		std::string savePath = g_SaveFolder + "\\" + std::string(tmp_time);
		mkdir(savePath.c_str());

		mpProcessThread->SetSavePath(savePath);
		mpProcessThread->StartRecord();

		if (mpControlThread != nullptr && mpControlThread->IsCollecting())
		{
			mpControlThread->SetSavePath(savePath + ".csv");
			
			mpControlThread->StartSaving();
		}
    }
	else
	{
		mpProcessThread->StopRecord();
		if (mpControlThread != nullptr && mpControlThread->IsCollecting())
		{
			//mpControlThread->StopCollect();
			mpControlThread->StopSaving();
		}
	}
	ui->actionRecord->setChecked(pressed);
    ui->actionSetSavePath->setEnabled(!pressed);
}

void MainWindow::on_processButton_toggled(bool pressed)
{
	if (pressed == true)
	{
		connect(mpProcessThread, SIGNAL(sendWidth(double)), this, SLOT(receiveWidth(double)));
		connect(mpProcessThread, SIGNAL(sendLength(double)), this, SLOT(receiveLength(double)));
		connect(mpProcessThread, SIGNAL(sendROI(QImage)), this, SLOT(displayROI(QImage)));

		ImageProcessParameters ipParameters;
		ipParameters.roi_x = ui->parameter_tree->topLevelItem(6)->child(0)->text(1).toInt();
		ipParameters.roi_y = ui->parameter_tree->topLevelItem(6)->child(1)->text(1).toInt();
		ipParameters.roi_w = ui->parameter_tree->topLevelItem(6)->child(2)->text(1).toInt();
		ipParameters.roi_h = ui->parameter_tree->topLevelItem(6)->child(3)->text(1).toInt();
		ipParameters.ip_threshold = ui->parameter_tree->topLevelItem(6)->child(4)->text(1).toInt();
		mpProcessThread->SetProcessParam(ipParameters);

		mpProcessThread->StartProcess();
	}
	else
	{
		mpProcessThread->StopProcess();

		disconnect(mpProcessThread, SIGNAL(sendWidth(double)), this, SLOT(receiveWidth(double)));
		disconnect(mpProcessThread, SIGNAL(sendLength(double)), this, SLOT(receiveLength(double)));
		disconnect(mpProcessThread, SIGNAL(sendROI(QImage)), this, SLOT(displayROI(QImage)));
	}
	ui->roi_wnd->setEnabled(true);
	ui->processButton->setChecked(pressed);
}

void MainWindow::on_actionBoard_triggered()
{
	mpControlThread = new ControlThread();
	int rtn = mpControlThread->Initialize();
	if (rtn != 0)
	{
		QMessageBox::warning(this, tr("Warning"), tr("Cannot connect to data acquisition board!"), QMessageBox::Ok);
		return;
	}
	else
	{
		QMessageBox::information(this, tr("Information"), mpControlThread->GetDeviceName()+tr(" is connected!"), QMessageBox::Ok);
	}
	connect(mpControlThread, SIGNAL(SendAnalogData(QVector<double>)), this, SLOT(ReceiveAnalogData(QVector<double>)));
	connect(mpControlThread, SIGNAL(SendFinishSignal()), this, SLOT(ReceiveFinishSignal()));
}

void MainWindow::ReceiveFinishSignal()
{
	QMessageBox::information(this, tr("Notice"), tr("Capture Finished!"), QMessageBox::Ok);
}

void MainWindow::ReceiveAnalogData(QVector<double> analogData)
{
	int iCurrent = static_cast<int>(analogData[0] * 219);
	int iVoltage = static_cast<int>(analogData[1] * 21);
	double t = analogData[2];
	ui->AI_Current->display(iCurrent);
	ui->AI_Voltage->display(iVoltage);
	i_series->append(t, iCurrent);
	v_series->append(t, iVoltage);
}

void MainWindow::receiveWidth(double width)
{
	ui->widthBar->setValue(width/ratio);
	ui->widthBar->setFormat(QString::fromLocal8Bit("Width：%1mm").arg(QString::number(width / ratio, 'f', 2)));
	//电压建议
	if (width > 100 && width < 170)
		ui->VoltageBar->setValue(19.5);
	else if (width < 180)
		ui->VoltageBar->setValue(18.5);
	else if (width < 185)
		ui->VoltageBar->setValue(18);
	else if (width < 190)
		ui->VoltageBar->setValue(17.5);
	else if (width < 200)
		ui->VoltageBar->setValue(17);
}

void MainWindow::receiveLength(double length)
{
	ui->lengthBar->setValue(length/ratio);
	ui->lengthBar->setFormat(QString::fromLocal8Bit("Length：%1mm").arg(QString::number(length / ratio, 'f', 2)));
	//电流建议
	if (length > 100 && length < 240)
		ui->CurrentBar->setValue(190);
	else if (length < 250)
		ui->CurrentBar->setValue(185);
	else if (length < 255)
		ui->CurrentBar->setValue(180);
	else if (length < 280)
		ui->CurrentBar->setValue(175);
	else if (length < 290)
		ui->CurrentBar->setValue(170);
	else if (length < 300)
		ui->CurrentBar->setValue(165);
}

void MainWindow::ConnectLostHandler()
{
	QMessageBox::warning(this, tr("WARNING"), tr("Camera Connection Lost!"), QMessageBox::Ok);
}

void MainWindow::ReadSettings()
{
	QSettings config(settingFilePath, QSettings::IniFormat);

	config.beginGroup("Camera");
	QString width = config.value(QString("Width"), 1280).toString();
	QString height = config.value(QString("Height"), 1024).toString();
	QString fps = config.value(QString("Frame rate"), 20).toString();
	QString expose = config.value(QString("Exposure time"), 2000).toString();
	QString trigger = config.value(QString("Trigger mode"), "Intern").toString();
	QString polarity = config.value(QString("Polarity"), "Positive edge").toString();
	QString roi_x = config.value(QString("ROI_x"), 0).toString();
	QString roi_y = config.value(QString("ROI_y"), 0).toString();
	QString roi_w = config.value(QString("ROI_w"), width.toInt()).toString();
	QString roi_h = config.value(QString("ROI_h"), height.toInt()).toString();
	QString threshold = config.value(QString("Threshold"), 30).toString();
	config.endGroup();

	ui->parameter_tree->topLevelItem(0)->setText(1, width);
	ui->parameter_tree->topLevelItem(1)->setText(1, height);
	ui->parameter_tree->topLevelItem(2)->setText(1, fps);
	ui->parameter_tree->topLevelItem(3)->setText(1, expose);
	synchro_box->setCurrentText(trigger);
	polarity_box->setCurrentText(polarity);
	ui->parameter_tree->topLevelItem(6)->child(0)->setText(1, roi_x);
	ui->parameter_tree->topLevelItem(6)->child(1)->setText(1, roi_y);
	ui->parameter_tree->topLevelItem(6)->child(2)->setText(1, roi_w);
	ui->parameter_tree->topLevelItem(6)->child(3)->setText(1, roi_h);
	ui->parameter_tree->topLevelItem(6)->child(4)->setText(1, threshold);
	}

void MainWindow::WriteSettings()
{
	QSettings config(settingFilePath, QSettings::IniFormat);

	config.beginGroup("Camera");
	config.setValue(QString("Width"), ui->parameter_tree->topLevelItem(0)->text(1));
	config.setValue(QString("Height"), ui->parameter_tree->topLevelItem(1)->text(1));
	config.setValue(QString("Frame rate"), ui->parameter_tree->topLevelItem(2)->text(1));
	config.setValue(QString("Exposure time"), ui->parameter_tree->topLevelItem(3)->text(1));
	config.setValue(QString("Trigger mode"), synchro_box->currentText());
	config.setValue(QString("Polarity"), polarity_box->currentText());
	config.setValue(QString("ROI_x"), ui->parameter_tree->topLevelItem(6)->child(0)->text(1));
	config.setValue(QString("ROI_y"), ui->parameter_tree->topLevelItem(6)->child(1)->text(1));
	config.setValue(QString("ROI_w"), ui->parameter_tree->topLevelItem(6)->child(2)->text(1));
	config.setValue(QString("ROI_h"), ui->parameter_tree->topLevelItem(6)->child(3)->text(1));
	config.setValue(QString("Threshold"), ui->parameter_tree->topLevelItem(6)->child(4)->text(1));
	config.endGroup();
}
