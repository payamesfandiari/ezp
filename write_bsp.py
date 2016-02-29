'''
Created on Nov 3, 2015

@author: payam
'''


import numpy as np


def write_bsp(file_path,data,labels):
    f = open(file_path,'w')
    for x,y in zip(data,labels):
        print(y,end=' ', file=f)
        for d in x:
            print("{0} ".format(d),end=' ', file=f)
        print('',file=f)
    f.close()


def write_liblinear(file_path,data,labels):
    f = open(file_path,'w')
    for x,y in zip(data,labels):
        print(y,end=' ', file=f)
        i = 1
        for d in x:
            print("{0}:{1} ".format(i,d),end=' ', file=f)
            i += 1
        print('',file=f)
    f.close()
    
def write_arff(file_path,data,labels,name):
    f = open(file_path,'w')
    print("@RELATION {0}".format(name),file=f)
    for i in range(data.shape[1]):
        print("@ATTRIBUTE {0} NUMERIC".format(i),file=f)
    
    print("@ATTRIBUTE class {-1,1}",file=f)
    print("@DATA",file=f)
    
    for x,y in zip(data,labels):        
        for d in x:
            print("{0},".format(d),end='', file=f)
        print(y, file=f)
    f.close()
    
def write_lemga(file_path,data,labels):
    f = open(file_path,'w')
    for x,y in zip(data,labels):
        for d in x:
            print("{0:0.5f} ".format(d),end='', file=f)
        print(y,file=f)
        # print('\n')

    f.close()
