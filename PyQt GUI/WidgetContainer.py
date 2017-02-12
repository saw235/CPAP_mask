#!/usr/bin/python3
# -*- coding: utf-8 -*-
import sys

from PyQt5.QtWidgets import *
from PyQt5.QtCore import QThread, pyqtSignal, QMutex

from ConnectWidget import ConnectWidget
from MaskInfoWidget import MaskInfoWidget
from SampleFetcher import SampleFetcher


### class WidgetContainer: 
### Description : Widget to contain every other widget
class WidgetContainer(QWidget):
    def __init__(self):
        super().__init__()

        self.shared_mutex = QMutex()

        self.initWidgetObject()
        self.initLayout()



    def initWidgetObject(self):

        self.cW = ConnectWidget()
        self.miW = MaskInfoWidget()

        #Connect cross object signals and slots
        self.cW.connected_sig.connect(self.startFetcherThread)
        self.cW.disconnected_sig.connect(self.stopFetcherThread)
        self.cW.connect_fail_sig.connect(self.connect_fail_handler)

        self.miW.scale_btn.clicked.connect(self.startRequestScaleThread)

    def initLayout(self):
        #Construct a grid layout
        self.grid = QGridLayout()

        #Add widget to it
        self.grid.addWidget(self.cW,0,0)
        self.grid.addWidget(self.miW,1,0)

        #Assign layout to self
        self.setLayout(self.grid)
        self.resize(1000,500)

    ### Start of SLOT ###
    def startFetcherThread(self):

        #Instantiate a fetcher and a thread
        self.fetcher = SampleFetcher(self.cW.getConnectedPort())
        self.thread = QThread()

        #Connect signals so that process is called when thread is started
        self.thread.started.connect(self.fetcher.process)

        self.fetcher.doneSampling.connect(lambda : self.miW.updateSamples(self.fetcher.getSamples()))
        self.fetcher.doneSampling.connect(self.miW.update)
        self.fetcher.doneSampling.connect(self.thread.quit)
        self.fetcher.doneSampling.connect(lambda : self.miW.getChart().updateSeries(self.fetcher.getSamples())) 

        self.thread.finished.connect(self.restartFetcherThread)

        #apply shared mutex
        self.fetcher.setMutex(self.shared_mutex)

        #Move fetcher to the thread and start the process
        self.fetcher.moveToThread(self.thread)
        self.thread.start()

    def restartFetcherThread(self):
        self.thread.start()

    def stopFetcherThread(self):
        self.thread.quit()
        sample = [0]*16
        self.miW.updateSamples(sample)
        self.miW.getChart().updateSeries(sample)

    def startRequestScaleThread(self):
         #Instantiate a fetcher and a thread
        self.req_scale = SampleFetcher(self.cW.getConnectedPort())
        self.req_scale_thread = QThread()

        #Connect signals so that requestScale is called when thread is started
        self.req_scale_thread.started.connect(self.fetcher.requestScale)
        self.fetcher.doneRequestScale.connect(self.req_scale_thread.quit)


        self.req_scale.setMutex(self.shared_mutex)
        self.req_scale.moveToThread(self.req_scale_thread)
        self.req_scale_thread.start()


    #Not implemented
    #Description: Handle when connection fail
    def connect_fail_handler(self):
        pass
    ### End of SLOT ###

if __name__ == '__main__':
    app = QApplication(sys.argv)
    wC = WidgetContainer()
    wC.show()
    app.exec_()