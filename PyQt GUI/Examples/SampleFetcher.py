#!/usr/bin/python3
# -*- coding: utf-8 -*-
import time
import sys
import struct
from PyQt5.QtCore import Qt, QByteArray, QTimer, pyqtSignal, QObject
from PyQt5.QtSerialPort import QSerialPortInfo, QSerialPort
from PyQt5.QtWidgets import (QMainWindow, QWidget, QPushButton, 
    QComboBox, QApplication, QGridLayout)


### SampleFetcher
### Description : class to retrieve serial data from Arduino
class SampleFetcher(QObject):
    def __init__(self, _qserialport):
        super().__init__()
        #get the port object
        self.port = _qserialport
        self.samples = [0]*16        
        self.initSlotSignals()




    finished = pyqtSignal()
    doneSampling = pyqtSignal()
    
    def initSlotSignals(self):
        pass

    #function to send a request
    def requestData(self):
         
        #clears the incoming buffer 
        self.port.clear()
        
        #send a request request
        self.port.writeData(bytes([0x28]))

    def beginSampleRoutine(self):

        #to get data from arduino start by sending a request
        self.requestData()

        #wait for incoming data in serialport
        print("waitingforReadyRead")
        ret = self.port.waitForReadyRead(3000)
        print("ReadyRead")
        #if timed out. ie: function return false
        if (not ret) : 
            return

        #else read and process data
        

        #make an empty list
        arr = []
        
        #ready while byte is coming in
        while self.port.bytesAvailable():

            #read 1 byte and append it to list
            qbA = self.port.read(1)
            arr.append(qbA)

        #return array
        self.samples = self.reconstructSample(arr)

        #signal process done
        self.doneSampling.emit()


    #Continuously polls for sensor data
    def process(self):
        while True:
            self.beginSampleRoutine()

    @staticmethod
    def reconstructSample(arr):

        #return empty list if size is incorrect
        if (len(arr) != 32):
            return []


        #else unpack the data into unsigned short
        ret_arr = []
        for i in range(0, 31, 2):
            temp = struct.unpack(">H", arr[i] + arr[i+1])[0]
            ret_arr.append(temp)

        return ret_arr

### GetSet Methods ###
    def getSamples(self):
        return self.samples



if __name__ == '__main__':

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
        fetcher.beginSampleRoutine()
        samples = fetcher.getSamples()
        
        print(samples)
    

    qsPort.close()
