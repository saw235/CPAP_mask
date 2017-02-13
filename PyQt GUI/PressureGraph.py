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
        self.t = range(0,150)
        self.q = deque([0]*len(self.t))
        self.initUI()


    def initUI(self):
        self.series = QLineSeries()

        self.chart = QChart()
        self.chart.legend().hide()
        self.chart.addSeries(self.series)

        self.axis_x = QValueAxis()
        self.axis_y = QValueAxis()
        self.chart.setAxisX(self.axis_x, self.series)
        self.chart.setAxisY(self.axis_y, self.series)

        self.axis_x.setRange(0, 150)
        self.axis_y.setRange(0, 200)

        self.chart.setTitle("Atmospheric Pressure in CPAP Mask")

        self.chartView = QChartView(self.chart)
        self.chartView.setRenderHint(QPainter.Antialiasing)

        self.grid = QGridLayout()
        self.grid.addWidget(self.chartView)

        self.test_btn = QPushButton("TEST")
        self.grid.addWidget(self.test_btn)
        self.test_btn.clicked.connect(lambda: self.update(random.randrange(0,180)))

        self.setLayout(self.grid)

        self.resize(1000,500)

    def update(self, newpoint):

        self.q.append(newpoint)
        if len(self.q) > len(self.t):
            self.q.popleft()

        p_list = []
        for i in range(0, len(self.t)):
            
            if i > (len(self.q) - 1):
                temp = 0
            else:
                temp = self.q[len(self.t) - 1 - i]
            p_list.append((self.t[i], temp))

        print(p_list)
        self.series.clear()         
        for i in p_list:
            self.series.append(*i)
        
        self.chartView.update()


if __name__ == "__main__":
    import sys
    app = QApplication([sys.argv])
    graph = PressureGraph()
    graph.show()

    sys.exit(app.exec_())





