
import numpy as np


class Problem(object):
    '''
    Class for Handling the Problem
    '''
    __dataPath = None
    __labelPath = None
    __alldata = None
    __truelabels = None

    def __init__(self,datapath, labelpath, datasetname = None, change0 = True):
        self.__dataPath = datapath
        self.__labelPath = labelpath
        self.datasetname = datasetname
        self.num_class = 0
        self.__read(change0)
        self.change0 = change0
    def __read(self,change0):
        self.__alldata = np.loadtxt(self.dataPath, dtype = 'f')
        self.__truelabels = np.loadtxt(self.labelPath, dtype = 'i')
        self.num_class = np.unique(self.__truelabels[:,0]).shape[0]
        if change0:
            self.__truelabels[self.__truelabels == 0] = -1

    def getuserrandomclass(self,rcpath):
        train_labels = np.loadtxt(rcpath, dtype = 'i')
        train_labels = train_labels[train_labels[:, 1].argsort()]
        train_data = self.__alldata[train_labels[:, 1], :]
        ind = np.in1d(self.__truelabels[:, 1], train_labels[:, 1], assume_unique = True) == False
        test_data = self.__alldata[ind, :]
        test_labels = self.__truelabels[ind]
        train_labels = train_labels[:, 0]
        test_labels = test_labels[:, 0]
        if self.change0:
            train_labels[train_labels == 0] = -1
        return train_data, train_labels, test_data, test_labels

    def getdatapath(self):
        return self.__dataPath

    def getlabelpath(self):
        return self.__labelPath

    def __str__(self):
        if self.datasetname is None:
            return self.__dataPath
        else:
            return self.datasetname

    dataPath = property(fget = getdatapath)
    labelPath = property(fget = getlabelpath)



