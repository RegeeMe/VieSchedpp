/* 
 *  VieSched++ Very Long Baseline Interferometry (VLBI) Scheduling Software
 *  Copyright (C) 2018  Matthias Schartner
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file Station.h
 * @brief class Station
 *
 * @author Matthias Schartner
 * @date 21.06.2017
 */

#ifndef STATION_H
#define STATION_H
#include <iostream>
#include <fstream>

#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <utility>
#include <memory>

#include "Position.h"
#include "AbstractAntenna.h"
#include "AbstractCableWrap.h"
#include "Equipment.h"
#include "AbstractHorizonMask.h"
#include "Source.h"
#include "PointingVector.h"
#include "Constants.h"
#include "TimeSystem.h"
#include "AstronomicalParameters.h"

#include "sofa.h"
#include "VieVS_NamedObject.h"


namespace VieVS{
    /**
     * @class Station
     * @brief representation of a VLBI station
     *
     * First all station objects must be created, usually via the VLBI_initializer::createStationsFromCatalogs()
     * Afterwards stations need to be initialized via VLBI_initializer::initializeStations()
     *
     * @author Matthias Schartner
     * @date 21.06.2017
     */
    class Station: public VieVS_NamedObject {
    public:

        /**
         * @brief azimuth elevation calculation model
         * @author Matthias Schartner
         */
        enum class AzelModel {
            simple, ///< simple model without nutation
            rigorous ///< rigorous model
        };

        /**
         * @brief station parameters
         * @author Matthias Schartner
         */
        class Parameters: public VieVS_NamedObject{
        private:
            static unsigned long nextId;  ///< next id for this object type
        public:

            /**
             * @brief constructor
             * @author Matthias Schartner
             *
             * @param name parameter name
             */
            explicit Parameters(const std::string &name):VieVS_NamedObject(name,nextId++){}

            /**
             * @brief set parameters from other
             * @author Matthias Schartner
             *
             * @param other source parameters
             */
            void setParameters(const Parameters &other);

            bool firstScan = false; ///< if set to true: no time is spend for setup, source, tape, calibration, and slewing
            bool available = true;  ///< if set to true: this station is available for a scan
            bool tagalong = false;  ///< if set to true: station is in tagalong mode
            bool availableForFillinmode = true; ///< if set to true: station is available for fillin modes

            double weight = 1; ///< multiplicative factor of score for scans with this station
            double minElevation = 5*deg2rad; /// minimum elevation in radians

            std::unordered_map<std::string, double> minSNR; ///< minimum required signal to noise ration for each band

            unsigned int maxSlewtime = 600; ///< maximum allowed slewtime in seconds
            double maxSlewDistance = 175 * deg2rad; ///< maximum allowed slew distance in radians
            double minSlewDistance = 0; ///< minimum allowed slew distance in radians
            unsigned int maxWait = 600; ///< maximum allowed wait time for slow antennas in seconds
            unsigned int maxScan = 600; ///< maximum allowed scan time in seconds
            unsigned int minScan = 20; ///< minimum required scan time in seconds
            unsigned int maxNumberOfScans = 9999; ///< maximum allowed number of scans

            std::vector<unsigned long> ignoreSources; ///< list of all source ids which should be ignored

            /**
             * @brief output of the curren parameters to out stream
             * @author Matthias Schartner
             *
             * @param of out stream object
             */
            void output(std::ofstream &of) const {
                if (available) {
                    of << "    available: TRUE\n";
                } else {
                    of << "    available: FALSE\n";
                }

                of << "    maxSlewtime: " << maxSlewtime << "\n";
                of << "    maxWait:     " << maxWait << "\n";
                of << "    maxScan:     " << maxScan << "\n";
                of << "    minScan:     " << minScan << "\n";
                of << "    weight:      " << weight << "\n";
                of << "    minElevation " << minElevation << "\n";

                for (const auto &it:minSNR) {
                    of << "    minSNR: " << it.first << " " << it.second << "\n";
                }

                if (!ignoreSources.empty()) {
                    of << "    ignoreSources:";
                    for (unsigned long ignoreSource : ignoreSources) {
                        of << " " << ignoreSource;
                    }
                    of << "\n";
                }
            }
        };

        /**
         * @brief wait times for field system and correlator synchronization
         * @author Matthias Schartner
         */
        struct WaitTimes {
            unsigned int fieldSystem = 6; ///< time required for field system commands
            unsigned int preob = 10; ///< time required for calibration
            unsigned int midob = 3; ///< extra observing time for correlator synchronization
            unsigned int postob = 0; ///< postob time
        };

        /**
         * @brief setter for wait times
         * @author Matthias Schartner
         *
         * @param waittimes new wait times
         */
        void setWaitTimes(WaitTimes &waittimes) {
            Station::waitTimes_ = std::make_shared<WaitTimes>(std::move(waittimes));
        }

        /**
         * @brief changes in parameters
         * @author Matthias Schartner
         */
        struct Event {
            /**
             * @brief constructor
             * @author Matthias Schartner
             *
             * @param time event time
             * @param softTransition transition type
             * @param PARA parameter
             */
            Event(unsigned int time, bool softTransition, Parameters PARA): time{time},
                                                                            softTransition{softTransition},
                                                                            PARA{std::move(PARA)}{}

            unsigned int time; ///< time when new parameters should be used in seconds since start
            bool softTransition; ///< transition type
            Parameters PARA; ///< new parameters
        };


        /**
         * @brief statistics
         * @author Matthias Schartner
         */
        struct Statistics{
            std::vector<unsigned int> scanStartTimes{}; ///< list of scan start times
            int totalObservingTime{0}; ///< integrated observing time
            int totalSlewTime{0}; ///< integrated slew time
            int totalIdleTime{0}; ///< integrated idle time
            int totalFieldSystemTime{0}; ///< integrated field system time
            int totalPreobTime{0}; ///< integrated calibration time
        };

        /**
         * @brief constructor
         * @author Matthias Schartner
         *
         * @param sta_name station name
         * @param tlc two letter code
         * @param sta_antenna station antenna
         * @param sta_cableWrap station cable wrap
         * @param sta_position station position
         * @param sta_equip station equipment
         * @param sta_mask station horizon mask
         * @param nSource number of sources
         */
        Station(std::string sta_name, std::string tlc, std::shared_ptr<AbstractAntenna> sta_antenna,
                std::shared_ptr<AbstractCableWrap> sta_cableWrap, std::shared_ptr<Position> sta_position,
                std::shared_ptr<Equipment> sta_equip, std::shared_ptr<AbstractHorizonMask> sta_mask,
                unsigned long nSources);


        /**
         * @brief getter for parameters
         * @author Matthias Schartner
         *
         * @return currently used parameters
         */
        const Parameters &getPARA() const {
            return parameters_;
        }

        /**
         * @brief reference to current parameters
         * @author Matthias Schartner
         *
         * @return reference of current parameters
         */
        Parameters &referencePARA() {
            return parameters_;
        }

        /**
         * @brief getter for wait times
         * @author Matthias Schartner
         *
         * @return station wait times
         */
        const WaitTimes &getWaittimes() const {
            return *waitTimes_;
        }


        /**
         * @brief getter for cable wrap
         * @author Matthias Schartner
         *
         * @return cable wrap of this station
         */
        const AbstractCableWrap &getCableWrap() const noexcept {
            return *cableWrap_;
        }

        /**
         * @brief reference to cable wrap
         * @author Matthias Schartner
         *
         * @return cable wrap of this station
         */
        AbstractCableWrap &referenceCableWrap() noexcept {
            return *cableWrap_;
        }

        /**
         * @brief getter for last time this antenna was mentioned in scheduling
         * @author Matthias Schartner
         *
         * @return last station usage time in seconds since session start
         */
        unsigned int getCurrentTime() const noexcept {
            return currentPositionVector_.getTime();
        }

        /**
         * @brief getter for equipment information
         * @author Matthias Schartner
         *
         * @return equipment objecct
         */
        const Equipment &getEquip() const noexcept {
            return *equip_;
        }

        /**
         * @brief check if station has horizon mask
         * @author Matthias Schartner
         *
         * @return flag if horizon mask is defined
         */
        bool hasHorizonMask() const{
            return mask_ !=  nullptr;
        }

        /**
         * @brief getter for horizon mask
         * @author Matthias Schartner
         *
         * @return horizon mask object
         */
        const AbstractHorizonMask &getMask() const {
            return *mask_;
        }

        /**
         * @brief getter for antenna
         * @author Matthias Schartner
         *
         * @return antenna object
         */
        const AbstractAntenna &getAntenna() const noexcept {
            return *antenna_;
        }

        /**
         * @brief getter for position
         * @author Matthias Schartner
         *
         * @return position object
         */
        const Position &getPosition() const noexcept {
            return *position_;
        }

        /**
         * @brief getter for number of baselines which were already observed with this station
         * @author Matthias Schartner
         *
         * @return number of already observed baselines
         */
        int getNObs() const noexcept {
            return nObs_;
        }

        /**
         * @brief get number of scans
         * @author Matthias Schartner
         *
         * @return number of scans
         */
        int getNScans() const noexcept  {
            return nScans_;
        }

        /**
         * @brief get number of total scans
         * @author Matthias Schartner
         *
         * @return number of total scans
         */
        int getNTotalScans() const noexcept{
            return nTotalScans_;
        }

        /**
         * @brief getter for current pointing vector
         * @author Matthias Schartner
         *
         * @return current pointing vector
         */
        const PointingVector &getCurrentPointingVector() const noexcept {
            return currentPositionVector_;
        }

        /**
         * @brief distance between two stations
         * @author Matthias Schartner
         *
         * @param other other station
         * @return distance between this two stations
         */
        double distance(const Station &other) const noexcept;

        /**
         * @brief checks if a source is visible for this station
         * @author Matthias Schartner
         *
         * @param p pointing vector which holds time information and will be filled with azimuth and elevation information
         * @param minElevationSource minimum elevation for this source
         * @return true if station is visible
         */
        bool isVisible(const PointingVector &p, double minElevationSource = 0) const noexcept;

        /**
         * @brief calculate slew time between current pointing vector and this pointing vector
         * @author Matthias Schartner
         *
         * If this is the first scan of this station the slew time is zero.
         *
         * @param pointingVector slew end position
         * @return slew time in seconds
         */
        boost::optional<unsigned int> slewTime(const PointingVector &pointingVector) const noexcept;

        /**
         * @brief calculation of azimuth, elevation, hour angle and declination with rigorouse model
         * @author Matthias Schartner
         *
         * @param source observed source
         * @param p pointing vector
         */
        void calcAzEl_rigorous(const Source &source, PointingVector &p) noexcept;

        /**
         * @brief calculation of azimuth, elevation, hour angle and declination with lookup tables
         * @author Matthias Schartner
         *
         * @param source observed source
         * @param p pointing vector
         */
        void calcAzEl_simple(const Source &source, PointingVector &p) const noexcept;

        /**
         * @brief change current pointing vector
         * @author Matthias Schartner
         *
         * @param pointingVector new current pointing vector
         */
        void setCurrentPointingVector(const PointingVector &pointingVector) noexcept;
        
        /**
         * @brief sets all upcoming events
         * @author Matthias Schartner
         *
         * @param EVENTS all upcoming events
         */
        void setEVENTS(std::vector<Event> &EVENTS) noexcept {
            Station::events_ = std::make_shared<std::vector<Event>>(move(EVENTS));
            Station::nextEvent_ = 0;
        }

        /**
         * @brief check for tagalong mode
         * @author Matthias Schartner
         *
         * @param time time
         * @return true if tagalong mode required
         */
        bool checkForTagalongMode(unsigned int time) const noexcept;

        /**
         * @brief this function checks if it is time to change the parameters
         *
         * @param time current time in seconds since start
         * @param hardBreak flag if a hard break was found
         */
        bool checkForNewEvent(unsigned int time, bool &hardBreak) noexcept;

        /**
         * @brief maxium allowed observing time
         * @author Matthias Schartner
         *
         * @param ts time stamp
         * @return maxium allowed observing time
         */
        unsigned int maximumAllowedObservingTime(Timestamp ts) const noexcept;

        /**
         * @brief changes parameters to next setup
         * @author Matthias Schartner
         *
         * @param of output stream object
         */
        void applyNextEvent(std::ofstream & of) noexcept;

        /**
         * @brief set next event index
         * @author Matthias Schartner
         *
         * @param idx next event index
         */
        void setNextEvent(unsigned int idx) noexcept{
            nextEvent_ = idx;
        }

        /**
         * @brief update station if used for a scan
         * @author Matthias Schartner
         *
         * @param nbl number of observed baselines
         * @param end pointing vector at end time
         * @param addToStatistics flag if scan should have an influence on the further scheduling process
         */
        void update(unsigned long nbl, const PointingVector &end, bool addToStatistics) noexcept;

        /**
         * @brief clear all observations
         * @author Matthias Schartner
         */
        void clearObservations();

        /**
         * @brief set station statistics
         * @author Matthias Schartner
         *
         * @param stat station statistics
         */
        void setStatistics(const Statistics &stat){
            statistics_ = stat;
        }

        /**
         * @brief get station statistics
         * @author Matthias Schartner
         *
         * @return station statistics
         */
        const Statistics &getStatistics() const {
            return statistics_;
        }

        /**
         * @brief get horizon mask
         * @author Matthias Schartner
         *
         * @return horizon mask
         */
        std::pair<std::vector<double>, std::vector<double>>  getHorizonMask() const noexcept;

    private:
        static unsigned long nextId; ///< next id for this object type

        std::shared_ptr<AbstractAntenna> antenna_; ///< station antenna
        std::shared_ptr<AbstractCableWrap> cableWrap_; ///< station cable wrap
        std::shared_ptr<Position> position_; ///< station position
        std::shared_ptr<Equipment> equip_; ///< station equipment
        std::shared_ptr<AbstractHorizonMask> mask_; ///< station horizon mask
        std::shared_ptr<WaitTimes> waitTimes_; ///< station wait times
        std::shared_ptr<std::vector<Event>> events_; ///< list of all events

        Statistics statistics_; ///< station statistics
        std::vector< std::vector<PointingVector>> azelPrecalc_; ///< pre calculated azimuth elevation lookup table


        Parameters parameters_; ///< station parameters
        PointingVector currentPositionVector_; ///< current pointing vector
        unsigned int nextEvent_{0}; ///< index of next event
        int nScans_{0}; ///< number of participated scans
        int nTotalScans_{0}; ///< number of total scans
        int nObs_{0}; ///< number of observed baselines


    };
}
#endif /* STATION_H */

