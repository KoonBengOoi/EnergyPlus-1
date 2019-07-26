// EnergyPlus, Copyright (c) 1996-2019, The Board of Trustees of the University of Illinois,
// The Regents of the University of California, through Lawrence Berkeley National Laboratory
// (subject to receipt of any required approvals from the U.S. Dept. of Energy), Oak Ridge
// National Laboratory, managed by UT-Battelle, Alliance for Sustainable Energy, LLC, and other
// contributors. All rights reserved.
//
// NOTICE: This Software was developed under funding from the U.S. Department of Energy and the
// U.S. Government consequently retains certain rights. As such, the U.S. Government has been
// granted for itself and others acting on its behalf a paid-up, nonexclusive, irrevocable,
// worldwide license in the Software to reproduce, distribute copies to the public, prepare
// derivative works, and perform publicly and display publicly, and to permit others to do so.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted
// provided that the following conditions are met:
//
// (1) Redistributions of source code must retain the above copyright notice, this list of
//     conditions and the following disclaimer.
//
// (2) Redistributions in binary form must reproduce the above copyright notice, this list of
//     conditions and the following disclaimer in the documentation and/or other materials
//     provided with the distribution.
//
// (3) Neither the name of the University of California, Lawrence Berkeley National Laboratory,
//     the University of Illinois, U.S. Dept. of Energy nor the names of its contributors may be
//     used to endorse or promote products derived from this software without specific prior
//     written permission.
//
// (4) Use of EnergyPlus(TM) Name. If Licensee (i) distributes the software in stand-alone form
//     without changes from the version obtained under this License, or (ii) Licensee makes a
//     reference solely to the software portion of its product, Licensee must refer to the
//     software as "EnergyPlus version X" software, where "X" is the version number Licensee
//     obtained under this License and may not use a different name for the software. Except as
//     specifically required in this Section (4), Licensee shall not use in a company name, a
//     product name, in advertising, publicity, or other promotional activities any name, trade
//     name, trademark, logo, or other designation of "EnergyPlus", "E+", "e+" or confusingly
//     similar designation, without the U.S. Department of Energy's prior written consent.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#include <Scheduling/Base.hh>
#include <Scheduling/TypeLimits.hh>
#include <InputProcessing/InputProcessor.hh>
#include <UtilityRoutines.hh>

namespace Scheduling {
std::vector<ScheduleTypeData> scheduleTypeLimits;
// bool needToGetInput = true;

ScheduleTypeData *ScheduleTypeData::factory(const std::string& name)
{
    // call this in the right order and we don't have to check this everytime...
    // just make SURE that the schedule input manager calls the type limits input processor before anything else
//    if (needToGetInput) {
//        ScheduleTypeData::processInput();
//    }
    for (auto & thisTypeLimit : scheduleTypeLimits) {
        if (thisTypeLimit.name == name) {
            return &thisTypeLimit;
        }
    }
    // ShowFatalError("Could not find type limits object with name: " + name);
    return nullptr;
}

void ScheduleTypeData::processInput() {
    std::string const thisObjectType = "ScheduleTypeLimits";
    auto const instances = EnergyPlus::inputProcessor->epJSON.find(thisObjectType);
    if (instances == EnergyPlus::inputProcessor->epJSON.end()) {
        return; // no schedule type limits to process
    }
    auto &instancesValue = instances.value();
    for (auto instance = instancesValue.begin(); instance != instancesValue.end(); ++instance) {
        auto const &fields = instance.value();
        auto const &thisObjectName = EnergyPlus::UtilityRoutines::MakeUPPERCase(instance.key());
        // do any pre-construction operations
        EnergyPlus::inputProcessor->markObjectAsUsed(thisObjectType, thisObjectName);
        if (std::find(Scheduling::allSchedNames.begin(), Scheduling::allSchedNames.end(), thisObjectName) != Scheduling::allSchedNames.end()) {
            EnergyPlus::ShowFatalError("Duplicate schedule name, all schedules, across all schedule types, must be uniquely named");
        }
        //EnergyPlus::GlobalNames::VerifyUniqueInterObjectName(UniqueScheduleNames, Alphas(1), CurrentModuleObject, cAlphaFields(1), ErrorsFound);
        // then just add it to the vector via the constructor
        scheduleTypeLimits.emplace_back(thisObjectName, fields);
    }
}

void ScheduleTypeData::clear_state() {
    scheduleTypeLimits.clear();
    //needToGetInput = true;
}

void toUpper(std::string &s) {
    std::for_each(s.begin(), s.end(), [](char & c) {
        c = ::toupper(c);
    });
}

ScheduleTypeData::ScheduleTypeData(std::string const &objectName, nlohmann::json const &fields) {
    this->name = EnergyPlus::UtilityRoutines::MakeUPPERCase(objectName);
    if (fields.find("lower_limit_value") != fields.end()) {
        this->minimum = fields.at("lower_limit_value");
    }
    if (fields.find("upper_limit_value") != fields.end()) {
        this->maximum = fields.at("upper_limit_value");
    }
    if (fields.find("numeric_type") != fields.end()) {
        std::string thisType = fields.at("numeric_type");
        toUpper(thisType);
        if (thisType == "DISCRETE" || thisType == "INTEGER") {
            this->isContinuous = false;
        } else if (thisType == "CONTINUOUS" || thisType == "REAL") {
            this->isContinuous = true;
        } else {
            //ShowSevereError - bad inputs
        }
    }
}

}