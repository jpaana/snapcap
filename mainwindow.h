/*
 Copyright 2017 Jarno Paananen

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
*/
  
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QTimer>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_connectButton_clicked();

    void on_openButton_clicked();

    void on_closeButton_clicked();

    void on_onButton_clicked();

    void on_offButton_clicked();

    void on_forceOpenButton_clicked();

    void on_forceCloseButton_clicked();

    void on_abortButton_clicked();

    void on_brightnessSlider_valueChanged(int value);

    void on_actionQuit_triggered();

    void handleTimeout();
    void handleError(QSerialPort::SerialPortError error);

private:
    QString sendCommand(char cmd, int value = 0);
    void startTimer();

    Ui::MainWindow *ui;

    bool connected;
    char coverStatus;
    char lightStatus;
    char motorStatus;
    int brightness;
    QSerialPort port;
    QTimer timer;

    void updateStatus();
    void readStatus();
    void readBrightness();

    std::vector<char> commandsSent;
};

#endif // MAINWINDOW_H
