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
		self.area_stat = [False]*5;
		self.SCALE_CONSTANT = 2500;
		self.initUI()
		self.initSlotSignals()

	
	def initUI(self):

		self.maskimg = QPixmap('Img\mask_layover.jpg')
		self.masklbl = QLabel()
		self.masklbl.setPixmap(self.maskimg)
		self.maskstatlbl = QLabel("")
		self.schart = sC.SensorChart(self.samples)

		self.scale_btn = QPushButton("Scale")
		self.calibrate_btn = QPushButton("Calibrate")

		btn_hbox = QHBoxLayout()
		btn_hbox.addWidget(self.scale_btn)
		btn_hbox.addWidget(self.calibrate_btn)


		vbox = QVBoxLayout()
		vbox.addWidget(self.schart)
		vbox.addLayout(btn_hbox)

		status_hbox = QHBoxLayout()
		status_hbox.addWidget(self.maskstatlbl)
		status_hbox.insertSpacing(0,150)

		vbox2 = QVBoxLayout()
		vbox2.addWidget(self.masklbl)
		vbox2.addLayout(status_hbox)

		hbox = QHBoxLayout()
		hbox.addLayout(vbox)
		hbox.addLayout(vbox2)

		
		#hbox.insertWidget(0, masklbl)
		self.setLayout(hbox)
		self.resize(1000,400)

		
		self.setLayout(hbox)
		self.show()

	
	def initSlotSignals(self):
		pass

		#self.scale_btn.clicked.connect(self.scaleToggle)



	#A paintEvent starts when the window regain focus
	def paintEvent(self,event):

		#Make a painter object
		qp = QPainter(self.maskimg)

		#start painting
		self.drawSensorCircles(qp)
		self.drawAreaColor(qp)
		self.masklbl.setPixmap(self.maskimg)


	#drawAreaColor()
	#Parameters: qp - QPainter
	def drawAreaColor(self,qp):
		self.area_stat_col = list(map(lambda x: QColor(*self.areastat2color(x)),self.area_stat))

		qp.setBrush(self.area_stat_col[0])
		qp.drawRect(10,20,10,10)
		qp.drawText(25,30,"Upper Right Area")

		qp.setBrush(self.area_stat_col[1])
		qp.drawRect(10,35,10,10)
		qp.drawText(25,45,"Lower Right Area")

		qp.setBrush(self.area_stat_col[2])
		qp.drawRect(10,50,10,10)
		qp.drawText(25,60,"Upper Left Area")

		qp.setBrush(self.area_stat_col[3])
		qp.drawRect(10,65,10,10)
		qp.drawText(25,75,"Lower Left Area")

		qp.setBrush(self.area_stat_col[4])
		qp.drawRect(10,80,10,10)
		qp.drawText(25,90,"Bottom Area")


	#drawSensorCircles()
	#Parameters: qp - QPainter
	def drawSensorCircles(self,qp):
		
		#color = QColor(0,0,0)
		#color.setNamedColor('#d4d4d4')
		#qp.setPen(color)

		#randnum = [random.randrange(0,255), random.randrange(0, 255), random.randrange(0, 255)]
		#randcolor = QColor(255,255,0)

		sample_col = list(map(lambda x: QColor(*self.value2color(x,self.SCALE_CONSTANT)) , self.samples))

		
		if len(sample_col) != 16 :
			self.list_access_error.emit()
			return
		#Left side Sensor Circles
		qp.setBrush(sample_col[10])
		qp.drawEllipse(125,60,30,30) #Sensor 11
		qp.setBrush(sample_col[11])
		qp.drawEllipse(110,110,30,30) #Sensor 12
		qp.setBrush(sample_col[8])
		qp.drawEllipse(95,160,30,30) #Sensor 9
		qp.setBrush(sample_col[9])
		qp.drawEllipse(65,200,30,30) #Sensor 10
		qp.setBrush(sample_col[6])
		qp.drawEllipse(30,250,30,30) #Sensor 7
		qp.setBrush(sample_col[7])
		qp.drawEllipse(15,290,30,30) #Sensor 8


		#Right side Sensor Circles
		qp.setBrush(sample_col[1])
		qp.drawEllipse(210,60,30,30) #Sensor 2 
		qp.setBrush(sample_col[0])
		qp.drawEllipse(225,110,30,30) #Sensor 1
		qp.setBrush(sample_col[3])
		qp.drawEllipse(240,160,30,30) #Sensor 4
		qp.setBrush(sample_col[2])
		qp.drawEllipse(270,200,30,30) #Sensor 3
		qp.setBrush(sample_col[5])
		qp.drawEllipse(300,250,30,30) #Sensor 6
		qp.setBrush(sample_col[4])
		qp.drawEllipse(315,290,30,30) #Sensor 5

		#Bot side Sensor Circles
		qp.setBrush(sample_col[13])
		qp.drawEllipse(30,370,30,30) #Sensor 14
		qp.setBrush(sample_col[12])
		qp.drawEllipse(120,395,30,30) #Sensor 13
		qp.setBrush(sample_col[15])
		qp.drawEllipse(220,395,30,30) #Sensor 16
		qp.setBrush(sample_col[14])
		qp.drawEllipse(310,370,30,30) #Sensor 15


	def updateSamples(self, arr):
		self.samples = arr
		self.schart.updateSeries(self.samples)

	def updateAreaStat(self, arr):
		self.area_stat = arr

	### Get Methods ###
	def getChart(self):
		return self.schart


	#value2color()
	#takes in a value between 0 to 1000 and converts to
	#a color between red and green

	#refer to "Color Gradient Guide.png" 
	#Credit to m00lti from stackoverflow.com
	@staticmethod
	def value2color(val, SCALE_CONSTANT):

		scaled = int(val * 510 / SCALE_CONSTANT)
		
		if (scaled <= 255 and scaled >= 0):
			r = 255
			g = scaled
		elif (scaled <= 510 and scaled > 255):
			r = 510 - scaled
			g = 255
		else:
			r = 255
			g = 0

		return (r,g,0)

	@staticmethod
	def areastat2color(val):
		if val:
			#return Green if true
			return (0,255,0)
		else:
			return (255,0,0)


	def scaleEnable(self):
		self.SCALE_CONSTANT = 1000
		self.schart.enableScale()
		self.scale_btn.setText("Unscale")

	def scaleDisable(self):
		self.SCALE_CONSTANT = 2800
		self.schart.disableScale()
		self.scale_btn.setText("Scale")



if __name__ == '__main__':
	app = QApplication(sys.argv)
	gui = MaskInfoWidget()
	sys.exit(app.exec_())