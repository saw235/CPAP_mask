#!/usr/bin/python3
# -*- coding: utf-8 -*-

from SerialReadHandler import SerialReadHandler
from PyQt5.QtCore import pyqtSignal, QObject, QThread
from PyQt5.QtSerialPort import QSerialPortInfo, QSerialPort
from PyQt5.QtWidgets import QApplication

class SerialReadWorker(QThread):
	def __init__(self, _qserialport):
		super().__init__()
		self.port = _qserialport
		self.read_handler = SerialReadHandler(self.port)


	#finished = pyqtSignal()
	def run(self):
		self.port.readyRead.connect(self.read_handler.handleReadyRead)


	def getReadHandler(self):
		return self.read_handler