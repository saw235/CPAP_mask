#!/usr/bin/python3
# -*- coding: utf-8 -*-

from PyQt5.QtChart import QChart, QBarSet, QBarSeries, QBarCategoryAxis, QChartView, QValueAxis
from PyQt5.QtWidgets import QWidget, QApplication, QGridLayout
from PyQt5.QtGui import QPixmap, QPainter, QColor


class SensorChart(QWidget):

    #constructor
    def __init__(self, sample):
        super().__init__()
        self.samples = sample
        self.initUI()

    def initUI(self):

        self.set0 = QBarSet("Sensors")

        for sample in self.samples:
            self.set0.append(sample)

        self.series = QBarSeries()
        self.series.append(self.set0)

        self.chart = QChart()
        self.chart.setTitle("Sensor Values")
        self.chart.addSeries(self.series)
        self.categories = []

        count = 1
        for sample in self.samples:
            self.categories.append("S" + str(count))
            count += 1

        self.axis_x = QBarCategoryAxis()
        self.axis_x.append(self.categories)

        self.chart.setAxisX(self.axis_x, self.series)

        self.axis_y = QValueAxis()
        self.chart.setAxisY(self.axis_y, self.series)
        self.axis_y.setRange(0,2500)

        self.chartView = QChartView(self.chart)
        self.chartView.setRenderHint(QPainter.Antialiasing)

        self.grid = QGridLayout()
        self.grid.addWidget(self.chartView)
        self.setLayout(self.grid)

        #self.resize(800,500)

    ## SLOTS ##
    def updateSeries(self, samples):

        #create a new bar set
        self.set_update = QBarSet("Sensors")

        for sample in samples:
            self.set_update.append(sample)    

        # Get the series from charview
        self.series.clear()
        self.series.append(self.set_update)
        self.chartView.update()

if __name__ == '__main__':
    import sys
    app = QApplication(sys.argv)
    sample = [100]*16
    w = SensorChart(sample)
    w.show()
    sys.exit(app.exec_())
