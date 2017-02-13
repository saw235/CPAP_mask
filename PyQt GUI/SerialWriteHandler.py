from PyQt5.QtCore import pyqtSignal, QObject
from PyQt5.QtSerialPort import QSerialPortInfo, QSerialPort
from PyQt5.QtWidgets import QApplication


class SerialWriteHandler(QObject):
    def __init__(self, _qserialport=QSerialPort()):
        super().__init__()
        self.port = _qserialport
        self.initSlotSignals()

    doneRequestScale = pyqtSignal()
    doneRequestCalibration = pyqtSignal()
    
    def initSlotSignals(self):
        pass

    def requestScale(self):
        self.port.writeData(bytes([0x27]))
        self.doneRequestScale.emit()

    def requestCalibration(self):
        self.port.writeData(bytes([0x26]))
        self.doneRequestCalibration.emit()