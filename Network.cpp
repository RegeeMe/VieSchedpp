//
// Created by matth on 23.06.2018.
//

#include "Network.h"
using namespace VieVS;
using namespace std;

unsigned long VieVS::Network::nextId = 0;

Network::Network(): VieVS_Object(nextId++) {
    nsta_ = 0;
    nbls_ = 0;
    maxDistBetweenCorrespondingTelescopes_ = 0;
}

void Network::addStation(Station station) {
    if(station.getId() != stations_.size()) {
        cerr << "Changing id of " << station.getName() << " from " << station.getId() << " to " << stations_.size()
             << "\n";
        station.setId(stations_.size());
    }


    bool skyCoveragFound = false;
    for(const auto &any: stations_){
        // create baseline
        string name = (boost::format("%s-%s") %any.getName() %station.getName()).str();
        string alternativeName = (boost::format("%s-%s") %station.getName() %any.getName()).str();
        baselines_.emplace_back(name,alternativeName,any.getId(),station.getId());
        staids2blid_[{any.getId(), station.getId()}] = baselines_.back().getId();

        // create delta xyz
        double dx = any.getPosition().getX()-station.getPosition().getX();
        double dy = any.getPosition().getY()-station.getPosition().getY();
        double dz = any.getPosition().getZ()-station.getPosition().getZ();
        staids2dxyz_[{any.getId(), station.getId()}] = {dx,dy,dz};

        // add to existing sky coverage object if possible
        double dist = sqrt(dx*dx+dy*dy+dz*dz);
        if(dist <= maxDistBetweenCorrespondingTelescopes_){
            staids2skyCoverageId_[station.getId()] = staids2skyCoverageId_[any.getId()];
            skyCoveragFound = true;
        }
    }
    // if necessary create new sky coverage object
    if(!skyCoveragFound){
        staids2skyCoverageId_[station.getId()] = skyCoverages_.size();
        skyCoverages_.emplace_back();
    }

    // finally push back station
    stations_.push_back(std::move(station));

    nsta_ = stations_.size();
    nbls_ = (nsta_*(nsta_-1))/2;

}

const Station &Network::getStation(unsigned long id) const noexcept {
    return stations_[id];
}

const Station &Network::getStation(const std::string &name) const noexcept {
    for(const auto &any: stations_){
        if(any.hasName(name)){
            return any;
        }
    }
}

const std::vector<Station> &Network::getStations() const noexcept {
    return stations_;
}

const Baseline &Network::getBaseline(unsigned long id) const noexcept {
    return baselines_[id];
}

const Baseline &Network::getBaseline(unsigned long staid1, unsigned long staid2) const noexcept {
    if(staid1>staid2){
        swap(staid1,staid2);
    }

    unsigned long blid = staids2blid_.at({staid1,staid2});
    return baselines_[blid];
}

const Baseline &Network::getBaseline(const std::pair<unsigned long, unsigned long> &staids) const noexcept {
    return getBaseline(staids.first,staids.second);
}


const Baseline &Network::getBaseline(const std::string &name) const noexcept {
    for(const auto &any: baselines_){
        if(any.hasName(name)){
            return any;
        }
    }
}

const std::vector<Baseline> &Network::getBaselines() const noexcept {
    return baselines_;
}


const SkyCoverage &Network::getSkyCoverage(unsigned long id) const noexcept {
    return skyCoverages_[id];
}

const std::vector<SkyCoverage> &Network::getSkyCoverages() const noexcept {
    return skyCoverages_;
}

Station &Network::refStation(unsigned long id) {
    return stations_[id];
}

Station &Network::refStation(const std::string &name) {
    for(auto &any: stations_){
        if(any.hasName(name)){
            return any;
        }
    }
}

std::vector<Station> &Network::refStations() {
    return stations_;
}


Baseline &Network::refBaseline(unsigned long id) {
    return baselines_[id];
}

Baseline &Network::refBaseline(unsigned long staid1, unsigned long staid2) {
    if(staid1>staid2){
        swap(staid1,staid2);
    }

    return baselines_[staids2blid_[{staid1,staid2}]];
}

Baseline &Network::refBaseline(const std::pair<unsigned long, unsigned long> &staids) {
    return refBaseline(staids.first, staids.second);
}

Baseline &Network::refBaseline(const std::string &name) {
    for(auto &any: baselines_){
        if(any.hasName(name)){
            return any;
        }
    }
}

std::vector<Baseline> &Network::refBaselines() {
    return baselines_;
}

unsigned long Network::getBlid(unsigned long staid1, unsigned long staid2) const noexcept {
    return getBaseline(staid1, staid2).getId();
}

unsigned long Network::getBlid(const std::pair<unsigned long, unsigned long> &staids) const noexcept {
    return getBaseline(staids.first, staids.second).getId();
}

SkyCoverage &Network::refSkyCoverage(unsigned long id) {
    return skyCoverages_[id];
}

std::vector<SkyCoverage> &Network::refSkyCoverages() {
    return skyCoverages_;
}

void Network::update(unsigned long nObs, const PointingVector &pointingVector, bool influence) {
    unsigned long staid = pointingVector.getStaid();

    stations_[staid].update(nObs, pointingVector, influence);

    if(influence){

        unsigned long skyCoverageId = staids2skyCoverageId_[staid];
        skyCoverages_[skyCoverageId].update(pointingVector);
    }
}

void Network::update(unsigned long blid, bool influence) {
    baselines_[blid].update(influence);
}

const std::vector<double> &Network::getDxyz(unsigned long staid1, unsigned long staid2) const {
    if(staid1>staid2){
        swap(staid1,staid2);
    }

    return staids2dxyz_.at({staid1,staid2});
}


double Network::calcScore_skyCoverage(const vector<PointingVector> &pvs) const {

    double score = 0;

    for (const auto &pv : pvs){
        unsigned long staid = pv.getStaid();
        unsigned long skyCovid = staids2skyCoverageId_.at(staid);
        const SkyCoverage &thisSkyCov = skyCoverages_.at(skyCovid);
        score += thisSkyCov.calcScore(pv);
    }

    return score / pvs.size();

}

double Network::calcScore_skyCoverage(const vector<PointingVector> &pvs,
                                      map<unsigned long, double> &staids2skyCoverageScore) const {

    double score = 0;

    for(int i=0; i<pvs.size(); ++i){
        const PointingVector &pv = pvs[i];
        unsigned long staid = pv.getStaid();
        unsigned long skyCovid = staids2skyCoverageId_.at(staid);
        const SkyCoverage &thisSkyCov = skyCoverages_.at(skyCovid);
        double thisScore = thisSkyCov.calcScore(pv);
        staids2skyCoverageScore[staid] = thisScore;
        score += thisScore;
    }

    return score / pvs.size();
}

double Network::calcScore_skyCoverage_subnetting(const vector<PointingVector> &pvs,
                                                 const map<unsigned long, double> &staids2skyCoverageScore) const {

    double score = 0;

    for(int i=0; i<pvs.size(); ++i){
        const PointingVector &pv = pvs[i];
        unsigned long staid = pv.getStaid();
        score += staids2skyCoverageScore.at(staid);
    }

    return score / pvs.size();

}




