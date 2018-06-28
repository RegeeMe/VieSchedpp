/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   skyCoverage.cpp
 * Author: mschartn
 * 
 * Created on June 29, 2017, 11:28 AM
 */

#include "SkyCoverage.h"
using namespace std;
using namespace VieVS;

unsigned long VieVS::SkyCoverage::nextId = 0;

double SkyCoverage::maxInfluenceTime = 3600;
double SkyCoverage::maxInfluenceDistance = 30*deg2rad;
SkyCoverage::Interpolation SkyCoverage::interpolationDistance = Interpolation::linear;
SkyCoverage::Interpolation SkyCoverage::interpolationTime = Interpolation::linear;

SkyCoverage::SkyCoverage(): VieVS_Object(nextId++){
}

double SkyCoverage::calcScore(const vector<PointingVector> &pvs,
                                   const vector<Station> &stations) const noexcept {

    double score = 0;

    for (const auto &pv_new : pvs) {
        if (!hasId(stations[pv_new.getStaid()].getSkyCoverageID())) {
            continue;
        }

        double min_score = 1;

        for (const auto &pv_old : pointingVectors_) {
            if(pv_old.getTime()>pv_new.getTime()){
                continue;
            }
            double thisScore = scorePerPointingVector(pv_new, pv_old);
            if (thisScore < min_score) {
                min_score = thisScore;
            }
        }
        score = min_score;
    }
    return score;
}


double SkyCoverage::calcScore(const vector<PointingVector> &pvs, const vector<Station> &stations,
                                   vector<double> &firstScorePerPv) const noexcept {

    double score = 0;

    for (int idx_newObs = 0; idx_newObs < pvs.size(); ++idx_newObs) {
        const PointingVector &pv_new = pvs[idx_newObs];
        if (!hasId(stations[pv_new.getStaid()].getSkyCoverageID())) {
            continue;
        }

        double min_score = 1;

        for (const auto &pv_old : pointingVectors_) {
            if(pv_old.getTime()>pv_new.getTime()){
                continue;
            }
            double thisScore = scorePerPointingVector(pv_new, pv_old);
            if (thisScore < min_score) {
                min_score = thisScore;
            }
        }

        firstScorePerPv[idx_newObs] = min_score;
        score = min_score;
    }
    return score;
}

double SkyCoverage::calcScore_subcon(const vector<PointingVector> &pvs,
                                          const vector<Station> &stations,
                                          const vector<double> &firstScorePerPv) const noexcept {

    double score = 0;

    for (int idx_newObs = 0; idx_newObs < pvs.size(); ++idx_newObs) {
        const PointingVector &pv_new = pvs[idx_newObs];
        if (!hasId(stations[pv_new.getStaid()].getSkyCoverageID())) {
            continue;
        }
        score = firstScorePerPv[idx_newObs];
        break;
    }
    return score;
}

void SkyCoverage::update(const PointingVector &pv) noexcept {
    pointingVectors_.push_back(pv);
}

double
SkyCoverage::scorePerPointingVector(const PointingVector &pv_new,
                                         const PointingVector &pv_old) const noexcept {
    long deltaTime = (long) pv_new.getTime() - (long) pv_old.getTime();
    if (deltaTime > SkyCoverage::maxInfluenceTime) {
        return 1;
    }

    if (abs(pv_new.getEl() - pv_old.getEl()) > SkyCoverage::maxInfluenceDistance) {
        return 1;
    }

    float distance = LookupTable::angularDistance(pv_new, pv_old);

    if (distance > SkyCoverage::maxInfluenceDistance) {
        return 1;
    }

    double scoreDistance;
    switch (interpolationDistance) {
        case Interpolation::constant: {
            scoreDistance = 0;
            break;
        }
        case Interpolation::linear: {
            scoreDistance = distance / maxInfluenceDistance;
            break;
        }
        case Interpolation::cosine: {
            scoreDistance = .5 + .5 * (LookupTable::cosLookup(distance * pi / maxInfluenceDistance));
            break;
        }
    }

    double scoreTime;
    switch (interpolationTime) {
        case Interpolation::constant: {
            scoreTime = 0;
            break;
        }
        case Interpolation::linear: {
            scoreTime = deltaTime / maxInfluenceTime;
            break;
        }
        case Interpolation::cosine: {
            scoreTime = .5 + .5 * (LookupTable::cosLookup(deltaTime * pi / maxInfluenceTime));
            break;
        }
    }

    return 1 - (scoreDistance * scoreTime);
}

void SkyCoverage::clearObservations() {
    pointingVectors_.clear();
}


