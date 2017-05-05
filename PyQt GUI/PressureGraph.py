#!/usr/bin/python3
# -*- coding: utf-8 -*-
import random
from collections import deque
from PyQt5.QtChart import QChart, QLineSeries, QChartView, QValueAxis
from PyQt5.QtWidgets import QWidget, QApplication, QGridLayout, QPushButton
from PyQt5.QtGui import QPainter

class PressureGraph(QWidget):
    def __init__(self):
        super().__init__()
        self.t = range(0,150) #time or ticks 
        self.q = deque([0]*len(self.t)) #data stored in a queue
        self.initUI()


    def initUI(self):
        self.series = QLineSeries()

        self.chart = QChart()
        self.chart.legend().hide()
        self.chart.addSeries(self.series)

        self.axis_x = QValueAxis()
        self.axis_x.setTitleText("Ticks elapsed")
        self.axis_y = QValueAxis()
        self.axis_y.setTitleText("Pressure (cmH2O)")
        self.chart.setAxisX(self.axis_x, self.series)
        self.chart.setAxisY(self.axis_y, self.series)

        self.axis_x.setRange(0, 150)
        self.axis_y.setRange(2.0, 3.5)

        self.chart.setTitle("Atmospheric Pressure in CPAP Mask")

        self.chartView = QChartView(self.chart)
        self.chartView.setRenderHint(QPainter.Antialiasing)

        self.grid = QGridLayout()
        self.grid.addWidget(self.chartView)

        #self.test_btn = QPushButton("TEST")
        #self.grid.addWidget(self.test_btn)
        #self.test_btn.clicked.connect(lambda: self.update(random.randrange(0,180)))

        self.setLayout(self.grid)

        self.resize(1000,500)

    def update(self, newpoint):
        '''Update the chart with a new point'''

        #convert the pressure value from kPa to cmH20
        newpoint = newpoint * 10.197442889221
        
        #Append the new datum, pop the oldest data 
        #from the front if exceed length
        self.q.append(newpoint)
        if len(self.q) > len(self.t):
            self.q.popleft()

        #Construct a new series
        #For each t value, append data from the back (newest value)
        self.series.clear()
        
        for i in range(0, len(self.t)):
            if i > (len(self.q) - 1):
                temp = 0
            else:
                temp = self.q[len(self.t) - 1 - i]
            self.series.append(self.t[i],temp)
   
        self.chartView.update()


if __name__ == "__main__":
    import sys
    app = QApplication([sys.argv])
    graph = PressureGraph()
    graph.show()

    sys.exit(app.exec_())





