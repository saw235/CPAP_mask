#!/usr/bin/python3
# -*- coding: utf-8 -*-
import sys
from PyQt5.QtCore import QObject, QThread, pyqtSignal
from PyQt5.QtWidgets import QApplication

class Worker(QObject):

	def __init__(self):
		super().__init__()



### SIGNALS ###
	finished = pyqtSignal()
	error = pyqtSignal()



### SLOTS ###
	def process(self):
		print("Hello World!")
		self.finished.emit()


	def test_finished(self):
		print("Finish signal emmited")

if __name__ == '__main__':

	app = QApplication(sys.argv)
	qthread = QThread()
	w = Worker()

	w.moveToThread(qthread)

	qthread.started.connect(w.process)
	w.finished.connect(app.quit)

	qthread.start()
	app.exec_()