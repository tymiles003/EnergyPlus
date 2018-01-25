// EnergyPlus, Copyright (c) 1996-2017, The Board of Trustees of the University of Illinois,
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

// EnergyPlus::ElectricPowerServiceManager Unit Tests
#include <map>
#include <string>

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include <PVWatts.hh>
#include <DataSurfaces.hh>
#include <DataHVACGlobals.hh>
#include <DataGlobals.hh>
#include <DataEnvironment.hh>
#include <WeatherManager.hh>
#include <ElectricPowerServiceManager.hh>

#include "Fixtures/EnergyPlusFixture.hh"

using namespace EnergyPlus;

TEST_F( EnergyPlusFixture, PVWattsGenerator_Constructor )
{
	using namespace PVWatts;

	PVWattsGenerator pvw("PVArray", 4000.0, ModuleType::STANDARD, ArrayType::FIXED_ROOF_MOUNTED);
	EXPECT_DOUBLE_EQ(4000.0, pvw.getDCSystemCapacity());
	EXPECT_EQ(ModuleType::STANDARD, pvw.getModuleType());
	EXPECT_EQ(ArrayType::FIXED_ROOF_MOUNTED, pvw.getArrayType());
	EXPECT_DOUBLE_EQ(0.14, pvw.getSystemLosses());
	EXPECT_EQ(GeometryType::TILT_AZIMUTH, pvw.getGeometryType());
	EXPECT_DOUBLE_EQ(20.0, pvw.getTilt());
	EXPECT_DOUBLE_EQ(180.0, pvw.getAzimuth());
	EXPECT_DOUBLE_EQ(0.4, pvw.getGroundCoverageRatio());

	ASSERT_THROW(PVWattsGenerator pvw2("", -1000.0, ModuleType::PREMIUM, ArrayType::FIXED_OPEN_RACK, 1.1, GeometryType::TILT_AZIMUTH, 91.0, 360.0, 0, -0.1), std::runtime_error);
	std::string const error_string = delimited_string({
		"   ** Severe  ** PVWatts: name cannot be blank.",
		"   ** Severe  ** PVWatts: DC system capacity must be greater than zero.",
		"   ** Severe  ** PVWatts: Invalid system loss value 1.10",
		"   ** Severe  ** PVWatts: Invalid tilt: 91.00",
		"   ** Severe  ** PVWatts: Invalid azimuth: 360.00",
		"   ** Severe  ** PVWatts: Invalid ground coverage ratio: -0.10",
		"   **  Fatal  ** Errors found in getting PVWatts input",
		"   ...Summary of Errors that led to program termination:",
		"   ..... Reference severe error count=6",
		"   ..... Last severe error=PVWatts: Invalid ground coverage ratio: -0.10"
	});
	EXPECT_TRUE( compare_err_stream( error_string, true ) );
}

TEST_F( EnergyPlusFixture, PVWattsGenerator_GetInputs )
{
	using namespace PVWatts;
	const std::string idfTxt = delimited_string({
		"Version, 8.9;",
		"Generator:PVWatts,",
		"PVWattsArray1,",
		"5,",
		"4000,",
		"Premium,",
		"OneAxis,",
		",",
		",",
		",",
		";",
		"Generator:PVWatts,",
		"PVWattsArray2,",
		"5,",
		"4000,",
		"Premium,",
		"OneAxis,",
		",",
		",",
		",",
		",",
		",",
		";",
		"Generator:PVWatts,",
		"PVWattsArray3,",
		"5,",
		"4000,",
		"Premium,",
		"OneAxis,",
		",",
		",",
		"21,",
		"175,",
		",",
		"0.5;",
		"Output:Variable,*,Generator Produced DC Electric Power,timestep;"
	});
	process_idf(idfTxt);
	EXPECT_FALSE(has_err_output());
	PVWattsGenerator &pvw1 = GetOrCreatePVWattsGenerator("PVWattsArray1");
	EXPECT_EQ(pvw1.getModuleType(), ModuleType::PREMIUM);
	EXPECT_EQ(pvw1.getArrayType(), ArrayType::ONE_AXIS);
	EXPECT_DOUBLE_EQ(0.4, pvw1.getGroundCoverageRatio());
	PVWattsGenerator &pvw2 = GetOrCreatePVWattsGenerator("PVWattsArray2");
	EXPECT_DOUBLE_EQ(0.4, pvw2.getGroundCoverageRatio());
	PVWattsGenerator &pvw3 = GetOrCreatePVWattsGenerator("PVWattsArray3");
	EXPECT_DOUBLE_EQ(175.0, pvw3.getAzimuth());
	EXPECT_DOUBLE_EQ(21.0, pvw3.getTilt());
	EXPECT_DOUBLE_EQ(0.5, pvw3.getGroundCoverageRatio());
	EXPECT_EQ(static_cast<int>(PVWattsGenerators.size()), 3);

}

TEST_F( EnergyPlusFixture, PVWattsGenerator_Calc )
{
	using namespace PVWatts;
	// USA_AZ_Phoenix-Sky.Harbor.Intl.AP.722780_TMY3.epw
	// 6/15 at 7am
	DataGlobals::TimeStep = 1;
	DataGlobals::TimeStepZone = 1.0;
	DataHVACGlobals::TimeStepSys = 1.0;
	DataGlobals::BeginTimeStepFlag = true;
	DataGlobals::MinutesPerTimeStep = 60;
	DataGlobals::NumOfTimeStepInHour = 1;
	WeatherManager::AllocateWeatherData(); // gets us the albedo array initialized
	DataEnvironment::Year = 1986;
	DataEnvironment::Month = 6;
	DataEnvironment::DayOfMonth = 15;
	DataGlobals::HourOfDay = 8; // 8th hour of day, 7-8am
	WeatherManager::WeatherFileLatitude = 33.45;
	WeatherManager::WeatherFileLongitude = -111.98;
	WeatherManager::WeatherFileTimeZone = -7;
	DataEnvironment::BeamSolarRad = 728;
	DataEnvironment::DifSolarRad = 70;
	DataEnvironment::WindSpeed = 3.1;
	DataEnvironment::OutDryBulbTemp = 31.7;

	PVWattsGenerator pvw("PVWattsArrayA", 4000.0, ModuleType::STANDARD, ArrayType::FIXED_ROOF_MOUNTED);
	pvw.setCellTemperature(30.345);
	pvw.setPlaneOfArrayIrradiance(92.257);
	pvw.calc();
	Real64 generatorPower, generatorEnergy, thermalPower, thermalEnergy;
	pvw.getResults(generatorPower, generatorEnergy, thermalPower, thermalEnergy);
	EXPECT_DOUBLE_EQ(thermalPower, 0.0);
	EXPECT_DOUBLE_EQ(thermalEnergy, 0.0);
	EXPECT_NEAR(generatorPower, 884.137, 0.001);
	EXPECT_NEAR(generatorEnergy, 884.137 * 60 * 60, 1);

}

TEST_F( EnergyPlusFixture, PVWattsInverter_Constructor )
{
	const std::string idfTxt = delimited_string({
		"Version, 8.9;",
		"ElectricLoadCenter:Distribution,",
		"ELC1,",
		"GeneratorList1,",
		"Baseload,",
		",",
		",",
		",",
		"DirectCurrentWithInverter,",
		"Inverter1;",
		"ElectricLoadCenter:Inverter:PVWatts,",
		"Inverter1,",
		"1.10,",
		"0.96;",
		"ElectricLoadCenter:Generators,",
		"GeneratorList1,",
		"PVWattsArray1,",
		"Generator:PVWatts,",
		"1500,",
		",",
		",",
		"PVWattsArray2,",
		"Generator:PVWatts,",
		"2500,",
		",",
		";",
		"Generator:PVWatts,",
		"PVWattsArray1,",
		"5,",
		"1500,",
		"Standard,",
		"OneAxis,",
		",",
		",",
		",",
		";",
		"Generator:PVWatts,",
		"PVWattsArray2,",
		"5,",
		"2500,",
		"Standard,",
		"OneAxis,",
		",",
		",",
		",",
		",",
		",",
		";"
	});
	ASSERT_FALSE(process_idf(idfTxt));
	auto eplc(ElectPowerLoadCenter(1));
	ASSERT_TRUE(eplc.inverterPresent);
	EXPECT_DOUBLE_EQ(eplc.inverterObj->pvWattsDCCapacity(), 4000.0);
	DataHVACGlobals::TimeStepSys = 1.0;
	eplc.inverterObj->simulate(884.018);
	EXPECT_NEAR(eplc.inverterObj->aCPowerOut(), 842.527, 0.001);

}
