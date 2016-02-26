//
// Created by payam on 2/14/16.
//

#include <limits>
#include <random>
#include <iostream>
#include <memory>
#include <cstring>
#include "bsp.h"

//
//#ifndef min
//template <class T> static inline T min(T x,T y) { return (x<y)?x:y; }
//#endif
//#ifndef max
//template <class T> static inline T max(T x,T y) { return (x>y)?x:y; }
//#endif
#define Malloc(type,n) (type*)malloc((n)*sizeof(type))

//
//#ifdef __cplusplus
//extern "C" {
//#endif
//
//extern double dnrm2_(int *, double *, int *);
//extern double ddot_(int *, double *, int *, double *, int *);
//extern int daxpy_(int *, double *, double *, int *, double *, int *);
//extern int dscal_(int *, double *, double *, int *);
//
//#ifdef __cplusplus
//}
//#endif


void fisher_yates(int* array, int l) {
    int i, j;
    int temp;
    for (i = l; i > 0; i--) {
        j = rand() % (i + 1);
        if (i == j)
            continue;
        temp = array[j];
        array[j] = array[i];
        array[i] = temp;
    }

}

struct model* bsp(struct parameter* param,struct problem* prob){
    std::mt19937 rng;
    rng.seed(std::random_device()());
    std::uniform_real_distribution<double> dist(0.0,param->initial_w);
    std::uniform_real_distribution<double> ils_random_gen(0.0,1.0);
    int rows(prob->rows);
    int cols(prob->cols);
    struct model* output_model_ = new struct model[param->ils_itr];

    for (int i = 0; i < param->ils_itr; ++i) {
        output_model_[i].W = Malloc(double,cols);
    }
    /*allocate space*/
    int* uniq_proj_ind = Malloc(int,rows+1);
    int* rows_j = Malloc(int,cols);
    int* column_numbers = Malloc(int,cols);
    int* labelmap = Malloc(int,rows);
    double* column = Malloc(double,rows);
    double* projection = Malloc(double,rows);
    double *w = Malloc(double,cols);

    for (int j = 0; j < cols; j++) {
        rows_j[j] = 2;
    }

    /*******************************/
    /***** Initialization **********/
    /*******************************/

    /*make random plane*/
     for (int j = 0; j < cols; ++j) {
        w[j] = dist(rng);
    }

    /*length of w*/
    double w_length = 0;
    for (int j = 0; j < cols; j++) {
        w_length += w[j] * w[j];
    }
    w_length = sqrt(w_length);

    /*obtain initial dot products*/
    for (int i = 0; i < rows; i++) {
        projection[i] = 0;
        for (int j = 0; j < cols; j++) {
            projection[i] += prob->data[i*cols + j] * w[j];
        }
        projection[i] /= w_length;
    }

    for (int i = 0; i < rows; i++)
        labelmap[i] = i;

    /*first we sort the projection*/
    double temp;
    int temp2;
    int j = 0;
    for (int i = 1; i < rows; i++) {
        j = i;
        while (j > 0 && projection[j - 1] > projection[j]) {
            temp = projection[j];
            projection[j] = projection[j - 1];
            projection[j - 1] = temp;
            temp2 = prob->labels[j];
            prob->labels[j] = prob->labels[j - 1];
            prob->labels[j - 1] = temp2;
            temp2 = labelmap[j];
            labelmap[j] = labelmap[j - 1];
            labelmap[j - 1] = temp2;
            j--;
        }
    }

    /*now obtain unique projection values*/
    double tempval = projection[0];
    uniq_proj_ind[0] = 0;
    int uniq_proj_ind_n = 1;
    for (int k = 1; k < rows; k++) {
        if (projection[k] != tempval) {
            uniq_proj_ind[uniq_proj_ind_n++] = k;
            tempval = projection[k];
        }
    }
    uniq_proj_ind[uniq_proj_ind_n] = rows;
    uniq_proj_ind_n++;

    double w0 = -1 * (projection[uniq_proj_ind[0]] + projection[uniq_proj_ind[1]]) / 2;
    double error = 0;
    double errormargin = 0;
    int below = 0;
    int above = 0;
    double errorplus = 0, errorminus = 0;
    double geomargin = INF;
    double abovedist = INF;
    double belowdist = INF;
    bool belowrow = false;
    bool aboverow = false;
    for (int m = 0; m < uniq_proj_ind_n - 1; m++) {
        for (int k = uniq_proj_ind[m]; k < uniq_proj_ind[m + 1]; k++) {
            if (prob->labels[k] * (projection[k] + w0) < geomargin) {
                geomargin = prob->labels[k] * (projection[k] + w0);
                if (prob->labels[k] == 1 && (projection[k] + w0) <= 0) {
                    belowrow = true;
                    belowdist = geomargin;
                } else if (prob->labels[k] == -1 && (projection[k] + w0) > 0) {
                    aboverow = true;
                    abovedist = geomargin;
                }
            }
            if (prob->labels[k] * (projection[k] + w0) == 0) {
                if (prob->labels[k] == 1) {
                    errorplus++;
                    below++;
                }
            } else if (prob->labels[k] * (projection[k] + w0) < 0) {
                if (prob->labels[k] == 1)
                    errorplus++;
                else
                    errorminus++;
                errormargin += prob->labels[k] * (projection[k] + w0);
                if (prob->labels[k] == 1 && (projection[k] + w0) < 0)
                    below++;
                else if (prob->labels[k] == -1 && (projection[k] + w0) > 0)
                    above++;
            }
        }
    }

    error = .5 * (errorplus / prob->nr_plus + errorminus / prob->nr_minus);

    /*initialize bestobjective*/
    double objective = errormargin + geomargin - param->C * error;

    double bestobjective = objective;
    double besterror = error;
    double bestw0 = w0;
    int bestm = 0;
    double oldw0 = 0,z=0;
    int below_prime = 0 ;
    for (int m = 1; m < uniq_proj_ind_n - 2; m++) {
        oldw0 = w0;
        z = ((projection[uniq_proj_ind[m]] + projection[uniq_proj_ind[m + 1]])
             / 2) - (-1 * w0);
        w0 -= z;
        if (belowrow) {
            belowdist -= z;
        }
        if (aboverow) {
            abovedist += z;
        }
        below_prime = below;
        for (int k = uniq_proj_ind[m]; k < uniq_proj_ind[m + 1]; k++) {
            if (prob->labels[k] * (projection[k] + w0) == 0) {
                if (prob->labels[k] == 1) {
                    errorplus++;
                    below++;
                    if (!belowrow) {
                        belowrow = true;
                        belowdist = prob->labels[k] * (projection[k] + w0);
                    }
                }
            } else if (prob->labels[k] * (projection[k] + w0) > 0) {
                if (prob->labels[k] == 1)
                    errorplus--;
                else
                    errorminus--;
                above = above - 1;
                errormargin = errormargin + (projection[k] + oldw0);
            } else if (prob->labels[k] * (projection[k] + w0) < 0) {
                if (!belowrow) {
                    belowrow = true;
                    belowdist = prob->labels[k] * (projection[k] + w0);
                }
                if (prob->labels[k] == 1)
                    errorplus++;
                else
                    errorminus++;
                below = below + 1;
                errormargin = errormargin + (projection[k] + w0);
            }
        }
        if (abovedist > 0)
            aboverow = false;
        if (!aboverow && !belowrow) {
            geomargin = prob->labels[uniq_proj_ind[m]]
                        * (projection[uniq_proj_ind[m]] + w0);
//            assert(geomargin >= 0);
        } else {
            if (belowdist < abovedist)
                geomargin = belowdist;
            else
                geomargin = abovedist;
        }
        errormargin = errormargin - below_prime * z + above * z;

//        margin = errormargin + geomargin;
        error = .5 * (errorplus / prob->nr_plus + errorminus / prob->nr_minus);

        objective = errormargin + geomargin - param->C * error;

        if (objective > bestobjective) {
            //    if(objective >= bestobjective) {
            bestobjective = objective;
            besterror = error;
            bestw0 = w0;
            bestm = m;
        }
    }
    /*******************************/
    /**** Main loop starts here ****/
    /*******************************/

    /********** Main loop *********/
    bool redocolumn(false);
    double prevbestobjective = -INF,old_w_length =0, oldval = 0, bestval = 0,newval =0  ;
//    assert(prevbestobjective < bestobjective);
    int extraiter(0),localit(0),startm(0),ils(0),plus_(0),minus_(0),localit_(0),endm(0);
    while (extraiter < param->ils_itr) {

        if (bestobjective - prevbestobjective <= param->stop_thresh || localit > param->local_itr_thresh) {
            /* Put the W in output */

            output_model_[extraiter].W0 = w0;
            output_model_[extraiter].error = besterror;
            output_model_[extraiter].obj = bestobjective;
            for (int j = 0; j < cols; j++) {
                output_model_[extraiter].W[j] = w[j] / w_length;
            }

            prevbestobjective =  -INF;
            bestobjective = -INF;
            extraiter++;
            ils = 1;
            localit = 0;
            redocolumn = false ;
            printf("******* DOING ILS PERMUTATION ******** \n");
            for (int j = 0; j < cols; j++) {
                if (ils_random_gen(rng) < param->ils_perm) {
                    temp = w[j];
                    if (ils_random_gen(rng) > .5)
                        w[j] = w[j] + param->ils_step * param->w_inc;
                    else
                        w[j] = w[j] - param->ils_step * param->w_inc;

                    /*update length of w*/
                    old_w_length = w_length;
                    w_length = w_length * w_length;
                    w_length -= temp * temp;
                    w_length += w[j] * w[j];
                    w_length = sqrt(w_length);

                    /*take out old value from dot product and add new value*/
                    for (int k = 0; k < rows; k++) {
                        projection[k] *= old_w_length;
                        projection[k] -= prob->data[labelmap[k]*cols + j] * temp;
                        projection[k] += prob->data[labelmap[k]*cols + j] * w[j];
                        projection[k] /= w_length;
                    }
                }
            }
        }

        for (j = 0; j < cols; j++) {
            column_numbers[j] = j;
        }
        fisher_yates(column_numbers, cols - 1);

        for (int j_ = 0; j_ < cols; j_++) {

            if (redocolumn && bestobjective - prevbestobjective > param->stop_thresh
                && localit <= param->local_itr_thresh && localit_ <= param->local_itr_thresh) {
                j_--;
                j = column_numbers[j_];
                if (plus_)
                    column[0] = w[j] + param->w_inc;
                if (minus_)
                    column[0] = w[j] - param->w_inc;

                rows_j[j] = 1;
                localit_++;
            } else {
                j = column_numbers[j_];
                column[0] = w[j];
                column[1] = w[j] + param->w_inc;
                column[2] = w[j] - param->w_inc;
                rows_j[j] = 3;
                redocolumn = false;
            }

            prevbestobjective = bestobjective;

            /*randomly permute column j values -- disable, this way it is faster because it doesn't affect the sort so much*/
            //  fisher_yates(column, rows_j[j]);
            /*iterate through numbers*/
            oldval = w[j];
            old_w_length = w_length;
            bestval = oldval;

            for (int i = 0; i < rows_j[j]; i += 1) {
                newval = column[i];
                /*update length of w*/
                w_length = w_length * w_length;
                w_length -= oldval * oldval;
                w_length += newval * newval;
                w_length = sqrt(w_length);

                /*take out old value from dot product and add new value*/
                for (int k = 0; k < rows; k++) {
                    projection[k] *= old_w_length;
                    projection[k] -= prob->data[labelmap[k]*cols + j] * oldval;
                    projection[k] += prob->data[labelmap[k]*cols + j] * newval;
                    projection[k] /= w_length;
                }
                for (int m = 1; m < rows; m++) {
                    int n = m;
                    while (n > 0 && projection[n - 1] > projection[n]) {
                        temp = projection[n];
                        projection[n] = projection[n - 1];
                        projection[n - 1] = temp;
                        temp2 = prob->labels[n];
                        prob->labels[n] = prob->labels[n - 1];
                        prob->labels[n - 1] = temp2;
                        temp2 = labelmap[n];
                        labelmap[n] = labelmap[n - 1];
                        labelmap[n - 1] = temp2;
                        n--;
                    }
                }

                tempval = projection[0];
                uniq_proj_ind[0] = 0;
                uniq_proj_ind_n = 1;
                for (int k = 1; k < rows; k++) {
                    if (projection[k] != tempval) {
                        uniq_proj_ind[uniq_proj_ind_n++] = k;
                        tempval = projection[k];
                    }
                }
                uniq_proj_ind[uniq_proj_ind_n] = rows;
                uniq_proj_ind_n++;

                if (ils) {
                    startm = 0;
                    endm = uniq_proj_ind_n - 1;
                } else {
                    startm = bestm - 10;
                    endm = bestm + 10;
                    if (startm < 0)
                        startm = 0;
                    if (endm > uniq_proj_ind_n - 1)
                        endm = uniq_proj_ind_n - 1;
                }

                w0 = -1
                     * (projection[uniq_proj_ind[startm]]
                        + projection[uniq_proj_ind[startm + 1]]) / 2;
//                error = 0;
                errormargin = 0;
                below = 0;
                above = 0;
                errorplus = 0;
                errorminus = 0;
                geomargin = INF;
                abovedist = INF;
                belowdist = INF;
                belowrow = false;
                aboverow = false;
                for (int m = 0; m < uniq_proj_ind_n - 1; m++) {
                    for (int k = uniq_proj_ind[m]; k < uniq_proj_ind[m + 1]; k++) {
                        if (prob->labels[k] * (projection[k] + w0) < geomargin) {
                            geomargin = prob->labels[k] * (projection[k] + w0);
                            if (prob->labels[k] == 1 && (projection[k] + w0) <= 0) {
                                belowrow = true;
                                belowdist = geomargin;
                            } else if (prob->labels[k] == -1
                                       && (projection[k] + w0) > 0) {
                                aboverow = true;
                                abovedist = geomargin;
                            }
                        }
                        if (prob->labels[k] * (projection[k] + w0) == 0) {
                            if (prob->labels[k] == 1) {
                                errorplus++;
                                below++;
                            }
                        } else if (prob->labels[k] * (projection[k] + w0) < 0) {
                            if (prob->labels[k] == 1)
                                errorplus++;
                            else
                                errorminus++;
                            errormargin += prob->labels[k] * (projection[k] + w0);
                            if (prob->labels[k] == 1 && (projection[k] + w0) < 0)
                                below++;
                            else if (prob->labels[k] == -1 && (projection[k] + w0) > 0)
                                above++;
                        }
                    }
                }
                //					if (belowdist < abovedist)
                //						assert(geomargin == belowdist);
                //					else
                //						assert(geomargin == abovedist);

//                margin = geomargin + errormargin;
                error = .5 * (errorplus / prob->nr_plus + errorminus / prob->nr_minus);

                /*update bestobjective*/
                objective = errormargin + geomargin - param->C * error;
                //    objective = -1 * error;

                if (objective > bestobjective || ils == 1) {
                    ils = 0;
                    bestobjective = objective;
                    besterror = error;
                    bestw0 = w0;
                    bestval = newval;
                    bestm = startm;
                }

                for (int m = startm + 1; m < endm - 1; m++) {
                    oldw0 = w0;
                    z = ((projection[uniq_proj_ind[m]]
                          + projection[uniq_proj_ind[m + 1]]) / 2)
                        - (-1 * w0);
                    w0 -= z;
                    if (belowrow) {
                        belowdist -= z;
                    }
                    if (aboverow) {
                        abovedist += z;
                    }
                    below_prime = below;
                    for (int k = uniq_proj_ind[m]; k < uniq_proj_ind[m + 1]; k++) {
                        if (prob->labels[k] * (projection[k] + w0) == 0) {
                            if (prob->labels[k] == 1) {
                                errorplus++;
                                below++;
                                if (!belowrow) {
                                    belowrow = true;
                                    belowdist = prob->labels[k] * (projection[k] + w0);
                                }
                            }
                        } else if (prob->labels[k] * (projection[k] + w0) > 0) {
                            if (prob->labels[k] == 1)
                                errorplus--;
                            else
                                errorminus--;
                            above = above - 1;
                            errormargin = errormargin + (projection[k] + oldw0);
                        } else if (prob->labels[k] * (projection[k] + w0) < 0) {
                            if (!belowrow) {
                                belowrow = true;
                                belowdist = prob->labels[k] * (projection[k] + w0);
                            }
                            if (prob->labels[k] == 1)
                                errorplus++;
                            else
                                errorminus++;
                            below = below + 1;
                            errormargin = errormargin + (projection[k] + w0);
                        }
                    }
                    if (abovedist > 0)
                        aboverow = false;
                    if (!aboverow && !belowrow) {
                        geomargin = prob->labels[uniq_proj_ind[m]] * (projection[uniq_proj_ind[m]] + w0);
                        //	assert(geomargin >= 0);
                    } else {
                        if (belowdist < abovedist)
                            geomargin = belowdist;
                        else
                            geomargin = abovedist;
                    }
                    errormargin = errormargin - below_prime * z + above * z;

//                    margin = errormargin + geomargin;
                    error = .5 * (errorplus / prob->nr_plus + errorminus / prob->nr_minus);

                    objective = errormargin + geomargin - param->C * error;

                    if (objective > bestobjective) {
                        bestobjective = objective;
                        besterror = error;
                        bestw0 = w0;
                        bestval = newval;
                        bestm = m;
                    }
                }

                //Do next number in the column (maybe skip a few if inc=50 or 100)
                oldval = newval;
                old_w_length = w_length;

            }

            if (bestval != w[j]) {
                redocolumn = true;
                if (plus_ == 0 && minus_ == 0) {
                    if (bestval == column[1])
                        plus_ = 1;
                    else
                        minus_ = 1;
                }
            } else {
                redocolumn = false;
                plus_ = 0;
                minus_ = 0;
            }

            w[j] = bestval;
            w0 = bestw0;

            /*length of w*/
            w_length = 0;
            for (int k = 0; k < cols; k++) {
                w_length += w[k] * w[k];
            }
            w_length = sqrt(w_length);

            /*take out old value from dot product and add new value*/
            for (int k = 0; k < rows; k++) {
                projection[k] *= old_w_length;
                projection[k] -= prob->data[labelmap[k]*cols + j] * oldval;
                projection[k] += prob->data[labelmap[k]*cols + j] * w[j];
                projection[k] /= w_length;
            }

        }

//        globalit++;
        localit++;
        //End of doing one iteration through all columns
    }


    return output_model_;

}
