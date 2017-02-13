#!/usr/bin/python3
# -*- coding: utf-8 -*-
import sys
from PyQt5.QtCore import Qt, pyqtSignal
from PyQt5.QtSerialPort import QSerialPortInfo, QSerialPort
from PyQt5.QtWidgets import (QWidget, QPushButton, QComboBox, QApplication, QHBoxLayout)

#### ConnectWidget
#### Description : Widget to connect to com ports #######
class ConnectWidget(QWidget):
    def __init__(self):
        super().__init__()
        
        self.initUI()
        self.initElements()

        #connect signals to slots
        self.b_refresh.clicked.connect(self.populateComboBox)
        self.b_conn.clicked.connect(self.connect_btn_handler)
        self.connected_port.error.connect(self.serial_errorHandler)

        self.connected_sig.connect(lambda: self.b_conn.setText("Disconnect"))
        self.disconnected_sig.connect(lambda: self.b_conn.setText("Connect"))

    ## Signals ##
    connected_sig = pyqtSignal()
    disconnected_sig = pyqtSignal()
    connect_fail_sig = pyqtSignal()

    def initUI(self):
        #set up buttons
        self.b_conn = QPushButton("Connect")
        self.b_refresh = QPushButton("Refresh")
        

        #set up combo box for list of box
        self.port_cb = QComboBox()
        #populate combo box with port list
        self.populateComboBox()

        #set up a grid layout
        hbox = QHBoxLayout()
        
        #grid.setSpacing(10)

        #add buttons and combobox
        hbox.addWidget(self.port_cb, Qt.AlignLeft)
        hbox.addWidget(self.b_refresh, Qt.AlignLeft)
        hbox.addWidget(self.b_conn, Qt.AlignLeft)

        self.setLayout(hbox)
        #self.show()

    ##initializes other variables to be used
    def initElements(self):
        self.connected_port = QSerialPort()
        self.connected = False

    #method to get all the available port list
    def getportlist(self):
        return QSerialPortInfo.availablePorts()


    #method to populate the combo box
    def populateComboBox(self):

        #clears the combo box
        self.port_cb.clear()

        #get the list of available ports
        l = self.getportlist()

        #populate the combobox
        for i in range(0, len(l)):
            self.port_cb.addItem(l[i].portName())

    #method to generate msg for status bar
    def StatusBarMsg(self):
        conn_stat = self.getConnectionStatus()
        port = self.getConnectedPort()

        msg = ''
        if (conn_stat):
            msg = "Connected to port " + port.portName() + '.'
        else:
            msg = "Disconnected."

        return msg

###GET SET METHODS#######################
###GET#####
    def getConnectionStatus(self):
        return self.connected

    def getConnectedPort(self):
        return self.connected_port
###SET####

#####SLOTS###############################
    #slot for disconnect button

    def connect_btn_handler(self):
        if self.connected:
            self.disconnectPort()

        elif not self.connected:
            self.connectPort()


    def disconnectPort(self):
        self.connected_port.close()
        if (self.connected_port.error() == QSerialPort.NoError):
            print("Disconnected %s" % self.connected_port.portName())
            self.disconnected_sig.emit()
            self.connected = False
        else:
            print("Unable to close the port. Error Code : %s" % self.connected_port.error())

    def connectPort(self):
        #Create a Serial Port Object
        ser = QSerialPort()

        #Set port name to the one selected in combo box, Baud Rate is presetted to 115200
        ser.setPortName(self.port_cb.currentText())
        ser.setBaudRate(QSerialPort.Baud115200)

        #Open serial port in Read and Write Mode
        if (ser.open(QSerialPort.ReadOnly | QSerialPort.WriteOnly)):
            print("Successfully connected to %s" % ser.portName())
            self.connected_port = ser
            self.connected = True

            #emit a signal
            self.connected_sig.emit()

        else :
            self.connect_fail_sig.emit()
            print("failed to connect to %s" % ser.portName())

    ##NOT IMPLEMENTED YET#########
    def serial_errorHandler(self, error):
        print("Error occured in serial communcations. Error Code : %s" % error)


#####end SLOTS#############################

if __name__ == '__main__':
    app = QApplication(sys.argv)
    wC = ConnectWidget()
    wC.show()
    app.exec_()