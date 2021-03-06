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

#include "Bbc.h"

using namespace VieVS;
using namespace std;

unsigned long VieVS::Bbc::nextId = 0;
unsigned long VieVS::Bbc::Bbc_assign::nextId = 0;

Bbc::Bbc(std::string name): VieVS_NamedObject{std::move(name), nextId++} {

}

void Bbc::addBbc(std::string name, unsigned int physical_bbc_number, unsigned int if_name) {

    bbc_assigns_.emplace_back(name, physical_bbc_number, if_name);

}
