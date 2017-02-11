#!/usr/bin/python3
# -*- coding: utf-8 -*-
import sys
from PyQt5.QtWidgets import *
from ConnectWidget import ConnectWidget
from MaskInfoWidget import MaskInfoWidget
from SampleFetcher import SampleFetcher

### class WidgetContainer: 
### Description : Widget to contain every other widget
class WidgetContainer(QWidget):
    def __init__(self):
        super().__init__()
        self.initWidgetObject()
        self.initLayout()

    def initWidgetObject(self):

        self.cW = ConnectWidget()
        self.miW = MaskInfoWidget()

        #self.fetcher =


    def initLayout(self):
        #Construct a grid layout
        self.grid = QGridLayout()
        
        #Add widget to it
        self.grid.addWidget(self.cW,0,0)
        self.grid.addWidget(self.miW,1,0)

        #Assign layout to self
        self.setLayout(self.grid)




if __name__ == '__main__':
    app = QApplication(sys.argv)
    wC = WidgetContainer()
    wC.show()
    app.exec_()