import sys
import struct
import time
from PyQt5.QtCore import pyqtSignal, QObject, QThread
from PyQt5.QtSerialPort import QSerialPortInfo, QSerialPort
from PyQt5.QtWidgets import QApplication


class SerialReadHandler(QObject):
    def __init__(self, _qserialport=QSerialPort()):
        super().__init__()
        self.port = _qserialport

        #self.initSlotSignals()

    dataReady = pyqtSignal()

    def handleReadyRead(self):
        EXPECTED_BYTES = 33
        if (self.port.bytesAvailable() < EXPECTED_BYTES):
            return
        self.data = self.port.readAll()

        self.sample = struct.unpack_from(">c16H", self.data, 0)

        print(self.sample[0])
        if (not((self.sample[0] == b'\x12') or (self.sample[0] == b'\x13'))):
            return

        self.dataReady.emit()

    def setSerialPort(self, _qserialport):
        self.port = _qserialport

    def getDataRead(self):
        return self.data





if __name__ == '__main__':
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
        rHandler = SerialReadHandler(qsPort)
        
    app.exec_()