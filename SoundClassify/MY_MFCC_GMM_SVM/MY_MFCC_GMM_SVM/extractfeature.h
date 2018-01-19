#pragma once
#include <stdio.h>
#include <string>
#include <math.h>
#include <iostream>
#include <time.h>
#include "filterbank.h"
#include "GMM.h"
#include <vector>
using namespace std;

#define FRAMENUM 400
#define PLUSFACTOR 1E10

double *extracttrain_mfcc(string wavpath,const int wavnum,long int &datasize, int &dim,string &feature);

double *extracttrain_mdiff(string wavpath,const int wavnum,long int &datasize, int &dim,string &feature);

double *extracttrain_mfcc_diff(string wavpath,const int wavnum,long int &datasize, int &dim,string &feature);

double *extracttrain_mfcc_diff_valide(string wavpath,const int wavnum,long int &datasize, int &dim,string &feature,bool output=false,string classname="+1");

void extracttrain_mfcc_diff_valide_singleoutput(string wavpath,const int wavnum,long int &datasize, int &dim,string &feature,bool output=false,string classname="+1");

double *extracttrain_mfcc_energy(string wavpath,int wavnum,long int &datasize, int &dim,string &feature);

double *extracttrain_mfcc_energy_valide(string wavpath,const int wavnum,long int &datasize, int &dim,string &feature);

void train_save_gmm(const int dim, const int gmm_mix_num, const int size, double *wav_data, string gmm_txt);

GMM* loadgmm(const int dim, const int gmm_mix_num, string load_gmm_struct);

void testwavs(string wavpath,const int wavnum,GMM *gmm,const int dim,const string feature);

void testwavs_valide(string wavpath,const int wavnum,GMM *gmm,const int dim,const string feature);

void testsinglewav(string wavpath,GMM *gmm,const int dim,const string feature,double &averprob,double &maxprob);

void testsinglewav_valide(string wavpath,GMM *gmm,int dim,const string feature,double &averprob,double &maxprob);