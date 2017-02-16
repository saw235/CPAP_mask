#!/usr/bin/python3
# -*- coding: utf-8 -*-

from PyQt5.QtWidgets import *
from PyQt5.QtCore import QThread, pyqtSignal

from ConnectWidget import ConnectWidget
from MaskInfoWidget import MaskInfoWidget
from SerialReadWorker import SerialReadWorker
from SerialWriteHandler import SerialWriteHandler
from PressureGraph import PressureGraph

### class WidgetContainer: 
### Description : Widget to contain every other widget
class WidgetContainer(QWidget):
    def __init__(self):
        super().__init__()
        self.initWidgetObject()
        self.initLayout()

    def initWidgetObject(self):
        '''Initializes Widget, Signals and Slots'''
        self.cW = ConnectWidget()
        self.miW = MaskInfoWidget()
        self.pGraph = PressureGraph()

        #Connect cross object signals and slots
        self.cW.connected_sig.connect(self.startSerialReadHandler)
        self.cW.disconnected_sig.connect(self.stopSerialReadHandler)
        self.cW.connect_fail_sig.connect(self.connect_fail_handler)

    def initLayout(self):
        '''Initializes the layout of the widget'''

        #Construct a grid layout
        self.grid = QGridLayout()

        #Add widget to it
        self.grid.addWidget(self.cW, 0, 0)
        self.grid.addWidget(self.miW, 1, 0)
        self.grid.addWidget(self.pGraph, 2, 0 )

        #Assign layout to self
        self.setLayout(self.grid)
        self.resize(1000, 500)

    ### Start of SLOT ###
    def startSerialReadHandler(self):

        self.read_worker = SerialReadWorker(self.cW.getConnectedPort())
        self.write_handler = SerialWriteHandler(self.cW.getConnectedPort())

        self.miW.scale_btn.clicked.connect(self.write_handler.requestScale)
        self.miW.calibrate_btn.clicked.connect(self.write_handler.requestCalibration)          
        self.write_handler.moveToThread(self.read_worker)
        
        self.read_worker.start()
        self.read_worker.getReadHandler().dataReady.connect(self.updateWidget)

    def stopSerialReadHandler(self):
        self.read_worker.quit()
        sample = [0]*16
        self.miW.updateSamples(sample)
        self.miW.update()

    def updateWidget(self):
        '''Update the contained widget and start their repaint event''' 
        self.data_read = self.read_worker.getReadHandler().getDataRead();
        self.startbyte = self.data_read[0]

        if (bytes([self.startbyte[0] & b'\x02'[0]]) == b'\x02'):
            self.miW.scaleEnable()
        else:
            self.miW.scaleDisable()

        if (bytes([self.startbyte[0] & b'\x04'[0]]) == b'\x04'):
            self.miW.maskstatlbl.setText("Mask Attached Properly")
        else:
            self.miW.maskstatlbl.setText("Mask Not Attached Properly")

        self.miW.updateSamples(self.data_read[1:-1])
        self.miW.update()
        self.pGraph.update(self.data_read[-1])

    #Not implemented
    #Description: Handle when connection fail
    def connect_fail_handler(self):
        pass
    ### End of SLOT ###

    def getConnectWidget(self):
        return self.cW

if __name__ == '__main__':
    import sys
    app = QApplication(sys.argv)
    app.setStyle("fusion")
    wC = WidgetContainer()
    wC.show()
    app.exec_()