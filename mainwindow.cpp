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
  
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSerialPortInfo>
#include <QString>
#include <QDebug>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Enumerate serial ports
    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
    for(auto port: ports)
    {
        QString name = port.portName();
        ui->serialPort->addItem(name);
    }
    ui->openButton->setDisabled(true);
    ui->closeButton->setDisabled(true);
    ui->onButton->setDisabled(true);
    ui->offButton->setDisabled(true);
    ui->brightnessSlider->setDisabled(true);
    ui->forceOpenButton->setDisabled(true);
    ui->forceCloseButton->setDisabled(true);
    ui->abortButton->setDisabled(true);

    ui->coverStatus->setDisabled(true);
    ui->lightStatus->setDisabled(true);
    ui->brightnessText->setDisabled(true);

    ui->coverStatus->setText("");
    ui->lightStatus->setText("");
    ui->brightnessText->setText("");
    ui->connectionStatus->setText("Disconnected");

    coverStatus = false;
    lightStatus = false;
    brightness = 255;

    connected = false;

    timer.setSingleShot(true);

    connect(&port, static_cast<void (QSerialPort::*)(QSerialPort::SerialPortError)>(&QSerialPort::error),
            this, &MainWindow::handleError);
    connect(&timer, &QTimer::timeout, this, &MainWindow::handleTimeout);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_connectButton_clicked()
{
    if(connected)
    {
        port.close();
        ui->connectButton->setText("Connect");

        ui->openButton->setDisabled(true);
        ui->onButton->setDisabled(true);
        ui->brightnessSlider->setDisabled(true);
        ui->forceOpenButton->setDisabled(true);
        ui->forceCloseButton->setDisabled(true);
        ui->abortButton->setDisabled(true);
        ui->coverStatus->setDisabled(true);
        ui->lightStatus->setDisabled(true);
        ui->brightnessText->setDisabled(true);
        ui->connectionStatus->setText("Disconnected");
        connected = false;
        return;
    }
    port.setPortName(ui->serialPort->currentText());
    port.setBaudRate(QSerialPort::Baud38400);
    if(port.open(QIODevice::ReadWrite))
    {
        // Check that we have proper device
        QString response = sendCommand('V');
        if (response[0] == '*' && response[1] == 'V')
        {
            ui->connectButton->setText("Disconnect");

            ui->openButton->setDisabled(false);
            ui->onButton->setDisabled(false);
            ui->brightnessSlider->setDisabled(false);
            ui->forceOpenButton->setDisabled(false);
            ui->forceCloseButton->setDisabled(false);
            ui->abortButton->setDisabled(false);
            ui->coverStatus->setDisabled(false);
            ui->lightStatus->setDisabled(false);
            ui->brightnessText->setDisabled(false);

            readStatus();
            readBrightness();
            updateStatus();
            connected = true;
            ui->connectionStatus->setText("Connected to SnapCap "+response.mid(1, 4));
        }
        else
        {
            port.close();
            QMessageBox msg;
            msg.setText("Invalid response from device, is the port correct?");
            msg.exec();
        }
   }
}

void MainWindow::startTimer()
{
    timer.setSingleShot(true);
    timer.start(1000);
}

QString MainWindow::sendCommand(char cmd, int value)
{
    QString str = QString::asprintf(">%c%03d\r\n", cmd, value);
    port.clear();
    port.write(str.toLatin1());
    port.flush();
    QByteArray response;
    while(response.size() != 7)
    {
        port.waitForReadyRead(1000);
        response.append(port.readAll());
    }
    //qDebug() << response;
    if (response[0] != '*' || response[1] != cmd)
    {
        QMessageBox msg;
        msg.setText("Invalid response to command!");
        msg.exec();
    }
    return QString(response);
}

void MainWindow::on_openButton_clicked()
{
    sendCommand('O');
    startTimer();
}

void MainWindow::on_closeButton_clicked()
{
    sendCommand('C');
    startTimer();
}

void MainWindow::on_onButton_clicked()
{
    sendCommand('L');
    readStatus();
    updateStatus();
}

void MainWindow::on_offButton_clicked()
{
    sendCommand('D');
    readStatus();
    updateStatus();
}

void MainWindow::on_forceOpenButton_clicked()
{
    sendCommand('o');
    startTimer();
}

void MainWindow::on_forceCloseButton_clicked()
{
    sendCommand('c');
    startTimer();
}

void MainWindow::on_abortButton_clicked()
{
    sendCommand('A');
    readStatus();
    updateStatus();
}

void MainWindow::readStatus()
{
    QString response = sendCommand('S');
    motorStatus = response.mid(2, 1).toInt();
    lightStatus = response.mid(3, 1).toInt();
    coverStatus = response.mid(4, 1).toInt();
}

void MainWindow::readBrightness()
{
    QString response = sendCommand('J');
    brightness = response.mid(2, 3).toInt();
    ui->brightnessSlider->setValue(brightness * 100 / 255);
    ui->brightnessText->setText(QString::number((brightness * 100 + 127) / 255) + "%");
}

void MainWindow::updateStatus()
{
    if (motorStatus == 1)
    {
        ui->coverStatus->setText("Moving");
        ui->openButton->setDisabled(true);
        ui->closeButton->setDisabled(true);
        startTimer();
    }
    else
    {
        switch(coverStatus)
        {
        default:
            ui->coverStatus->setText("Unknown");
            ui->openButton->setDisabled(true);
            ui->closeButton->setDisabled(true);
            break;
        case 1:
            ui->coverStatus->setText("Open");
            ui->openButton->setDisabled(true);
            ui->closeButton->setDisabled(false);
            break;
        case 2:
            ui->coverStatus->setText("Closed");
            ui->openButton->setDisabled(false);
            ui->closeButton->setDisabled(true);
            break;
        case 3:
            ui->coverStatus->setText("Timeout");
            ui->openButton->setDisabled(false);
            ui->closeButton->setDisabled(false);
            break;
        case 4:
            ui->coverStatus->setText("Open circuit");
            ui->openButton->setDisabled(false);
            ui->closeButton->setDisabled(false);
            break;
        case 5:
            ui->coverStatus->setText("Overcurrent");
            ui->openButton->setDisabled(true);
            ui->closeButton->setDisabled(true);
            break;
        case 6:
            ui->coverStatus->setText("User abort");
            ui->openButton->setDisabled(false);
            ui->closeButton->setDisabled(false);
            break;
        }
    }
    if(lightStatus == 1)
    {
        ui->lightStatus->setText("On");
        ui->onButton->setDisabled(true);
        ui->offButton->setDisabled(false);
    }
    else
    {
        ui->lightStatus->setText("Off");
        ui->onButton->setDisabled(false);
        ui->offButton->setDisabled(true);
    }
}

void MainWindow::on_brightnessSlider_valueChanged(int value)
{
    brightness = value * 255 / 100;
    sendCommand('B', brightness);
    ui->brightnessText->setText(QString::number((brightness * 100  + 127) / 255) + "%");
}

void MainWindow::on_actionQuit_triggered()
{
    QApplication::quit();
}

void MainWindow::handleTimeout()
{
    readStatus();
    updateStatus();
}

void MainWindow::handleError(QSerialPort::SerialPortError error)
{
    if (error != 0)
    {
        qDebug() << "HandleError() " << error;
    }
}
