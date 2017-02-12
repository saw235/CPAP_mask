#!/usr/bin/python3
# -*- coding: utf-8 -*-

import sys
import SensorChart as sC

from PyQt5.QtCore import Qt, QByteArray, pyqtSignal
from PyQt5.QtGui import QPixmap, QPainter, QColor
from PyQt5.QtSerialPort import QSerialPortInfo, QSerialPort
from PyQt5.QtWidgets import (QMainWindow, QWidget, QPushButton, 
    QComboBox, QApplication, QGridLayout, QLabel, QHBoxLayout, QVBoxLayout)


### MaskWidget
### Description : Widget to visualize the CPAP mask
class MaskInfoWidget(QWidget):


	list_access_error = pyqtSignal()

	def __init__(self):
		super().__init__()

		self.samples = [0]*16
		self.initUI()
		self.initSlotSignals()

	
	def initUI(self):

		self.maskimg = QPixmap('Img\mask_layover.jpg')
		self.masklbl = QLabel()
		self.masklbl.setPixmap(self.maskimg)

		self.schart = sC.SensorChart(self.samples)

		self.scale_btn = QPushButton("Scale")
		self.calibrate_btn = QPushButton("Calibrate")

		btn_hbox = QHBoxLayout()
		btn_hbox.addWidget(self.scale_btn)
		btn_hbox.addWidget(self.calibrate_btn)

		vbox = QVBoxLayout()
		vbox.addWidget(self.schart)
		vbox.addLayout(btn_hbox)

		hbox = QHBoxLayout()
		hbox.addLayout(vbox)
		hbox.addWidget(self.masklbl)
		
		#hbox.insertWidget(0, masklbl)
		self.setLayout(hbox)
		self.resize(1000,400)

		
		self.setLayout(hbox)
		self.show()

	
	def initSlotSignals(self):
		pass

	#A paintEvent starts when the window regain focus
	def paintEvent(self,event):

		#Make a painter object
		qp = QPainter(self.maskimg)

		#start painting
		self.drawSensorCircles(qp)
		self.masklbl.setPixmap(self.maskimg)


	#drawSensorCircles()
	#Parameters: qp - QPainter
	#            data - array of 16 data

	def drawSensorCircles(self,qp):
		
		color = QColor(0,0,0)
		color.setNamedColor('#d4d4d4')
		qp.setPen(color)

		#randnum = [random.randrange(0,255), random.randrange(0, 255), random.randrange(0, 255)]
		#randcolor = QColor(255,255,0)

		sample_col = list(map(lambda x: QColor(*self.value2color(x)) , self.samples))

		
		if len(sample_col) != 16 :
			self.list_access_error.emit()
			return
		#Left side Sensor Circles
		qp.setBrush(sample_col[0])
		qp.drawEllipse(125,60,30,30) #Sensor x
		qp.setBrush(sample_col[1])
		qp.drawEllipse(110,110,30,30) #Sensor x
		qp.setBrush(sample_col[2])
		qp.drawEllipse(95,160,30,30) #Sensor x
		qp.setBrush(sample_col[3])
		qp.drawEllipse(65,200,30,30) #Sensor x
		qp.setBrush(sample_col[4])
		qp.drawEllipse(30,250,30,30) #Sensor x
		qp.setBrush(sample_col[5])
		qp.drawEllipse(15,290,30,30) #Sensor x


		#Right side Sensor Circles
		qp.setBrush(sample_col[6])
		qp.drawEllipse(210,60,30,30) #Sensor x 
		qp.setBrush(sample_col[7])
		qp.drawEllipse(225,110,30,30) #Sensor x
		qp.setBrush(sample_col[8])
		qp.drawEllipse(240,160,30,30) #Sensor x
		qp.setBrush(sample_col[9])
		qp.drawEllipse(270,200,30,30) #Sensor x
		qp.setBrush(sample_col[10])
		qp.drawEllipse(300,250,30,30) #Sensor x
		qp.setBrush(sample_col[11])
		qp.drawEllipse(315,290,30,30) #Sensor x

		#Bot side Sensor Circles
		qp.setBrush(sample_col[12])
		qp.drawEllipse(30,370,30,30) #Sensor x
		qp.setBrush(sample_col[13])
		qp.drawEllipse(120,395,30,30) #Sensor x
		qp.setBrush(sample_col[14])
		qp.drawEllipse(220,395,30,30) #Sensor x
		qp.setBrush(sample_col[15])
		qp.drawEllipse(310,370,30,30) #Sensor x


	def updateSamples(self, arr):
		self.samples = arr

	### Get Methods ###
	def getChart(self):
		return self.schart


	#value2color()
	#takes in a value between 0 to 1000 and converts to
	#a color between red and green

	#refer to "Color Gradient Guide.png" 
	#Credit to m00lti from stackoverflow.com
	@staticmethod
	def value2color(val):

		SCALE_CONSTANT = 2500

		scaled = int(val*510/SCALE_CONSTANT)
		
		if scaled <= 255 :
			r = 255
			g = scaled
		else :
			r = 510 - scaled
			g = 255

		return (r,g,0)


if __name__ == '__main__':
	app = QApplication(sys.argv)
	gui = MaskInfoWidget()
	sys.exit(app.exec_())