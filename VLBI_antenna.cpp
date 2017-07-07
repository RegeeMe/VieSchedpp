/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   antenna.cpp
 * Author: mschartn
 * 
 * Created on June 27, 2017, 11:15 AM
 */

#include "VLBI_antenna.h"
using namespace std;

namespace VieVS{
    VLBI_antenna::VLBI_antenna(){}
    
    VLBI_antenna::VLBI_antenna(double offset_m, 
                     double diam_m, 
                     double rate1_deg_per_min,
                     double constantOverhead1_s, 
                     double rate2_deg_per_min,
                     double constantOverhead2_s):
                     offset{offset_m}, 
                     diam{diam_m},
                     rate1{rate1_deg_per_min*deg2rad/60},
                     con1{constantOverhead1_s},
                     rate2{rate2_deg_per_min*deg2rad/60},
                     con2{constantOverhead2_s}{}

    VLBI_antenna::~VLBI_antenna() {
    }
    
    void VLBI_antenna::scanStart(VLBI_pointingVector& old_pointingVector, VLBI_pointingVector& new_pointingVector, int wait_setup, int wait_source, int wait_tape, int wait_idle, int wait_calibration){
        
        double delta1 = old_pointingVector.getAz()-new_pointingVector.getAz();
        double delta2 = old_pointingVector.getEl()-new_pointingVector.getEl();
        double acc1 = 1*deg2rad;
        double acc2 = 1*deg2rad;
        
        double t_acc_1 = rate1/acc1;
        double t_acc_2 = rate2/acc2;
        
        double s_acc_1 = 2*(acc1*t_acc_1*t_acc_1/2);
        double s_acc_2 = 2*(acc2*t_acc_2*t_acc_2/2);
        
        double t_1, t_2;
        if (delta1<s_acc_1){
            t_1 = 2*sqrt(delta1/acc1);
        } else {
            t_1 = 2*t_acc_1 + (delta1)/rate1;
        }
        if (delta2<s_acc_2){
            t_2 = 2*sqrt(delta2/acc2);
        } else {
            t_2 = 2*t_acc_2 + (delta2)/rate2;
        }
        
        int slewtime = (int) ceil(t_1);
        if(t_2>t_1){
            slewtime = (int) ceil(t_2);
        }
        
        
        new_pointingVector.setTime(new_pointingVector.getTime()+
                                   boost::posix_time::seconds(slewtime)+
                                   boost::posix_time::seconds(wait_setup)+
                                   boost::posix_time::seconds(wait_source)+
                                   boost::posix_time::seconds(wait_tape)+
                                   boost::posix_time::seconds(wait_idle)+
                                   boost::posix_time::seconds(wait_calibration));
        
    }
    
    ostream& operator<<(ostream& out, const VLBI_antenna& antenna){
        double rate1degs = antenna.rate1*rad2deg;
        double rate2degs = antenna.rate2*rad2deg;
        cout << "Antenna: " << antenna.diam << " [m] \n" ;
        cout << boost::format("    slew rate axis1: %6.2f [deg/s]\n") % rate1degs;
        cout << boost::format("    slew rate axis2: %6.2f [deg/s]\n") % rate2degs;
        return out;
    }
}
