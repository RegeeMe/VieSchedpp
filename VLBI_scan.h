/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   VLBI_scan.h
 * Author: mschartn
 *
 * Created on June 29, 2017, 3:27 PM
 */

#ifndef VLBI_SCAN_H
#define VLBI_SCAN_H
#include <iostream>
#include <boost/date_time.hpp>
#include <vector>

#include "VLBI_pointingVector.h"
#include "VieVS_constants.h"

using namespace std;
namespace VieVS{

    class VLBI_scan {
    public:
        VLBI_scan();
        VLBI_scan(vector<VLBI_pointingVector> pointingVectors);
        vector<VLBI_pointingVector> getPointingVectors(){return pointingVectors;}
        virtual ~VLBI_scan();
    private:
        vector<VLBI_pointingVector> pointingVectors;
//        vector<int> staids;
//        int srcid;
//        vector<VLBI_baseline> baselines;
    };
}
#endif /* VLBI_SCAN_H */

