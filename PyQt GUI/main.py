#!/usr/bin/python3
# -*- coding: utf-8 -*-
import sys
from PyQt5.QtCore import Qt
from PyQt5.QtSerialPort import QSerialPortInfo, QSerialPort
from PyQt5.QtWidgets import (QMainWindow, QApplication)

from WidgetContainer import WidgetContainer


class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.initUI()
        self.initSlotSignals()

    def initUI(self):

        self.container = WidgetContainer()

        self.setCentralWidget(self.container)


        self.setGeometry(400, 200, 1000, 200)
        self.setWindowTitle("TESTGUI")
        self.show()


    def initSlotSignals(self):

        #make an anonymous slot function with showMessage with modified message passed to it
        #connects the slot to the clicked signals

        self.container.getConnectWidget().b_conn.clicked.connect(lambda: self.statusBar().showMessage(self.getStatusBarMsg()))



    def getStatusBarMsg(self):
        conn_stat = self.container.getConnectWidget().getConnectionStatus()

        port = self.container.getConnectWidget().getConnectedPort()

        msg = ''
        if (conn_stat):
            msg = "Connected to port " + port.portName() + '.'
        else:
            msg = "Disconnected."

        return msg


if __name__ == '__main__':
    app = QApplication(sys.argv)
    app.setStyle("fusion")
    gui = MainWindow()
    app.exec_()