#!/usr/bin/python3
# -*- coding: utf-8 -*-
import time
import sys
import struct
from SampleFetcher import SampleFetcher
from PyQt5.QtCore import QThread, pyqtSignal, QMutex
from PyQt5.QtWidgets import QApplication
from PyQt5.QtSerialPort import QSerialPortInfo, QSerialPort
    

def PrintSample(fetcher):
    sample = fetcher.getSamples()
    print(sample)


def main():

    app = QApplication(sys.argv)

    qsPort = QSerialPort()
    qsPort.setPortName('COM5')
    qsPort.setBaudRate(QSerialPort.Baud115200)
    connect = False

    if (qsPort.open(QSerialPort.ReadOnly | QSerialPort.WriteOnly)):
        print("successfully connected to %s" % qsPort.portName())
        connect = True
    else :
        print("failed to connect to %s" % qsPort.portName())


    time.sleep(4)
    if connect:

        fetcher = SampleFetcher(qsPort)
        qthread = QThread()

        qthread.started.connect(fetcher.process)
        fetcher.doneSampling.connect(lambda : PrintSample(fetcher))

        fetcher.moveToThread(qthread)
        qthread.start()


    app.exec_()

if __name__ == '__main__':
    main()