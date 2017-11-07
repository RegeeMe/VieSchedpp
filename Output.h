/**
 * @file Output.h
 * @brief class Output
 *
 *
 * @author Matthias Schartner
 * @date 22.08.2017
 */

#ifndef OUTPUT_H
#define OUTPUT_H
#include "Scheduler.h"
#include "boost/format.hpp"

namespace VieVS{

    /**
     * @class Output
     * @brief this is the VLBI output class that creates all output files
     *
     * @author Matthias Schartner
     * @date 22.08.2017
     */
    class Output {
    public:
        /**
         * @brief empty default constructor
         */
        Output();

        /**
         * @brief constructor
         *
         * @param sched scheduler
         */
        explicit Output(Scheduler &sched);

        /**
         * @brief sets the number of this schedule
         *
         * If you do not use multi scheduling the number is 0, otherwise the multi scheduling number is used.
         *
         * @param isched number of this schedule
         */
        void setIsched(int isched) {
            Output::iSched_ = isched;
        }

        /**
         * @brief writes a summary text file containing some basic statistics and overviews
         *
         * @param general write general block
         * @param station write station based block
         * @param source write source based block
         * @param baseline write baseline based block
         * @param duration write duration based block
         */
        void writeStatistics(bool general, bool station, bool source, bool baseline, bool duration);

        /**
         * @brief create a ngs file
         */
        void writeNGS();


        /**
         * @brief create a skd file
         */
        void writeSkd(const SkdCatalogReader &skdCatalogReader);


    private:
        boost::property_tree::ptree xml_; ///< content of parameters.xml file

        int iSched_; ///< number of this schedule
        std::vector<Station> stations_; ///< all stations
        std::vector<Source> sources_; ///< all sources
        std::vector<SkyCoverage> skyCoverages_; ///< all sky coverages
        std::vector<Scan> scans_; ///< all scans in schedule

        /**
         * @brief displays some general statistics of the schedule
         *
         * @param out outsteam file object
         */
        void displayGeneralStatistics(std::ofstream &out);

        /**
         * @brief displays some baseline dependent statistics of the schedule
         *
         * @param out outsteam file object
         */
        void displayBaselineStatistics(std::ofstream &out);

        /**
         * @brief displays some station dependent statistics of the schedule
         *
         * @param out outsteam file object
         */
        void displayStationStatistics(std::ofstream &out);

        /**
         * @brief displays some source dependent statistics of the schedule
         *
         * @param out outsteam file object
         */
        void displaySourceStatistics(std::ofstream &out);

        /**
         * @brief displays some source dependent statistics of the schedule
         *
         * @param out outsteam file object
         */
        void displayScanDurationStatistics(std::ofstream &out);

        /**
         * @brief write skd $PARAM block
         *
         * @param skdCatalogReader catalog reader
         * @param ofstream out stream
         */
        void skd_PARAM(const SkdCatalogReader &skdCatalogReader, std::ofstream &ofstream);

        /**
         * @brief write skd $OP block
         *
         * @param ofstream out stream
         */
        void skd_OP(std::ofstream &ofstream);

        /**
         * @brief write skd $DOWNTIME block
         *
         * @param ofstream out stream
         */
        void skd_DOWNTIME(std::ofstream &ofstream);

        /**
         * @brief write skd $MAJOR block
         *
         * @param skdCatalogReader catalog reader
         * @param ofstream out stream
         */
        void skd_MAJOR(const SkdCatalogReader &skdCatalogReader, std::ofstream &ofstream);

        /**
         * @brief write skd $MINOR block
         *
         * @param ofstream out stream
         */
        void skd_MINOR(std::ofstream &ofstream);

        /**
         * @brief write skd $ASTROMETRIC block
         *
         * @param ofstream out stream
         */
        void skd_ASTROMETRIC(std::ofstream &ofstream);

        /**
         * @brief write skd %STATWT block
         *
         * @param of out stream
         */
        void skd_STATWT(std::ofstream &of);

        /**
         * @brief write skd $SRCWT block
         *
         * @param of out stream
         */
        void skd_SRCWT(std::ofstream &of);

        /**
         * @brief write skd $CATALOG_USED block
         *
         * @param of out stream
         */
        void skd_CATALOG_USED(std::ofstream &of);

        /**
         * @brief write skd $BROADBAND block
         *
         * @param ofstream out stream
         */
        void skd_BROADBAND(std::ofstream &ofstream);

        /**
         * @brief write skd $SOURCES block
         *
         * @param skdCatalogReader catalog reader
         * @param of out stream
         */
        void skd_SOURCES(const SkdCatalogReader &skdCatalogReader, std::ofstream &of);

        /**
         * @brief write skd $STATIONS block
         * @param skdCatalogReader catalog reader
         * @param of out stream
         */
        void skd_STATIONS(const SkdCatalogReader &skdCatalogReader, std::ofstream &of);

        /**
         * @brief write skd $FLUX block
         *
         * @param skdCatalogReader catalog reader
         * @param of out stream
         */
        void skd_FLUX(const SkdCatalogReader &skdCatalogReader, std::ofstream &of);

        /**
         * @brief write skd $SKED block
         *
         * @param skdCatalogReader catalog reader
         * @param of out stream
         */
        void skd_SKED(const SkdCatalogReader &skdCatalogReader, std::ofstream &of);

        /**
         * @brief write skd $CODES block
         *
         * @param skdCatalogReader catalog reader
         * @param of out stream
         */
        void skd_CODES(const SkdCatalogReader &skdCatalogReader, std::ofstream &of);
    };
}


#endif //OUTPUT_H