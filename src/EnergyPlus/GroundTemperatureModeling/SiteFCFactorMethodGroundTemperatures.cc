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

// C++ Headers
#include <memory>

// ObjexxFCL Headers
#include <ObjexxFCL/gio.hh>

// EnergyPlus Headers
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/GroundTemperatureModeling/GroundTemperatureModelManager.hh>
#include <EnergyPlus/GroundTemperatureModeling/SiteFCFactorMethodGroundTemperatures.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/WeatherManager.hh>

namespace EnergyPlus {

static ObjexxFCL::gio::Fmt fmtA("(A)");
static ObjexxFCL::gio::Fmt fmtAN("(A,$)");

//******************************************************************************

// Site:GroundTemperature:FCFactorMethod factory
std::shared_ptr<SiteFCFactorMethodGroundTemps> SiteFCFactorMethodGroundTemps::FCFactorGTMFactory(int objectType, std::string objectName)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Matt Mitchell
    //       DATE WRITTEN   Summer 2015
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // Reads input and creates instance of Site:GroundTemperature:FCfactorMethod object

    // USE STATEMENTS:
    using DataEnvironment::FCGroundTemps;
    using DataGlobals::OutputFileInits;
    using WeatherManager::GroundTempsFCFromEPWHeader;
    using WeatherManager::wthFCGroundTemps;
    using namespace DataIPShortCuts;
    using namespace GroundTemperatureManager;
    using namespace ObjexxFCL::gio;

    // Locals
    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    bool found = false;
    int NumNums;
    int NumAlphas;
    int IOStat;

    // New shared pointer for this model object
    std::shared_ptr<SiteFCFactorMethodGroundTemps> thisModel(new SiteFCFactorMethodGroundTemps());

    std::string const cCurrentModuleObject = CurrentModuleObjects(objectType_SiteFCFactorMethodGroundTemp);
    int numCurrObjects = inputProcessor->getNumObjectsFound(cCurrentModuleObject);

    thisModel->objectType = objectType;
    thisModel->objectName = objectName;

    if (numCurrObjects == 1) {

        // Get the object names for each construction from the input processor
        inputProcessor->getObjectItem(cCurrentModuleObject, 1, cAlphaArgs, NumAlphas, rNumericArgs, NumNums, IOStat);

        if (NumNums < 12) {
            ShowSevereError(cCurrentModuleObject + ": Less than 12 values entered.");
            thisModel->errorsFound = true;
        }

        // overwrite values read from weather file for the 0.5m set ground temperatures
        for (int i = 1; i <= 12; ++i) {
            thisModel->fcFactorGroundTemps(i) = rNumericArgs(i);
        }

        FCGroundTemps = true;
        found = true;

    } else if (numCurrObjects > 1) {
        ShowSevereError(cCurrentModuleObject + ": Too many objects entered. Only one allowed.");
        thisModel->errorsFound = true;

    } else if (wthFCGroundTemps) {

        for (int i = 1; i <= 12; ++i) {
            thisModel->fcFactorGroundTemps(i) = GroundTempsFCFromEPWHeader(i);
        }

        FCGroundTemps = true;
        found = true;

    } else {
        thisModel->fcFactorGroundTemps = 0.0;
        found = true;
    }

    // Write Final Ground Temp Information to the initialization output file
    if (FCGroundTemps) {
        ObjexxFCL::gio::write(OutputFileInits, fmtA)
            << "! <Site:GroundTemperature:FCfactorMethod>,Jan{C},Feb{C},Mar{C},Apr{C},May{C},Jun{C},Jul{C},Aug{C},Sep{C},Oct{C},Nov{C},Dec{C}";
        ObjexxFCL::gio::write(OutputFileInits, fmtAN) << " Site:GroundTemperature:FCfactorMethod";
        for (int i = 1; i <= 12; ++i) {
            ObjexxFCL::gio::write(OutputFileInits, "(', ',F6.2,$)") << thisModel->fcFactorGroundTemps(i);
        }
        ObjexxFCL::gio::write(OutputFileInits);
    }

    if (found && !thisModel->errorsFound) {
        groundTempModels.push_back(thisModel);
        return thisModel;
    } else {
        ShowContinueError("Site:GroundTemperature:FCFactorMethod--Errors getting input for ground temperature model");
        return nullptr;
    }
}

//******************************************************************************

Real64 SiteFCFactorMethodGroundTemps::getGroundTemp()
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Matt Mitchell
    //       DATE WRITTEN   Summer 2015
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // Returns the ground temperature for Site:GroundTemperature:FCFactorMethod

    return fcFactorGroundTemps(timeOfSimInMonths);
}

//******************************************************************************

Real64 SiteFCFactorMethodGroundTemps::getGroundTempAtTimeInSeconds(Real64 const EP_UNUSED(_depth), Real64 const _seconds)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Matt Mitchell
    //       DATE WRITTEN   Summer 2015
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // Returns the ground temperature when input time is in seconds

    // USE STATEMENTS:
    using DataGlobals::SecsInDay;
    using WeatherManager::NumDaysInYear;

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Real64 secPerMonth = NumDaysInYear * SecsInDay / 12;

    // Convert secs to months
    int month = ceil(_seconds / secPerMonth);

    if (month >= 1 && month <= 12) {
        timeOfSimInMonths = month;
    } else {
        timeOfSimInMonths = remainder(month, 12);
    }

    // Get and return ground temp
    return getGroundTemp();
}

//******************************************************************************

Real64 SiteFCFactorMethodGroundTemps::getGroundTempAtTimeInMonths(Real64 const EP_UNUSED(_depth), int const _month)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Matt Mitchell
    //       DATE WRITTEN   Summer 2015
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // Returns the ground temperature when input time is in months

    // Set month
    if (_month >= 1 && _month <= 12) {
        timeOfSimInMonths = _month;
    } else {
        timeOfSimInMonths = remainder(_month, 12);
    }

    // Get and return ground temp
    return getGroundTemp();
}

//******************************************************************************

} // namespace EnergyPlus
