//
// Created by matth on 28.01.2018.
//
#ifndef VIEVS_SCHEDULER_H
#define VIEVS_SCHEDULER_H

#include <cstdlib>
#include <chrono>
#include <vector>
#include <boost/format.hpp>
#include <iostream>
#include <thread>

#ifdef VIESCHEDPP_LOG
#include <boost/log/trivial.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/attributes/named_scope.hpp>
#endif

#include "Initializer.h"
#include "Scheduler.h"
#include "Output.h"
#include "ParameterSettings.h"
#include "HighImpactScanDescriptor.h"


#ifdef _OPENMP
#include <omp.h>
#endif

namespace VieVS {

    class VieSchedpp {
    public:
        VieSchedpp() = default;

        explicit VieSchedpp(const std::string &inputFile);

        void run();


    private:
        std::string inputFile_;
        std::string path_;
        std::string fileName_;
        boost::property_tree::ptree xml_; ///< content of parameters.xml file

        SkdCatalogReader skdCatalogs_;
        std::vector<VieVS::MultiScheduling::Parameters> multiSchedParameters_;

        void readSkdCatalogs();

        #ifdef _OPENMP
        void multiCoreSetup();
        #endif

        #ifdef VIESCHEDPP_LOG
        void init_log();
        #endif

    };
}

#endif //VIEVS_SCHEDULER_H
