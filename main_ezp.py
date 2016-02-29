
import numpy as np
from problem import Problem
import write_bsp as wbsp
import subprocess as sub
from sklearn.svm import LinearSVC
from os import remove

bootstrap_num = 1000
dataset_name = 'fertility/'
data_path = 'datasets_v1/{0}data'.format(dataset_name)
trueclass = 'datasets_v1/{0}trueclass'.format(dataset_name)
train_file = 'datasets_v1/{0}train.bsp'.format(dataset_name)
test_file = 'datasets_v1/{0}test.bsp'.format(dataset_name)
model_file = 'datasets_v1/{0}model.bsp'.format(dataset_name)
p = Problem(data_path,trueclass,change0=True)

err = 0
err_rnd = 0
train_ops = "-z 2 -zs 1000 -i 100 -l 10000"
test_ops = "-f 1 -s 4"

for rnd in range(10):
    randomclass = 'datasets_v1/{0}random_class.{1}'.format(dataset_name, rnd)
    X,y,X_t,y_t = p.getuserrandomclass(randomclass)
    wbsp.write_bsp(train_file,X,y)
    wbsp.write_bsp(test_file,X_t,y_t)
    out = sub.check_output('./exe/train {0} {1} {2}'.format(train_ops,train_file,model_file),shell=True)

    out = sub.check_output('./exe/predict {0} {1} {2} {3}'.format(test_ops,train_file,model_file,train_file+".tr"),shell=True)
    out = sub.check_output('./exe/predict {0} {1} {2} {3}'.format(test_ops,test_file,model_file,test_file+".tr"),shell=True)
    X_train_trans,X_test_trans = np.loadtxt(train_file+".tr"),np.loadtxt(test_file+".tr")
    X_train_trans,X_test_trans = X_train_trans[:,1:],X_test_trans[:,1:]
    clf = LinearSVC(C=0.001,tol= 0.00001,dual = False)
    clf.fit(X_train_trans,y)
    err_rnd = 1 - clf.score(X_test_trans,y_t)
    err += err_rnd
    print('Dataset : %s Random Class : %s -> %s'%(dataset_name,rnd,100 * err_rnd))
    remove(train_file+".tr")
    remove(test_file+".tr")
    remove(train_file)
    remove(test_file)
print('Dataset %s error = %f'%(dataset_name,100* err/10))