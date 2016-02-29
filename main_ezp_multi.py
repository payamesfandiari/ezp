
import numpy as np
import subprocess as sub
from operator import itemgetter
import sklearn.metrics as m
from sklearn.svm import LinearSVC
import xgboost as xgb
from sklearn.model_selection import GridSearchCV


def report(grid_scores, n_top=3):
    top_scores = sorted(grid_scores, key=itemgetter(1), reverse=True)[:n_top]
    return top_scores[0]


dataset_name = 'mnist/'
train_file = 'datasets_v1/{0}train.bsp'.format(dataset_name)
test_file = 'datasets_v1/{0}test.bsp'.format(dataset_name)
model_file = 'datasets_v1/{0}model.bsp'.format(dataset_name)
X,X_t = np.loadtxt(train_file),np.loadtxt(test_file)
X,y,X_t,y_t = X[:,1:],X[:,0].astype('i'),X_t[:,1:],X_t[:,0].astype('i')
num_class = np.unique(y)
err = 0
out_clf =np.zeros((num_class.shape[0],y_t.shape[0]))
#
train_ops = "-z 2 -c 10 -zs 5000 -i 1"
test_ops = "-f 1 -s 5"

sub.check_output('./exe/train {0} {1} {2}'.format(train_ops,train_file,model_file),shell=True)
sub.check_output('./exe/predict {0} {1} {2} {3}'.format(test_ops,train_file,model_file,train_file+".tr"),shell=True)
sub.check_output('./exe/predict {0} {1} {2} {3}'.format(test_ops,test_file,model_file,test_file+".tr"),shell=True)

for i in num_class:
    X_train_trans,X_test_trans = np.loadtxt(train_file+".tr%d"%i),np.loadtxt(test_file+".tr%d"%i)
    param_grid = {"C": [.000001,0.00001,.0001,0.001,0.01,0.1,1,10,100,1000,10000,1e5,1e6]}
    clf = LinearSVC(tol= 0.00001,dual = False)

    grid_search = GridSearchCV(clf, param_grid=param_grid,n_jobs=-1,verbose=0)
    grid_search.fit(X_train_trans[:,1:],X_train_trans[:,0].astype('i'))

    best_params = report(grid_search.grid_scores_)
    clf = LinearSVC(C=best_params.parameters['C'],tol = 1e-6,dual = False,class_weight = 'balanced')
    out_clf[i] = clf.fit(X_train_trans[:,1:],X_train_trans[:,0].astype('i')).decision_function(X_test_trans[:,1:])
    # clf = xgb.XGBClassifier(max_depth = 6,n_estimators = 500,nthread = 6)
    # out_clf[i] = clf.fit(X_train_trans[:,1:],X_train_trans[:,0].astype('i')).predict_proba(X_test_trans[:,1:])[:,1]


pred_y = np.argmax(out_clf.T,axis = 1)
# print(pred_y)
print(m.accuracy_score(y_t,pred_y))

# param_grid = {"C": [.000001,0.00001,.0001,0.001,0.01,0.1,1,10,100,1000,10000,1e5,1e6]}
# clf = LinearSVC(tol= 0.00001,dual = False)
#
# grid_search = GridSearchCV(clf, param_grid=param_grid,n_jobs=-1,verbose=1)
# grid_search.fit(X,y)
#
# best_params = report(grid_search.grid_scores_)
# clf = LinearSVC(C=best_params.parameters['C'],tol= 0.00001,dual = False)
# clf.fit(X,y)
# print(clf.score(X_t,y_t))
#
#
# clf = xgb.XGBClassifier(max_depth = 6,n_estimators = 500,nthread = 6)
# clf.fit(X,y)
# print(clf.score(X_t,y_t))
