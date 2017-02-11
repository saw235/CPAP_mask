#!/usr/bin/python3
# -*- coding: utf-8 -*-

import random
import sys
import struct
from PyQt5.QtCore import Qt, QByteArray
from PyQt5.QtGui import QPixmap, QPainter, QColor
from PyQt5.QtSerialPort import QSerialPortInfo, QSerialPort
from PyQt5.QtWidgets import (QMainWindow, QWidget, QPushButton, 
    QComboBox, QApplication, QGridLayout, QLabel, QHBoxLayout)


### MaskWidget
### Description : Widget to visualize the CPAP mask
class MaskInfoWidget(QWidget):
	

	def __init__(self):
		super().__init__()
		self.initUI()
		self.initSlotSignals()

	
	def initUI(self):

		self.maskimg = QPixmap('Img\mask_layover.jpg')
		self.masklbl = QLabel()
		self.masklbl.setPixmap(self.maskimg)

		hbox = QHBoxLayout()
		hbox.addStretch(1)
		
		hbox.addWidget(self.masklbl)

		hbox.insertSpacing(0, 600)
		#hbox.insertWidget(0, masklbl)

		
		self.setLayout(hbox)
		self.show()

	
	def initSlotSignals(self):
		pass

	#A paintEvent starts when the window regain focus
	def paintEvent(self,event):

		#Make a painter object
		qp = QPainter(self.maskimg)

		#start painting
		#qp.begin(self)
		self.drawSensorCircles(qp)
		self.masklbl.setPixmap(self.maskimg)
		#qp.end()

	#drawSensorCircles()
	#Parameters: qp - QPainter
	#            data - array of 16 data

	def drawSensorCircles(self,qp):
		
		color = QColor(0,0,0)
		color.setNamedColor('#d4d4d4')
		qp.setPen(color)

		randnum = [random.randrange(0,255), random.randrange(0, 255), random.randrange(0, 255)]
		randcolor = QColor(255,255,0)

		test_val = 800
		test_col = QColor(*self.value2color(test_val))

		qp.setBrush(test_col)

		#Left side Sensor Circles
		qp.drawEllipse(125,60,30,30) #Sensor x
		qp.drawEllipse(110,110,30,30) #Sensor x
		qp.drawEllipse(95,160,30,30) #Sensor x
		qp.drawEllipse(65,200,30,30) #Sensor x
		qp.drawEllipse(30,250,30,30) #Sensor x
		qp.drawEllipse(15,290,30,30) #Sensor x


		#Right side Sensor Circles
		qp.drawEllipse(210,60,30,30) #Sensor x 
		qp.drawEllipse(225,110,30,30) #Sensor x
		qp.drawEllipse(240,160,30,30) #Sensor x
		qp.drawEllipse(270,200,30,30) #Sensor x
		qp.drawEllipse(300,250,30,30) #Sensor x
		qp.drawEllipse(315,290,30,30) #Sensor x

		#Bot side Sensor Circles
		qp.drawEllipse(30,370,30,30) #Sensor x
		qp.drawEllipse(120,395,30,30) #Sensor x
		qp.drawEllipse(220,395,30,30) #Sensor x
		qp.drawEllipse(310,370,30,30) #Sensor x


	#takes in a value between 0 to 1000 and converts to
	#a color between red and green

	#refer to "Color Gradient Guide.png" 
	#Credit to m00lti from stackoverflow.com
	@staticmethod
	def value2color(val):

		scaled = int(val*510/1000)
		
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