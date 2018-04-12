//
// Created by matth on 11.04.2018.
//

#include "CableWrap_HaDc.h"
using namespace VieVS;
using namespace std;

CableWrap_HaDc::CableWrap_HaDc(double axis1_low_deg, double axis1_up_deg, double axis2_low_deg, double axis2_up_deg)
        : CableWrap(axis1_low_deg, axis1_up_deg, axis2_low_deg, axis2_up_deg) {
}

bool CableWrap_HaDc::anglesInside(const PointingVector &p) const noexcept {
    double ax1 = p.getHa();
    double ax2 = p.getDc();

    return axisInsideCableWrap(ax1, ax2);
}

void CableWrap_HaDc::unwrapAzNearAz(PointingVector &new_pointingVector, double az_old) const noexcept {
}

void CableWrap_HaDc::calcUnwrappedAz(const PointingVector &old_pointingVector,
                                     PointingVector &new_pointingVector) const noexcept {
}

std::string CableWrap_HaDc::cableWrapFlag(const PointingVector &pointingVector) const noexcept {
    return "-";
}

bool CableWrap_HaDc::unwrapAzInSection(PointingVector &pv, char section) const noexcept {
    return section == '-';
}

std::pair<std::string, std::string> CableWrap_HaDc::getMotions() const noexcept {
    return {"ha","dec"};
}

std::string CableWrap_HaDc::vexPointingSectors() const noexcept {
    return pointingSector("ha","dec",'-');
}