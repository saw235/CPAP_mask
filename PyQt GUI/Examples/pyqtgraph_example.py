from PyQt5.QtChart import QChart, QBarSet, QBarSeries, QBarCategoryAxis, QChartView
from PyQt5.QtWidgets import QMainWindow
from PyQt5.QtGui import QPixmap, QPainter, QColor



if __name__ == '__main__':
    import sys
    from PyQt5.QtWidgets import QApplication
    from PyQt5.QtCore import Qt
    app = QApplication(sys.argv)

    set0 = QBarSet("Sensors")
    set0.append(100)
    set0.append(200)
    set0.append(300)

    series = QBarSeries()
    series.append(set0)

    chart = QChart()
    chart.addSeries(series)
    chart.setTitle("Simple Barchart")


    cat = ["S1", "S2", "S3"]
    axis = QBarCategoryAxis()

    axis.append(cat)
    chart.createDefaultAxes()
    chart.setAxisX(axis, series)

    chartView = QChartView(chart)
    chartView.setRenderHint(QPainter.Antialiasing)

    w = QMainWindow()

    w.setCentralWidget(chartView)
    w.show()
    sys.exit(app.exec_())
