#!/usr/bin/python3
# -*- coding: utf-8 -*-
import sys
import struct
from PyQt5.QtCore import Qt, QByteArray
from PyQt5.QtSerialPort import QSerialPortInfo, QSerialPort
from PyQt5.QtWidgets import (QMainWindow, QWidget, QPushButton, 
    QComboBox, QApplication, QGridLayout)


class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.initUI()
        self.initSlotSignals()

    def initUI(self):

        ##instantiate a connect widget
        self.connectwidget = ConnectWidget()
        self.setCentralWidget(self.connectwidget)


        self.setGeometry(300, 300, 300, 200)
        self.setWindowTitle("TESTGUI")
        self.show()


    def initSlotSignals(self):

        #make an anonymous slot function with showMessage with modified message passed to it
        #connects the slot to the clicked signals

        self.connectwidget.b_conn.clicked.connect(lambda: self.statusBar().showMessage(self.getStatusBarMsg()))
        self.connectwidget.b_disconn.clicked.connect(lambda: self.statusBar().showMessage(self.getStatusBarMsg()))



    def getStatusBarMsg(self):
        conn_stat = self.connectwidget.getConnectionStatus()

        port = self.connectwidget.getConnectedPort()

        msg = ''
        if (conn_stat):
            msg = "Connected to port " + port.portName() + '.'
        else:
            msg = "Disconnected."

        return msg
########SLOTS#####################################################




### Widget Container 
### Description : Widget to contain every other widget
#class WidgetContainer:




### MaskWidget
### Description : Widget to visualize the CPAP mask






#### ConnectWidget
#### Description : Widget to connect to com ports #######
class ConnectWidget(QWidget):
    def __init__(self):
        super().__init__()
        
        self.initUI()
        self.initElements()

        #connect signals to slots
        self.b_refresh.clicked.connect(self.populateComboBox)
        self.b_conn.clicked.connect(self.connectPort)
        self.b_disconn.clicked.connect(self.disconnectPort)
        self.connected_port.error.connect(self.serial_errorHandler)

    def initUI(self):
        #set up buttons
        self.b_conn = QPushButton("Connect")
        self.b_disconn = QPushButton("Disconnect")
        self.b_refresh = QPushButton("Refresh")
        

        #set up combo box for list of box
        self.port_cb = QComboBox()
        #populate combo box with port list
        self.populateComboBox()

        #set up a grid layout
        grid = QGridLayout()
        
        #grid.setSpacing(10)

        #add buttons and combobox
        grid.addWidget(self.port_cb,0,0, Qt.AlignTop | Qt.AlignLeft)
        grid.addWidget(self.b_refresh,0,1, Qt.AlignTop | Qt.AlignLeft)
        grid.addWidget(self.b_conn,0,2, Qt.AlignTop | Qt.AlignRight)
        grid.addWidget(self.b_disconn,0,3, Qt.AlignTop | Qt.AlignRight)

        self.setLayout(grid)
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
    def disconnectPort(self):
        self.connected_port.close()
        if (self.connected_port.error() == QSerialPort.NoError):
            print("Successfully closed port %s" % self.connected_port.portName())
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
            print("successfully connected to %s" % ser.portName())
            self.connected_port = ser
            self.connected = True
        else :
            print("failed to connect to %s" % ser.portName())

    ##NOT IMPLEMENTED YET#########
    def serial_errorHandler(self, error):
        print("Error occured in serial communcations. Error Code : %s" % error)


#####end SLOTS#############################

if __name__ == '__main__':
    app = QApplication(sys.argv)
    gui = MainWindow()
    app.exec_()