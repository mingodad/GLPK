# OSeMOSYS_2017_11_08
#
# Open Source energy MOdeling SYStem
#
# Modified by Francesco Gardumi, Constantinos Taliotis, Igor Tatarewicz, Adrian Levfert
# Main changes to previous version OSeMOSYS_2016_08_01
# Bugs fixed in equations:
#		- Objective function
#		- E5, E8, E9
#		- SV1, SV2
#		- SC2 (both lower and upper)
#		- RE4
# ============================================================================
#
#    Copyright [2010-2015] [OSeMOSYS Forum steering committee see: www.osemosys.org]
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
# ============================================================================
#
#  To run OSeMOSYS, enter the following line into your command prompt after replacing FILEPATH & YOURDATAFILE with your folder structure and data file name:
#
#  C:\...FILEPATH...\glpsol -m C:\...FILEPATH...\osemosys_short.txt -d C:\...FILEPATH...\YOURDATAFILE.txt -o C:\...FILEPATH...\Results.txt
#
#  Alternatively, install GUSEK (http://gusek.sourceforge.net/gusek.html) and run the model within this integrated development environment (IDE).
#  To do so, open the .dat file and select "Use External .dat file" from the Options menu. Then change to the model file and select the "Go" icon or press F5.
#
#                                      #########################################
######################                        Model Definition                                #############
#                                      #########################################
#
###############
#    Sets     #
###############
#
set YEAR;
set TECHNOLOGY;
set TIMESLICE;
set FUEL;
set EMISSION;
set MODE_OF_OPERATION;
set REGION;
set SEASON;
set DAYTYPE;
set DAILYTIMEBRACKET;
set FLEXIBLEDEMANDTYPE;
set STORAGE;
#
#####################
#    Parameters     #
#####################
#
########                        Global                                                 #############
#
param YearSplit{l in TIMESLICE, y in YEAR};
param DiscountRate{r in REGION};
param DaySplit{lh in DAILYTIMEBRACKET, y in YEAR};
param Conversionls{l in TIMESLICE, ls in SEASON};
param Conversionld{l in TIMESLICE, ld in DAYTYPE};
param Conversionlh{l in TIMESLICE, lh in DAILYTIMEBRACKET};
param DaysInDayType{ls in SEASON, ld in DAYTYPE, y in YEAR};
param TradeRoute{r in REGION, rr in REGION, f in FUEL, y in YEAR};
param DepreciationMethod{r in REGION};
#
########                        Demands                                         #############
#
param SpecifiedAnnualDemand{r in REGION, f in FUEL, y in YEAR};
param SpecifiedDemandProfile{r in REGION, f in FUEL, l in TIMESLICE, y in YEAR};
param AccumulatedAnnualDemand{r in REGION, f in FUEL, y in YEAR};
#
#########                        Performance                                        #############
#
param CapacityToActivityUnit{r in REGION, t in TECHNOLOGY};
param TechWithCapacityNeededToMeetPeakTS{r in REGION, t in TECHNOLOGY};
param CapacityFactor{r in REGION, t in TECHNOLOGY, l in TIMESLICE, y in YEAR};
param AvailabilityFactor{r in REGION, t in TECHNOLOGY, y in YEAR};
param OperationalLife{r in REGION, t in TECHNOLOGY};
param ResidualCapacity{r in REGION, t in TECHNOLOGY, y in YEAR};
param InputActivityRatio{r in REGION, t in TECHNOLOGY, f in FUEL, m in MODE_OF_OPERATION, y in YEAR};
param OutputActivityRatio{r in REGION, t in TECHNOLOGY, f in FUEL, m in MODE_OF_OPERATION, y in YEAR};
#
#########                        Technology Costs                        #############
#
param CapitalCost{r in REGION, t in TECHNOLOGY, y in YEAR};
param VariableCost{r in REGION, t in TECHNOLOGY, m in MODE_OF_OPERATION, y in YEAR};
param FixedCost{r in REGION, t in TECHNOLOGY, y in YEAR};
#
#########                           Storage                                 #############
#
param TechnologyToStorage{r in REGION, t in TECHNOLOGY, s in STORAGE, m in MODE_OF_OPERATION};
param TechnologyFromStorage{r in REGION, t in TECHNOLOGY, s in STORAGE, m in MODE_OF_OPERATION};
param StorageLevelStart{r in REGION, s in STORAGE};
param StorageMaxChargeRate{r in REGION, s in STORAGE};
param StorageMaxDischargeRate{r in REGION, s in STORAGE};
param MinStorageCharge{r in REGION, s in STORAGE, y in YEAR};
param OperationalLifeStorage{r in REGION, s in STORAGE};
param CapitalCostStorage{r in REGION, s in STORAGE, y in YEAR};
param ResidualStorageCapacity{r in REGION, s in STORAGE, y in YEAR};
#
#########                        Capacity Constraints                #############
#
param CapacityOfOneTechnologyUnit{r in REGION, t in TECHNOLOGY, y in YEAR};
param TotalAnnualMaxCapacity{r in REGION, t in TECHNOLOGY, y in YEAR};
param TotalAnnualMinCapacity{r in REGION, t in TECHNOLOGY, y in YEAR};
#
#########                        Investment Constraints                #############
#
param TotalAnnualMaxCapacityInvestment{r in REGION, t in TECHNOLOGY, y in YEAR};
param TotalAnnualMinCapacityInvestment{r in REGION, t in TECHNOLOGY, y in YEAR};
#
#########                        Activity Constraints                #############
#
param TotalTechnologyAnnualActivityUpperLimit{r in REGION, t in TECHNOLOGY, y in YEAR};
param TotalTechnologyAnnualActivityLowerLimit{r in REGION, t in TECHNOLOGY, y in YEAR};
param TotalTechnologyModelPeriodActivityUpperLimit{r in REGION, t in TECHNOLOGY};
param TotalTechnologyModelPeriodActivityLowerLimit{r in REGION, t in TECHNOLOGY};
#
#########                        Reserve Margin                                #############
#
param ReserveMarginTagTechnology{r in REGION, t in TECHNOLOGY, y in YEAR};
param ReserveMarginTagFuel{r in REGION, f in FUEL, y in YEAR};
param ReserveMargin{r in REGION, y in YEAR};
#
#########                        RE Generation Target                #############
#
param RETagTechnology{r in REGION, t in TECHNOLOGY, y in YEAR};
param RETagFuel{r in REGION, f in FUEL, y in YEAR};
param REMinProductionTarget{r in REGION, y in YEAR};
#
#########                        Emissions & Penalties                #############
#
param EmissionActivityRatio{r in REGION, t in TECHNOLOGY, e in EMISSION, m in MODE_OF_OPERATION, y in YEAR};
param EmissionsPenalty{r in REGION, e in EMISSION, y in YEAR};
param AnnualExogenousEmission{r in REGION, e in EMISSION, y in YEAR};
param AnnualEmissionLimit{r in REGION, e in EMISSION, y in YEAR};
param ModelPeriodExogenousEmission{r in REGION, e in EMISSION};
param ModelPeriodEmissionLimit{r in REGION, e in EMISSION};
#
######################
#   Model Variables  #
######################
#
########                        Demands                                         #############
#
#var RateOfDemand{r in REGION, l in TIMESLICE, f in FUEL, y in YEAR}>= 0;
#var Demand{r in REGION, l in TIMESLICE, f in FUEL, y in YEAR}>= 0;
#
########                     Storage                                 #############
#
var NewStorageCapacity{r in REGION, s in STORAGE, y in YEAR} >=0;
var SalvageValueStorage{r in REGION, s in STORAGE, y in YEAR} >=0;
var StorageLevelYearStart{r in REGION, s in STORAGE, y in YEAR} >=0;
var StorageLevelYearFinish{r in REGION, s in STORAGE, y in YEAR} >=0;
var StorageLevelSeasonStart{r in REGION, s in STORAGE, ls in SEASON, y in YEAR} >=0;
var StorageLevelDayTypeStart{r in REGION, s in STORAGE, ls in SEASON, ld in DAYTYPE, y in YEAR} >=0;
var StorageLevelDayTypeFinish{r in REGION, s in STORAGE, ls in SEASON, ld in DAYTYPE, y in YEAR} >=0;
#var RateOfStorageCharge{r in REGION, s in STORAGE, ls in SEASON, ld in DAYTYPE, lh in DAILYTIMEBRACKET, y in YEAR};
#var RateOfStorageDischarge{r in REGION, s in STORAGE, ls in SEASON, ld in DAYTYPE, lh in DAILYTIMEBRACKET, y in YEAR};
#var NetChargeWithinYear{r in REGION, s in STORAGE, ls in SEASON, ld in DAYTYPE, lh in DAILYTIMEBRACKET, y in YEAR};
#var NetChargeWithinDay{r in REGION, s in STORAGE, ls in SEASON, ld in DAYTYPE, lh in DAILYTIMEBRACKET, y in YEAR};
#var StorageLowerLimit{r in REGION, s in STORAGE, y in YEAR}>=0;
#var StorageUpperLimit{r in REGION, s in STORAGE, y in YEAR} >=0;
#var AccumulatedNewStorageCapacity{r in REGION, s in STORAGE, y in YEAR} >=0;
#var CapitalInvestmentStorage{r in REGION, s in STORAGE, y in YEAR} >=0;
#var DiscountedCapitalInvestmentStorage{r in REGION, s in STORAGE, y in YEAR} >=0;
#var DiscountedSalvageValueStorage{r in REGION, s in STORAGE, y in YEAR} >=0;
#var TotalDiscountedStorageCost{r in REGION, s in STORAGE, y in YEAR} >=0;
#
#########                    Capacity Variables                         #############
#
var NumberOfNewTechnologyUnits{r in REGION, t in TECHNOLOGY, y in YEAR} >= 0,integer;
var NewCapacity{r in REGION, t in TECHNOLOGY, y in YEAR} >= 0;
#var AccumulatedNewCapacity{r in REGION, t in TECHNOLOGY, y in YEAR} >= 0;
#var TotalCapacityAnnual{r in REGION, t in TECHNOLOGY, y in YEAR}>= 0;
#
#########                    Activity Variables                         #############
#
var RateOfActivity{r in REGION, l in TIMESLICE, t in TECHNOLOGY, m in MODE_OF_OPERATION, y in YEAR} >= 0;
var UseByTechnology{r in REGION, l in TIMESLICE, t in TECHNOLOGY, f in FUEL, y in YEAR}>= 0;
var Trade{r in REGION, rr in REGION, l in TIMESLICE, f in FUEL, y in YEAR};
var UseAnnual{r in REGION, f in FUEL, y in YEAR}>= 0;
#var RateOfTotalActivity{r in REGION, t in TECHNOLOGY, l in TIMESLICE, y in YEAR} >= 0;
#var TotalTechnologyAnnualActivity{r in REGION, t in TECHNOLOGY, y in YEAR} >= 0;
#var TotalAnnualTechnologyActivityByMode{r in REGION, t in TECHNOLOGY, m in MODE_OF_OPERATION, y in YEAR}>=0;
#var TotalTechnologyModelPeriodActivity{r in REGION, t in TECHNOLOGY};
#var RateOfProductionByTechnologyByMode{r in REGION, l in TIMESLICE, t in TECHNOLOGY, m in MODE_OF_OPERATION, f in FUEL, y in YEAR}>= 0;
#var RateOfProductionByTechnology{r in REGION, l in TIMESLICE, t in TECHNOLOGY, f in FUEL, y in YEAR}>= 0;
#var ProductionByTechnology{r in REGION, l in TIMESLICE, t in TECHNOLOGY, f in FUEL, y in YEAR}>= 0;
#var ProductionByTechnologyAnnual{r in REGION, t in TECHNOLOGY, f in FUEL, y in YEAR}>= 0;
#var RateOfProduction{r in REGION, l in TIMESLICE, f in FUEL, y in YEAR} >= 0;
#var Production{r in REGION, l in TIMESLICE, f in FUEL, y in YEAR} >= 0;
#var RateOfUseByTechnologyByMode{r in REGION, l in TIMESLICE, t in TECHNOLOGY, m in MODE_OF_OPERATION, f in FUEL, y in YEAR}>= 0;
#var RateOfUseByTechnology{r in REGION, l in TIMESLICE, t in TECHNOLOGY, f in FUEL, y in YEAR} >= 0;
#var UseByTechnologyAnnual{r in REGION, t in TECHNOLOGY, f in FUEL, y in YEAR}>= 0;
#var RateOfUse{r in REGION, l in TIMESLICE, f in FUEL, y in YEAR}>= 0;
#var Use{r in REGION, l in TIMESLICE, f in FUEL, y in YEAR}>= 0;
#var TradeAnnual{r in REGION, rr in REGION, f in FUEL, y in YEAR};
#var ProductionAnnual{r in REGION, f in FUEL, y in YEAR}>= 0;
#
#########                    Costing Variables                         #############
#
#var CapitalInvestment{r in REGION, t in TECHNOLOGY, y in YEAR}>= 0;
####var DiscountedCapitalInvestment{r in REGION, t in TECHNOLOGY, y in YEAR}>= 0;
#
var SalvageValue{r in REGION, t in TECHNOLOGY, y in YEAR}>= 0;
var DiscountedSalvageValue{r in REGION, t in TECHNOLOGY, y in YEAR}>= 0;
var OperatingCost{r in REGION, t in TECHNOLOGY, y in YEAR}>= 0;
#var DiscountedOperatingCost{r in REGION, t in TECHNOLOGY, y in YEAR}>= 0;
#var AnnualVariableOperatingCost{r in REGION, t in TECHNOLOGY, y in YEAR}>= 0;
#var AnnualFixedOperatingCost{r in REGION, t in TECHNOLOGY, y in YEAR}>= 0;
#var TotalDiscountedCostByTechnology{r in REGION, t in TECHNOLOGY, y in YEAR}>= 0;
#var TotalDiscountedCost{r in REGION, y in YEAR}>= 0;
#var ModelPeriodCostByRegion{r in REGION} >= 0;
#
#########                        Reserve Margin                                #############
#
#var TotalCapacityInReserveMargin{r in REGION, y in YEAR}>= 0;
#var DemandNeedingReserveMargin{r in REGION,l in TIMESLICE, y in YEAR}>= 0;
#
#########                        RE Gen Target                                #############
#
#var TotalREProductionAnnual{r in REGION, y in YEAR};
#var RETotalProductionOfTargetFuelAnnual{r in REGION, y in YEAR};
#
#########                        Emissions                                        #############
#
var DiscountedTechnologyEmissionsPenalty{r in REGION, t in TECHNOLOGY, y in YEAR}>=0;
var ModelPeriodEmissions{r in REGION, e in EMISSION}>=0;
#var AnnualTechnologyEmissionByMode{r in REGION, t in TECHNOLOGY, e in EMISSION, m in MODE_OF_OPERATION, y in YEAR}>= 0;
#var AnnualTechnologyEmission{r in REGION, t in TECHNOLOGY, e in EMISSION, y in YEAR}>= 0;
#var AnnualTechnologyEmissionPenaltyByEmission{r in REGION, t in TECHNOLOGY, e in EMISSION, y in YEAR}>= 0;
#var AnnualTechnologyEmissionsPenalty{r in REGION, t in TECHNOLOGY, y in YEAR}>= 0;
#var AnnualEmissions{r in REGION, e in EMISSION, y in YEAR}>= 0;
#
#        table data IN "CSV" "data.csv": s <- [FROM,TO], d~DISTANCE, c~COST;
#        table capacity IN "CSV" "SpecifiedAnnualDemand.csv": [YEAR, FUEL, REGION], SpecifiedAnnualDemand~ColumnNameInCSVSheet;
#
######################
# Objective Function #
######################
#
minimize cost: sum{r in REGION, t in TECHNOLOGY, y in YEAR}(((((sum{yy in YEAR: y-yy < OperationalLife[r,t] && y-yy>=0}NewCapacity[r,t,yy])+ResidualCapacity[r,t,y])*FixedCost[r,t,y]+sum{m in MODE_OF_OPERATION, l in TIMESLICE}RateOfActivity[r,l,t,m,y]*YearSplit[l,y]*VariableCost[r,t,m,y])/((1+DiscountRate[r])^(y-min{yy in YEAR} min(yy)+0.5))+CapitalCost[r,t,y] * NewCapacity[r,t,y]/((1+DiscountRate[r])^(y-min{yy in YEAR} min(yy)))+DiscountedTechnologyEmissionsPenalty[r,t,y]-DiscountedSalvageValue[r,t,y]))+sum{r in REGION, s in STORAGE, y in YEAR}(CapitalCostStorage[r,s,y] * NewStorageCapacity[r,s,y]/((1+DiscountRate[r])^(y-min{yy in YEAR} min(yy)))-SalvageValueStorage[r,s,y]/((1+DiscountRate[r])^(max{yy in YEAR} max(yy)-min{yy in YEAR} min(yy)+1)));
#
#####################
# Constraints       #
#####################
#
#s.t. EQ_SpecifiedDemand{r in REGION, l in TIMESLICE, f in FUEL, y in YEAR}: SpecifiedAnnualDemand[r,f,y]*SpecifiedDemandProfile[r,f,l,y] / YearSplit[l,y]=RateOfDemand[r,l,f,y];
#
#########               Capacity Adequacy A                     #############
#
#s.t. CAa1_TotalNewCapacity{r in REGION, t in TECHNOLOGY, y in YEAR}:AccumulatedNewCapacity[r,t,y] = sum{yy in YEAR: y-yy < OperationalLife[r,t] && y-yy>=0} NewCapacity[r,t,yy];
#s.t. CAa2_TotalAnnualCapacity{r in REGION, t in TECHNOLOGY, y in YEAR}: ((sum{yy in YEAR: y-yy < OperationalLife[r,t] && y-yy>=0} NewCapacity[r,t,yy])+ ResidualCapacity[r,t,y]) = TotalCapacityAnnual[r,t,y];
#s.t. CAa3_TotalActivityOfEachTechnology{r in REGION, t in TECHNOLOGY, l in TIMESLICE, y in YEAR}: sum{m in MODE_OF_OPERATION} RateOfActivity[r,l,t,m,y] = RateOfTotalActivity[r,t,l,y];
s.t. CAa4_Constraint_Capacity{r in REGION, l in TIMESLICE, t in TECHNOLOGY, y in YEAR}: sum{m in MODE_OF_OPERATION} RateOfActivity[r,l,t,m,y] <= ((sum{yy in YEAR: y-yy < OperationalLife[r,t] && y-yy>=0} NewCapacity[r,t,yy])+ ResidualCapacity[r,t,y])*CapacityFactor[r,t,l,y]*CapacityToActivityUnit[r,t];
s.t. CAa5_TotalNewCapacity{r in REGION, t in TECHNOLOGY, y in YEAR: CapacityOfOneTechnologyUnit[r,t,y]<>0}: CapacityOfOneTechnologyUnit[r,t,y]*NumberOfNewTechnologyUnits[r,t,y] = NewCapacity[r,t,y];
#
# Note that the PlannedMaintenance equation below ensures that all other technologies have a capacity great enough to at least meet the annual average.
#
#########               Capacity Adequacy B                         #############
#
s.t. CAb1_PlannedMaintenance{r in REGION, t in TECHNOLOGY, y in YEAR}: sum{l in TIMESLICE} sum{m in MODE_OF_OPERATION} RateOfActivity[r,l,t,m,y]*YearSplit[l,y] <= sum{l in TIMESLICE} (((sum{yy in YEAR: y-yy < OperationalLife[r,t] && y-yy>=0} NewCapacity[r,t,yy])+ ResidualCapacity[r,t,y])*CapacityFactor[r,t,l,y]*YearSplit[l,y])* AvailabilityFactor[r,t,y]*CapacityToActivityUnit[r,t];
#
#########                Energy Balance A                     #############
#
#s.t. EBa1_RateOfFuelProduction1{r in REGION, l in TIMESLICE, f in FUEL, t in TECHNOLOGY, m in MODE_OF_OPERATION, y in YEAR: OutputActivityRatio[r,t,f,m,y] <>0}:  RateOfActivity[r,l,t,m,y]*OutputActivityRatio[r,t,f,m,y]  = RateOfProductionByTechnologyByMode[r,l,t,m,f,y];
#s.t. EBa2_RateOfFuelProduction2{r in REGION, l in TIMESLICE, f in FUEL, t in TECHNOLOGY, y in YEAR}: sum{m in MODE_OF_OPERATION: OutputActivityRatio[r,t,f,m,y] <>0} RateOfActivity[r,l,t,m,y]*OutputActivityRatio[r,t,f,m,y] = RateOfProductionByTechnology[r,l,t,f,y] ;
#s.t. EBa3_RateOfFuelProduction3{r in REGION, l in TIMESLICE, f in FUEL, y in YEAR}: sum{m in MODE_OF_OPERATION, t in TECHNOLOGY: OutputActivityRatio[r,t,f,m,y] <>0} RateOfActivity[r,l,t,m,y]*OutputActivityRatio[r,t,f,m,y]  =  RateOfProduction[r,l,f,y];
#s.t. EBa4_RateOfFuelUse1{r in REGION, l in TIMESLICE, f in FUEL, t in TECHNOLOGY, m in MODE_OF_OPERATION, y in YEAR: InputActivityRatio[r,t,f,m,y]<>0}: RateOfActivity[r,l,t,m,y]*InputActivityRatio[r,t,f,m,y]  = RateOfUseByTechnologyByMode[r,l,t,m,f,y];
#s.t. EBa5_RateOfFuelUse2{r in REGION, l in TIMESLICE, f in FUEL, t in TECHNOLOGY, y in YEAR}: sum{m in MODE_OF_OPERATION: InputActivityRatio[r,t,f,m,y]<>0} RateOfActivity[r,l,t,m,y]*InputActivityRatio[r,t,f,m,y] = RateOfUseByTechnology[r,l,t,f,y];
#s.t. EBa6_RateOfFuelUse3{r in REGION, l in TIMESLICE, f in FUEL, y in YEAR}: sum{m in MODE_OF_OPERATION, t in TECHNOLOGY: InputActivityRatio[r,t,f,m,y]<>0} RateOfActivity[r,l,t,m,y]*InputActivityRatio[r,t,f,m,y]  = RateOfUse[r,l,f,y];
#s.t. EBa7_EnergyBalanceEachTS1{r in REGION, l in TIMESLICE, f in FUEL, y in YEAR}: sum{m in MODE_OF_OPERATION, t in TECHNOLOGY: OutputActivityRatio[r,t,f,m,y] <>0} RateOfActivity[r,l,t,m,y]*OutputActivityRatio[r,t,f,m,y]*YearSplit[l,y] = Production[r,l,f,y];
#s.t. EBa8_EnergyBalanceEachTS2{r in REGION, l in TIMESLICE, f in FUEL, y in YEAR}: sum{m in MODE_OF_OPERATION, t in TECHNOLOGY: InputActivityRatio[r,t,f,m,y]<>0} RateOfActivity[r,l,t,m,y]*InputActivityRatio[r,t,f,m,y]*YearSplit[l,y] = Use[r,l,f,y];
#s.t. EBa9_EnergyBalanceEachTS3{r in REGION, l in TIMESLICE, f in FUEL, y in YEAR}: SpecifiedAnnualDemand[r,f,y]*SpecifiedDemandProfile[r,f,l,y] = Demand[r,l,f,y];
s.t. EBa10_EnergyBalanceEachTS4{r in REGION, rr in REGION, l in TIMESLICE, f in FUEL, y in YEAR}: Trade[r,rr,l,f,y] = -Trade[rr,r,l,f,y];
s.t. EBa11_EnergyBalanceEachTS5{r in REGION, l in TIMESLICE, f in FUEL, y in YEAR}: sum{m in MODE_OF_OPERATION, t in TECHNOLOGY: OutputActivityRatio[r,t,f,m,y] <>0} RateOfActivity[r,l,t,m,y]*OutputActivityRatio[r,t,f,m,y]*YearSplit[l,y] >= SpecifiedAnnualDemand[r,f,y]*SpecifiedDemandProfile[r,f,l,y] + sum{m in MODE_OF_OPERATION, t in TECHNOLOGY: InputActivityRatio[r,t,f,m,y]<>0} RateOfActivity[r,l,t,m,y]*InputActivityRatio[r,t,f,m,y]*YearSplit[l,y] + sum{rr in REGION} Trade[r,rr,l,f,y]*TradeRoute[r,rr,f,y];
#
#########                Energy Balance B                         #############
#
#s.t. EBb1_EnergyBalanceEachYear1{r in REGION, f in FUEL, y in YEAR}: sum{m in MODE_OF_OPERATION, t in TECHNOLOGY, l in TIMESLICE: OutputActivityRatio[r,t,f,m,y] <>0} RateOfActivity[r,l,t,m,y]*OutputActivityRatio[r,t,f,m,y]*YearSplit[l,y] = ProductionAnnual[r,f,y];
#s.t. EBb2_EnergyBalanceEachYear2{r in REGION, f in FUEL, y in YEAR}: sum{m in MODE_OF_OPERATION, t in TECHNOLOGY, l in TIMESLICE: InputActivityRatio[r,t,f,m,y]<>0} RateOfActivity[r,l,t,m,y]*InputActivityRatio[r,t,f,m,y]*YearSplit[l,y] = UseAnnual[r,f,y];
#s.t. EBb3_EnergyBalanceEachYear3{r in REGION, rr in REGION, f in FUEL, y in YEAR}: sum{l in TIMESLICE} Trade[r,rr,l,f,y] = TradeAnnual[r,rr,f,y];
s.t. EBb4_EnergyBalanceEachYear4{r in REGION, f in FUEL, y in YEAR}: sum{m in MODE_OF_OPERATION, t in TECHNOLOGY, l in TIMESLICE: OutputActivityRatio[r,t,f,m,y] <>0} RateOfActivity[r,l,t,m,y]*OutputActivityRatio[r,t,f,m,y]*YearSplit[l,y] >= sum{m in MODE_OF_OPERATION, t in TECHNOLOGY, l in TIMESLICE: InputActivityRatio[r,t,f,m,y]<>0} RateOfActivity[r,l,t,m,y]*InputActivityRatio[r,t,f,m,y]*YearSplit[l,y] + sum{l in TIMESLICE, rr in REGION} Trade[r,rr,l,f,y]*TradeRoute[r,rr,f,y] + AccumulatedAnnualDemand[r,f,y];
#
#########                Accounting Technology Production/Use        #############
#
#s.t. Acc1_FuelProductionByTechnology{r in REGION, l in TIMESLICE, t in TECHNOLOGY, f in FUEL, y in YEAR}: sum{m in MODE_OF_OPERATION: OutputActivityRatio[r,t,f,m,y] <>0} RateOfActivity[r,l,t,m,y]*OutputActivityRatio[r,t,f,m,y] * YearSplit[l,y] = ProductionByTechnology[r,l,t,f,y];
#s.t. Acc2_FuelUseByTechnology{r in REGION, l in TIMESLICE, t in TECHNOLOGY, f in FUEL, y in YEAR}: sum{m in MODE_OF_OPERATION: InputActivityRatio[r,t,f,m,y]<>0} RateOfActivity[r,l,t,m,y]*InputActivityRatio[r,t,f,m,y] * YearSplit[l,y] = UseByTechnology[r,l,t,f,y];
#s.t. Acc3_AverageAnnualRateOfActivity{r in REGION, t in TECHNOLOGY, m in MODE_OF_OPERATION, y in YEAR}: sum{l in TIMESLICE} RateOfActivity[r,l,t,m,y]*YearSplit[l,y] = TotalAnnualTechnologyActivityByMode[r,t,m,y];
####s.t. Acc4_ModelPeriodCostByRegion{r in REGION}:sum{t in TECHNOLOGY, y in YEAR}(((((sum{yy in YEAR: y-yy < OperationalLife[r,t] && y-yy>=0} NewCapacity[r,t,yy])+ ResidualCapacity[r,t,y])*FixedCost[r,t,y] + sum{m in MODE_OF_OPERATION, l in TIMESLICE} RateOfActivity[r,l,t,m,y]*YearSplit[l,y]*VariableCost[r,t,m,y])/((1+DiscountRate[r])^(y-min{yy in YEAR} min(yy)+0.5))+CapitalCost[r,t,y] * NewCapacity[r,t,y]/((1+DiscountRate[r])^(y-min{yy in YEAR} min(yy)))+DiscountedTechnologyEmissionsPenalty[r,t,y]-DiscountedSalvageValue[r,t,y]) + sum{s in STORAGE} (CapitalCostStorage[r,s,y] * NewStorageCapacity[r,s,y]/((1+DiscountRate[r])^(y-min{yy in YEAR} min(yy)))-CapitalCostStorage[r,s,y] * NewStorageCapacity[r,s,y]/((1+DiscountRate[r])^(y-min{yy in YEAR} min(yy))))) = ModelPeriodCostByRegion[r];
#
#########                Storage Equations                        #############
#
#s.t. S1_RateOfStorageCharge{r in REGION, s in STORAGE, ls in SEASON, ld in DAYTYPE, lh in DAILYTIMEBRACKET, y in YEAR}: sum{t in TECHNOLOGY, m in MODE_OF_OPERATION, l in TIMESLICE:TechnologyToStorage[r,t,s,m]>0} RateOfActivity[r,l,t,m,y] * TechnologyToStorage[r,t,s,m] * Conversionls[l,ls] * Conversionld[l,ld] * Conversionlh[l,lh] = RateOfStorageCharge[r,s,ls,ld,lh,y];
#s.t. S2_RateOfStorageDischarge{r in REGION, s in STORAGE, ls in SEASON, ld in DAYTYPE, lh in DAILYTIMEBRACKET, y in YEAR}: sum{t in TECHNOLOGY, m in MODE_OF_OPERATION, l in TIMESLICE:TechnologyFromStorage[r,t,s,m]>0} RateOfActivity[r,l,t,m,y] * TechnologyFromStorage[r,t,s,m] * Conversionls[l,ls] * Conversionld[l,ld] * Conversionlh[l,lh] = RateOfStorageDischarge[r,s,ls,ld,lh,y];
#s.t. S3_NetChargeWithinYear{r in REGION, s in STORAGE, ls in SEASON, ld in DAYTYPE, lh in DAILYTIMEBRACKET, y in YEAR}: sum{l in TIMESLICE:Conversionls[l,ls]>0&&Conversionld[l,ld]>0&&Conversionlh[l,lh]>0}  (sum{t in TECHNOLOGY, m in MODE_OF_OPERATION:TechnologyToStorage[r,t,s,m]>0} (RateOfActivity[r,l,t,m,y] * TechnologyToStorage[r,t,s,m] * Conversionls[l,ls] * Conversionld[l,ld] * Conversionlh[l,lh]) - (sum{t in TECHNOLOGY, m in MODE_OF_OPERATION:TechnologyFromStorage[r,t,s,m]>0} RateOfActivity[r,l,t,m,y] * TechnologyFromStorage[r,t,s,m] * Conversionls[l,ls] * Conversionld[l,ld] * Conversionlh[l,lh])) * YearSplit[l,y] * Conversionls[l,ls] * Conversionld[l,ld] * Conversionlh[l,lh] = NetChargeWithinYear[r,s,ls,ld,lh,y];
#s.t. S4_NetChargeWithinDay{r in REGION, s in STORAGE, ls in SEASON, ld in DAYTYPE, lh in DAILYTIMEBRACKET, y in YEAR}: ((sum{t in TECHNOLOGY, m in MODE_OF_OPERATION, l in TIMESLICE:TechnologyToStorage[r,t,s,m]>0} RateOfActivity[r,l,t,m,y] * TechnologyToStorage[r,t,s,m] * Conversionls[l,ls] * Conversionld[l,ld] * Conversionlh[l,lh]) - (sum{t in TECHNOLOGY, m in MODE_OF_OPERATION, l in TIMESLICE:TechnologyFromStorage[r,t,s,m]>0} RateOfActivity[r,l,t,m,y] * TechnologyFromStorage[r,t,s,m] * Conversionls[l,ls] * Conversionld[l,ld] * Conversionlh[l,lh])) * DaySplit[lh,y] = NetChargeWithinDay[r,s,ls,ld,lh,y];
s.t. S5_and_S6_StorageLevelYearStart{r in REGION, s in STORAGE, y in YEAR}: if y = min{yy in YEAR} min(yy) then StorageLevelStart[r,s]
                                                                                                                                        else StorageLevelYearStart[r,s,y-1] + sum{ls in SEASON, ld in DAYTYPE, lh in DAILYTIMEBRACKET} sum{l in TIMESLICE:Conversionls[l,ls]>0&&Conversionld[l,ld]>0&&Conversionlh[l,lh]>0}  (sum{t in TECHNOLOGY, m in MODE_OF_OPERATION:TechnologyToStorage[r,t,s,m]>0} (RateOfActivity[r,l,t,m,y-1] * TechnologyToStorage[r,t,s,m] * Conversionls[l,ls] * Conversionld[l,ld] * Conversionlh[l,lh]) - (sum{t in TECHNOLOGY, m in MODE_OF_OPERATION:TechnologyFromStorage[r,t,s,m]>0} RateOfActivity[r,l,t,m,y-1] * TechnologyFromStorage[r,t,s,m] * Conversionls[l,ls] * Conversionld[l,ld] * Conversionlh[l,lh])) * YearSplit[l,y-1] * Conversionls[l,ls] * Conversionld[l,ld] * Conversionlh[l,lh]
                                                                                                                                        = StorageLevelYearStart[r,s,y];
s.t. S7_and_S8_StorageLevelYearFinish{r in REGION, s in STORAGE, y in YEAR}: if y < max{yy in YEAR} max(yy) then StorageLevelYearStart[r,s,y+1]
                                                                                                                                        else StorageLevelYearStart[r,s,y] + sum{ls in SEASON, ld in DAYTYPE, lh in DAILYTIMEBRACKET} sum{l in TIMESLICE:Conversionls[l,ls]>0&&Conversionld[l,ld]>0&&Conversionlh[l,lh]>0}  (sum{t in TECHNOLOGY, m in MODE_OF_OPERATION:TechnologyToStorage[r,t,s,m]>0} (RateOfActivity[r,l,t,m,y] * TechnologyToStorage[r,t,s,m] * Conversionls[l,ls] * Conversionld[l,ld] * Conversionlh[l,lh]) - (sum{t in TECHNOLOGY, m in MODE_OF_OPERATION:TechnologyFromStorage[r,t,s,m]>0} RateOfActivity[r,l,t,m,y] * TechnologyFromStorage[r,t,s,m] * Conversionls[l,ls] * Conversionld[l,ld] * Conversionlh[l,lh])) * YearSplit[l,y] * Conversionls[l,ls] * Conversionld[l,ld] * Conversionlh[l,lh]
                                                                                                                                        = StorageLevelYearFinish[r,s,y];
s.t. S9_and_S10_StorageLevelSeasonStart{r in REGION, s in STORAGE, ls in SEASON, y in YEAR}: if ls = min{lsls in SEASON} min(lsls) then StorageLevelYearStart[r,s,y]
                                                                                                                                        else StorageLevelSeasonStart[r,s,ls-1,y] + sum{ld in DAYTYPE, lh in DAILYTIMEBRACKET} sum{l in TIMESLICE:Conversionls[l,ls-1]>0&&Conversionld[l,ld]>0&&Conversionlh[l,lh]>0}  (sum{t in TECHNOLOGY, m in MODE_OF_OPERATION:TechnologyToStorage[r,t,s,m]>0} (RateOfActivity[r,l,t,m,y] * TechnologyToStorage[r,t,s,m] * Conversionls[l,ls-1] * Conversionld[l,ld] * Conversionlh[l,lh]) - (sum{t in TECHNOLOGY, m in MODE_OF_OPERATION:TechnologyFromStorage[r,t,s,m]>0} RateOfActivity[r,l,t,m,y] * TechnologyFromStorage[r,t,s,m] * Conversionls[l,ls-1] * Conversionld[l,ld] * Conversionlh[l,lh])) * YearSplit[l,y] * Conversionls[l,ls-1] * Conversionld[l,ld] * Conversionlh[l,lh]
                                                                                                                                        = StorageLevelSeasonStart[r,s,ls,y];
s.t. S11_and_S12_StorageLevelDayTypeStart{r in REGION, s in STORAGE, ls in SEASON, ld in DAYTYPE, y in YEAR}: if ld = min{ldld in DAYTYPE} min(ldld) then StorageLevelSeasonStart[r,s,ls,y]
                                                                                                                                        else StorageLevelDayTypeStart[r,s,ls,ld-1,y] + sum{lh in DAILYTIMEBRACKET} (((sum{t in TECHNOLOGY, m in MODE_OF_OPERATION, l in TIMESLICE:TechnologyToStorage[r,t,s,m]>0} RateOfActivity[r,l,t,m,y] * TechnologyToStorage[r,t,s,m] * Conversionls[l,ls] * Conversionld[l,ld-1] * Conversionlh[l,lh]) - (sum{t in TECHNOLOGY, m in MODE_OF_OPERATION, l in TIMESLICE:TechnologyFromStorage[r,t,s,m]>0} RateOfActivity[r,l,t,m,y] * TechnologyFromStorage[r,t,s,m] * Conversionls[l,ls] * Conversionld[l,ld-1] * Conversionlh[l,lh])) * DaySplit[lh,y]) * DaysInDayType[ls,ld-1,y]
                                                                                                                                        = StorageLevelDayTypeStart[r,s,ls,ld,y];
s.t. S13_and_S14_and_S15_StorageLevelDayTypeFinish{r in REGION, s in STORAGE, ls in SEASON, ld in DAYTYPE, y in YEAR}:        if ls = max{lsls in SEASON} max(lsls) && ld = max{ldld in DAYTYPE} max(ldld) then StorageLevelYearFinish[r,s,y]
                                                                                                                                        else if ld = max{ldld in DAYTYPE} max(ldld) then StorageLevelSeasonStart[r,s,ls+1,y]
                                                                                                                                        else StorageLevelDayTypeFinish[r,s,ls,ld+1,y] - sum{lh in DAILYTIMEBRACKET} (((sum{t in TECHNOLOGY, m in MODE_OF_OPERATION, l in TIMESLICE:TechnologyToStorage[r,t,s,m]>0} RateOfActivity[r,l,t,m,y] * TechnologyToStorage[r,t,s,m] * Conversionls[l,ls] * Conversionld[l,ld+1] * Conversionlh[l,lh]) - (sum{t in TECHNOLOGY, m in MODE_OF_OPERATION, l in TIMESLICE:TechnologyFromStorage[r,t,s,m]>0} RateOfActivity[r,l,t,m,y] * TechnologyFromStorage[r,t,s,m] * Conversionls[l,ls] * Conversionld[l,ld+1] * Conversionlh[l,lh])) * DaySplit[lh,y]) * DaysInDayType[ls,ld+1,y]
                                                                                                                                        = StorageLevelDayTypeFinish[r,s,ls,ld,y];
#
##########                Storage Constraints                                #############
#
s.t. SC1_LowerLimit_BeginningOfDailyTimeBracketOfFirstInstanceOfDayTypeInFirstWeekConstraint{r in REGION, s in STORAGE, ls in SEASON, ld in DAYTYPE, lh in DAILYTIMEBRACKET, y in YEAR}: 0 <= (StorageLevelDayTypeStart[r,s,ls,ld,y]+sum{lhlh in DAILYTIMEBRACKET:lh-lhlh>0} (((sum{t in TECHNOLOGY, m in MODE_OF_OPERATION, l in TIMESLICE:TechnologyToStorage[r,t,s,m]>0} RateOfActivity[r,l,t,m,y] * TechnologyToStorage[r,t,s,m] * Conversionls[l,ls] * Conversionld[l,ld] * Conversionlh[l,lhlh]) - (sum{t in TECHNOLOGY, m in MODE_OF_OPERATION, l in TIMESLICE:TechnologyFromStorage[r,t,s,m]>0} RateOfActivity[r,l,t,m,y] * TechnologyFromStorage[r,t,s,m] * Conversionls[l,ls] * Conversionld[l,ld] * Conversionlh[l,lhlh])) * DaySplit[lhlh,y]))-MinStorageCharge[r,s,y]*(sum{yy in YEAR: y-yy < OperationalLifeStorage[r,s] && y-yy>=0} NewStorageCapacity[r,s,yy]+ResidualStorageCapacity[r,s,y]);
s.t. SC1_UpperLimit_BeginningOfDailyTimeBracketOfFirstInstanceOfDayTypeInFirstWeekConstraint{r in REGION, s in STORAGE, ls in SEASON, ld in DAYTYPE, lh in DAILYTIMEBRACKET, y in YEAR}: (StorageLevelDayTypeStart[r,s,ls,ld,y]+sum{lhlh in DAILYTIMEBRACKET:lh-lhlh>0} (((sum{t in TECHNOLOGY, m in MODE_OF_OPERATION, l in TIMESLICE:TechnologyToStorage[r,t,s,m]>0} RateOfActivity[r,l,t,m,y] * TechnologyToStorage[r,t,s,m] * Conversionls[l,ls] * Conversionld[l,ld] * Conversionlh[l,lhlh]) - (sum{t in TECHNOLOGY, m in MODE_OF_OPERATION, l in TIMESLICE:TechnologyFromStorage[r,t,s,m]>0} RateOfActivity[r,l,t,m,y] * TechnologyFromStorage[r,t,s,m] * Conversionls[l,ls] * Conversionld[l,ld] * Conversionlh[l,lhlh])) * DaySplit[lhlh,y]))-(sum{yy in YEAR: y-yy < OperationalLifeStorage[r,s] && y-yy>=0} NewStorageCapacity[r,s,yy]+ResidualStorageCapacity[r,s,y]) <= 0;
s.t. SC2_LowerLimit_EndOfDailyTimeBracketOfLastInstanceOfDayTypeInFirstWeekConstraint{r in REGION, s in STORAGE, ls in SEASON, ld in DAYTYPE, lh in DAILYTIMEBRACKET, y in YEAR}: 0 <= if ld > min{ldld in DAYTYPE} min(ldld) then (StorageLevelDayTypeStart[r,s,ls,ld,y]-sum{lhlh in DAILYTIMEBRACKET:lh-lhlh<0} (((sum{t in TECHNOLOGY, m in MODE_OF_OPERATION, l in TIMESLICE:TechnologyToStorage[r,t,s,m]>0} RateOfActivity[r,l,t,m,y] * TechnologyToStorage[r,t,s,m] * Conversionls[l,ls] * Conversionld[l,ld-1] * Conversionlh[l,lhlh]) - (sum{t in TECHNOLOGY, m in MODE_OF_OPERATION, l in TIMESLICE:TechnologyFromStorage[r,t,s,m]>0} RateOfActivity[r,l,t,m,y] * TechnologyFromStorage[r,t,s,m] * Conversionls[l,ls] * Conversionld[l,ld-1] * Conversionlh[l,lhlh])) * DaySplit[lhlh,y]))-MinStorageCharge[r,s,y]*(sum{yy in YEAR: y-yy < OperationalLifeStorage[r,s] && y-yy>=0} NewStorageCapacity[r,s,yy]+ResidualStorageCapacity[r,s,y]);
s.t. SC2_UpperLimit_EndOfDailyTimeBracketOfLastInstanceOfDayTypeInFirstWeekConstraint{r in REGION, s in STORAGE, ls in SEASON, ld in DAYTYPE, lh in DAILYTIMEBRACKET, y in YEAR}: if ld > min{ldld in DAYTYPE} min(ldld) then (StorageLevelDayTypeStart[r,s,ls,ld,y]-sum{lhlh in DAILYTIMEBRACKET:lh-lhlh<0} (((sum{t in TECHNOLOGY, m in MODE_OF_OPERATION, l in TIMESLICE:TechnologyToStorage[r,t,s,m]>0} RateOfActivity[r,l,t,m,y] * TechnologyToStorage[r,t,s,m] * Conversionls[l,ls] * Conversionld[l,ld-1] * Conversionlh[l,lhlh]) - (sum{t in TECHNOLOGY, m in MODE_OF_OPERATION, l in TIMESLICE:TechnologyFromStorage[r,t,s,m]>0} RateOfActivity[r,l,t,m,y] * TechnologyFromStorage[r,t,s,m] * Conversionls[l,ls] * Conversionld[l,ld-1] * Conversionlh[l,lhlh])) * DaySplit[lhlh,y]))-(sum{yy in YEAR: y-yy < OperationalLifeStorage[r,s] && y-yy>=0} NewStorageCapacity[r,s,yy]+ResidualStorageCapacity[r,s,y]) <= 0;
s.t. SC3_LowerLimit_EndOfDailyTimeBracketOfLastInstanceOfDayTypeInLastWeekConstraint{r in REGION, s in STORAGE, ls in SEASON, ld in DAYTYPE, lh in DAILYTIMEBRACKET, y in YEAR}:  0 <= (StorageLevelDayTypeFinish[r,s,ls,ld,y] - sum{lhlh in DAILYTIMEBRACKET:lh-lhlh<0} (((sum{t in TECHNOLOGY, m in MODE_OF_OPERATION, l in TIMESLICE:TechnologyToStorage[r,t,s,m]>0} RateOfActivity[r,l,t,m,y] * TechnologyToStorage[r,t,s,m] * Conversionls[l,ls] * Conversionld[l,ld] * Conversionlh[l,lhlh]) - (sum{t in TECHNOLOGY, m in MODE_OF_OPERATION, l in TIMESLICE:TechnologyFromStorage[r,t,s,m]>0} RateOfActivity[r,l,t,m,y] * TechnologyFromStorage[r,t,s,m] * Conversionls[l,ls] * Conversionld[l,ld] * Conversionlh[l,lhlh])) * DaySplit[lhlh,y]))-MinStorageCharge[r,s,y]*(sum{yy in YEAR: y-yy < OperationalLifeStorage[r,s] && y-yy>=0} NewStorageCapacity[r,s,yy]+ResidualStorageCapacity[r,s,y]);
s.t. SC3_UpperLimit_EndOfDailyTimeBracketOfLastInstanceOfDayTypeInLastWeekConstraint{r in REGION, s in STORAGE, ls in SEASON, ld in DAYTYPE, lh in DAILYTIMEBRACKET, y in YEAR}:  (StorageLevelDayTypeFinish[r,s,ls,ld,y] - sum{lhlh in DAILYTIMEBRACKET:lh-lhlh<0} (((sum{t in TECHNOLOGY, m in MODE_OF_OPERATION, l in TIMESLICE:TechnologyToStorage[r,t,s,m]>0} RateOfActivity[r,l,t,m,y] * TechnologyToStorage[r,t,s,m] * Conversionls[l,ls] * Conversionld[l,ld] * Conversionlh[l,lhlh]) - (sum{t in TECHNOLOGY, m in MODE_OF_OPERATION, l in TIMESLICE:TechnologyFromStorage[r,t,s,m]>0} RateOfActivity[r,l,t,m,y] * TechnologyFromStorage[r,t,s,m] * Conversionls[l,ls] * Conversionld[l,ld] * Conversionlh[l,lhlh])) * DaySplit[lhlh,y]))-(sum{yy in YEAR: y-yy < OperationalLifeStorage[r,s] && y-yy>=0} NewStorageCapacity[r,s,yy]+ResidualStorageCapacity[r,s,y]) <= 0;
s.t. SC4_LowerLimit_BeginningOfDailyTimeBracketOfFirstInstanceOfDayTypeInLastWeekConstraint{r in REGION, s in STORAGE, ls in SEASON, ld in DAYTYPE, lh in DAILYTIMEBRACKET, y in YEAR}:         0 <= if ld > min{ldld in DAYTYPE} min(ldld) then (StorageLevelDayTypeFinish[r,s,ls,ld-1,y]+sum{lhlh in DAILYTIMEBRACKET:lh-lhlh>0} (((sum{t in TECHNOLOGY, m in MODE_OF_OPERATION, l in TIMESLICE:TechnologyToStorage[r,t,s,m]>0} RateOfActivity[r,l,t,m,y] * TechnologyToStorage[r,t,s,m] * Conversionls[l,ls] * Conversionld[l,ld] * Conversionlh[l,lhlh]) - (sum{t in TECHNOLOGY, m in MODE_OF_OPERATION, l in TIMESLICE:TechnologyFromStorage[r,t,s,m]>0} RateOfActivity[r,l,t,m,y] * TechnologyFromStorage[r,t,s,m] * Conversionls[l,ls] * Conversionld[l,ld] * Conversionlh[l,lhlh])) * DaySplit[lhlh,y]))-MinStorageCharge[r,s,y]*(sum{yy in YEAR: y-yy < OperationalLifeStorage[r,s] && y-yy>=0} NewStorageCapacity[r,s,yy]+ResidualStorageCapacity[r,s,y]);
s.t. SC4_UpperLimit_BeginningOfDailyTimeBracketOfFirstInstanceOfDayTypeInLastWeekConstraint{r in REGION, s in STORAGE, ls in SEASON, ld in DAYTYPE, lh in DAILYTIMEBRACKET, y in YEAR}: if ld > min{ldld in DAYTYPE} min(ldld) then (StorageLevelDayTypeFinish[r,s,ls,ld-1,y]+sum{lhlh in DAILYTIMEBRACKET:lh-lhlh>0} (((sum{t in TECHNOLOGY, m in MODE_OF_OPERATION, l in TIMESLICE:TechnologyToStorage[r,t,s,m]>0} RateOfActivity[r,l,t,m,y] * TechnologyToStorage[r,t,s,m] * Conversionls[l,ls] * Conversionld[l,ld] * Conversionlh[l,lhlh]) - (sum{t in TECHNOLOGY, m in MODE_OF_OPERATION, l in TIMESLICE:TechnologyFromStorage[r,t,s,m]>0} RateOfActivity[r,l,t,m,y] * TechnologyFromStorage[r,t,s,m] * Conversionls[l,ls] * Conversionld[l,ld] * Conversionlh[l,lhlh])) * DaySplit[lhlh,y]))-(sum{yy in YEAR: y-yy < OperationalLifeStorage[r,s] && y-yy>=0} NewStorageCapacity[r,s,yy]+ResidualStorageCapacity[r,s,y]) <= 0;
s.t. SC5_MaxChargeConstraint{r in REGION, s in STORAGE, ls in SEASON, ld in DAYTYPE, lh in DAILYTIMEBRACKET, y in YEAR}: sum{t in TECHNOLOGY, m in MODE_OF_OPERATION, l in TIMESLICE:TechnologyToStorage[r,t,s,m]>0} RateOfActivity[r,l,t,m,y] * TechnologyToStorage[r,t,s,m] * Conversionls[l,ls] * Conversionld[l,ld] * Conversionlh[l,lh] <= StorageMaxChargeRate[r,s];
s.t. SC6_MaxDischargeConstraint{r in REGION, s in STORAGE, ls in SEASON, ld in DAYTYPE, lh in DAILYTIMEBRACKET, y in YEAR}: sum{t in TECHNOLOGY, m in MODE_OF_OPERATION, l in TIMESLICE:TechnologyFromStorage[r,t,s,m]>0} RateOfActivity[r,l,t,m,y] * TechnologyFromStorage[r,t,s,m] * Conversionls[l,ls] * Conversionld[l,ld] * Conversionlh[l,lh] <= StorageMaxDischargeRate[r,s];
#
#########                Storage Investments                                #############
#
s.t. SI6_SalvageValueStorageAtEndOfPeriod1{r in REGION, s in STORAGE, y in YEAR: (y+OperationalLifeStorage[r,s]-1) <= (max{yy in YEAR} max(yy))}: 0 = SalvageValueStorage[r,s,y];
s.t. SI7_SalvageValueStorageAtEndOfPeriod2{r in REGION, s in STORAGE, y in YEAR: (DepreciationMethod[r]=1 && (y+OperationalLifeStorage[r,s]-1) > (max{yy in YEAR} max(yy)) && DiscountRate[r]=0) || (DepreciationMethod[r]=2 && (y+OperationalLifeStorage[r,s]-1) > (max{yy in YEAR} max(yy)))}: CapitalCostStorage[r,s,y] * NewStorageCapacity[r,s,y]*(1-(max{yy in YEAR} max(yy) - y+1)/OperationalLifeStorage[r,s]) = SalvageValueStorage[r,s,y];
s.t. SI8_SalvageValueStorageAtEndOfPeriod3{r in REGION, s in STORAGE, y in YEAR: DepreciationMethod[r]=1 && (y+OperationalLifeStorage[r,s]-1) > (max{yy in YEAR} max(yy)) && DiscountRate[r]>0}: CapitalCostStorage[r,s,y] * NewStorageCapacity[r,s,y]*(1-(((1+DiscountRate[r])^(max{yy in YEAR} max(yy) - y+1)-1)/((1+DiscountRate[r])^OperationalLifeStorage[r,s]-1))) = SalvageValueStorage[r,s,y];
#s.t. SI1_StorageUpperLimit{r in REGION, s in STORAGE, y in YEAR}: sum{yy in YEAR: y-yy < OperationalLifeStorage[r,s] && y-yy>=0} NewStorageCapacity[r,s,yy]+ResidualStorageCapacity[r,s,y] = StorageUpperLimit[r,s,y];
#s.t. SI2_StorageLowerLimit{r in REGION, s in STORAGE, y in YEAR}: MinStorageCharge[r,s,y]*(sum{yy in YEAR: y-yy < OperationalLifeStorage[r,s] && y-yy>=0} NewStorageCapacity[r,s,yy]+ResidualStorageCapacity[r,s,y]) = StorageLowerLimit[r,s,y];
#s.t. SI3_TotalNewStorage{r in REGION, s in STORAGE, y in YEAR}: sum{yy in YEAR: y-yy < OperationalLifeStorage[r,s] && y-yy>=0} NewStorageCapacity[r,s,yy]=AccumulatedNewStorageCapacity[r,s,y];
#s.t. SI4_UndiscountedCapitalInvestmentStorage{r in REGION, s in STORAGE, y in YEAR}: CapitalCostStorage[r,s,y] * NewStorageCapacity[r,s,y] = CapitalInvestmentStorage[r,s,y];
#s.t. SI5_DiscountingCapitalInvestmentStorage{r in REGION, s in STORAGE, y in YEAR}: CapitalCostStorage[r,s,y] * NewStorageCapacity[r,s,y]/((1+DiscountRate[r])^(y-min{yy in YEAR} min(yy))) = DiscountedCapitalInvestmentStorage[r,s,y];
#s.t. SI9_SalvageValueStorageDiscountedToStartYear{r in REGION, s in STORAGE, y in YEAR}: SalvageValueStorage[r,s,y]/((1+DiscountRate[r])^(max{yy in YEAR} max(yy)-min{yy in YEAR} min(yy)+1)) = DiscountedSalvageValueStorage[r,s,y];
#s.t. SI10_TotalDiscountedCostByStorage{r in REGION, s in STORAGE, y in YEAR}: (CapitalCostStorage[r,s,y] * NewStorageCapacity[r,s,y]/((1+DiscountRate[r])^(y-min{yy in YEAR} min(yy)))-CapitalCostStorage[r,s,y] * NewStorageCapacity[r,s,y]/((1+DiscountRate[r])^(y-min{yy in YEAR} min(yy)))) = TotalDiscountedStorageCost[r,s,y];
#
#########               Capital Costs                              #############
#
#s.t. CC1_UndiscountedCapitalInvestment{r in REGION, t in TECHNOLOGY, y in YEAR}: CapitalCost[r,t,y] * NewCapacity[r,t,y] = CapitalInvestment[r,t,y];
####s.t. CC2_DiscountingCapitalInvestment{r in REGION, t in TECHNOLOGY, y in YEAR}: CapitalCost[r,t,y] * NewCapacity[r,t,y]/((1+DiscountRate[r])^(y-min{yy in YEAR} min(yy))) = DiscountedCapitalInvestment[r,t,y];
#
#########           Salvage Value                    #############
#
s.t. SV1_SalvageValueAtEndOfPeriod1{r in REGION, t in TECHNOLOGY, y in YEAR: DepreciationMethod[r]=1 && (y + OperationalLife[r,t]-1) > (max{yy in YEAR} max(yy)) && DiscountRate[r]>0}: SalvageValue[r,t,y] = CapitalCost[r,t,y]*NewCapacity[r,t,y]*(1-(((1+DiscountRate[r])^(max{yy in YEAR} max(yy) - y+1)-1)/((1+DiscountRate[r])^OperationalLife[r,t]-1)));
s.t. SV2_SalvageValueAtEndOfPeriod2{r in REGION, t in TECHNOLOGY, y in YEAR: DepreciationMethod[r]=1 && (y + OperationalLife[r,t]-1) > (max{yy in YEAR} max(yy)) && DiscountRate[r]=0 || (DepreciationMethod[r]=2 && (y + OperationalLife[r,t]-1) > (max{yy in YEAR} max(yy)))}: SalvageValue[r,t,y] = CapitalCost[r,t,y]*NewCapacity[r,t,y]*(1-(max{yy in YEAR} max(yy) - y+1)/OperationalLife[r,t]);
s.t. SV3_SalvageValueAtEndOfPeriod3{r in REGION, t in TECHNOLOGY, y in YEAR: (y + OperationalLife[r,t]-1) <= (max{yy in YEAR} max(yy))}: SalvageValue[r,t,y] = 0;
s.t. SV4_SalvageValueDiscountedToStartYear{r in REGION, t in TECHNOLOGY, y in YEAR}: DiscountedSalvageValue[r,t,y] = SalvageValue[r,t,y]/((1+DiscountRate[r])^(1+max{yy in YEAR} max(yy)-min{yy in YEAR} min(yy)));
#
#########                Operating Costs                          #############
#
#s.t. OC1_OperatingCostsVariable{r in REGION, t in TECHNOLOGY, y in YEAR}: sum{m in MODE_OF_OPERATION, l in TIMESLICE} RateOfActivity[r,l,t,m,y]*YearSplit[l,y]*VariableCost[r,t,m,y] = AnnualVariableOperatingCost[r,t,y];
#s.t. OC2_OperatingCostsFixedAnnual{r in REGION, t in TECHNOLOGY, y in YEAR}: ((sum{yy in YEAR: y-yy < OperationalLife[r,t] && y-yy>=0} NewCapacity[r,t,yy])+ ResidualCapacity[r,t,y])*FixedCost[r,t,y] = AnnualFixedOperatingCost[r,t,y];
#s.t. OC3_OperatingCostsTotalAnnual{r in REGION, t in TECHNOLOGY, y in YEAR}: (((sum{yy in YEAR: y-yy < OperationalLife[r,t] && y-yy>=0} NewCapacity[r,t,yy])+ ResidualCapacity[r,t,y])*FixedCost[r,t,y] + sum{m in MODE_OF_OPERATION, l in TIMESLICE} RateOfActivity[r,l,t,m,y]*YearSplit[l,y]*VariableCost[r,t,m,y]) = OperatingCost[r,t,y];
####s.t. OC4_DiscountedOperatingCostsTotalAnnual{r in REGION, t in TECHNOLOGY, y in YEAR}: (((sum{yy in YEAR: y-yy < OperationalLife[r,t] && y-yy>=0} NewCapacity[r,t,yy])+ ResidualCapacity[r,t,y])*FixedCost[r,t,y] + sum{m in MODE_OF_OPERATION, l in TIMESLICE} RateOfActivity[r,l,t,m,y]*YearSplit[l,y]*VariableCost[r,t,m,y])/((1+DiscountRate[r])^(y-min{yy in YEAR} min(yy)+0.5)) = DiscountedOperatingCost[r,t,y];
#
#########               Total Discounted Costs                 #############
#
#s.t. TDC1_TotalDiscountedCostByTechnology{r in REGION, t in TECHNOLOGY, y in YEAR}: ((((sum{yy in YEAR: y-yy < OperationalLife[r,t] && y-yy>=0} NewCapacity[r,t,yy])+ ResidualCapacity[r,t,y])*FixedCost[r,t,y] + sum{m in MODE_OF_OPERATION, l in TIMESLICE} RateOfActivity[r,l,t,m,y]*YearSplit[l,y]*VariableCost[r,t,m,y])/((1+DiscountRate[r])^(y-min{yy in YEAR} min(yy)+0.5))+CapitalCost[r,t,y] * NewCapacity[r,t,y]/((1+DiscountRate[r])^(y-min{yy in YEAR} min(yy)))+DiscountedTechnologyEmissionsPenalty[r,t,y]-DiscountedSalvageValue[r,t,y]) = TotalDiscountedCostByTechnology[r,t,y];
####s.t. TDC2_TotalDiscountedCost{r in REGION, y in YEAR}: sum{t in TECHNOLOGY}((((sum{yy in YEAR: y-yy < OperationalLife[r,t] && y-yy>=0} NewCapacity[r,t,yy])+ ResidualCapacity[r,t,y])*FixedCost[r,t,y] + sum{m in MODE_OF_OPERATION, l in TIMESLICE} RateOfActivity[r,l,t,m,y]*YearSplit[l,y]*VariableCost[r,t,m,y])/((1+DiscountRate[r])^(y-min{yy in YEAR} min(yy)+0.5))+CapitalCost[r,t,y] * NewCapacity[r,t,y]/((1+DiscountRate[r])^(y-min{yy in YEAR} min(yy)))+DiscountedTechnologyEmissionsPenalty[r,t,y]-DiscountedSalvageValue[r,t,y]) + sum{s in STORAGE} (CapitalCostStorage[r,s,y] * NewStorageCapacity[r,s,y]/((1+DiscountRate[r])^(y-min{yy in YEAR} min(yy)))-CapitalCostStorage[r,s,y] * NewStorageCapacity[r,s,y]/((1+DiscountRate[r])^(y-min{yy in YEAR} min(yy)))) = TotalDiscountedCost[r,y];
#
#########                      Total Capacity Constraints         ##############
#
s.t. TCC1_TotalAnnualMaxCapacityConstraint{r in REGION, t in TECHNOLOGY, y in YEAR}: ((sum{yy in YEAR: y-yy < OperationalLife[r,t] && y-yy>=0} NewCapacity[r,t,yy])+ ResidualCapacity[r,t,y]) <= TotalAnnualMaxCapacity[r,t,y];
s.t. TCC2_TotalAnnualMinCapacityConstraint{r in REGION, t in TECHNOLOGY, y in YEAR: TotalAnnualMinCapacity[r,t,y]>0}: ((sum{yy in YEAR: y-yy < OperationalLife[r,t] && y-yy>=0} NewCapacity[r,t,yy])+ ResidualCapacity[r,t,y]) >= TotalAnnualMinCapacity[r,t,y];
#
#########                    New Capacity Constraints          ##############
#
s.t. NCC1_TotalAnnualMaxNewCapacityConstraint{r in REGION, t in TECHNOLOGY, y in YEAR}: NewCapacity[r,t,y] <= TotalAnnualMaxCapacityInvestment[r,t,y];
s.t. NCC2_TotalAnnualMinNewCapacityConstraint{r in REGION, t in TECHNOLOGY, y in YEAR: TotalAnnualMinCapacityInvestment[r,t,y]>0}: NewCapacity[r,t,y] >= TotalAnnualMinCapacityInvestment[r,t,y];
#
#########                   Annual Activity Constraints        ##############
#
s.t. AAC2_TotalAnnualTechnologyActivityUpperLimit{r in REGION, t in TECHNOLOGY, y in YEAR}: sum{l in TIMESLICE, m in MODE_OF_OPERATION} RateOfActivity[r,l,t,m,y]*YearSplit[l,y] <= TotalTechnologyAnnualActivityUpperLimit[r,t,y] ;
s.t. AAC3_TotalAnnualTechnologyActivityLowerLimit{r in REGION, t in TECHNOLOGY, y in YEAR: TotalTechnologyAnnualActivityLowerLimit[r,t,y]>0}: sum{l in TIMESLICE, m in MODE_OF_OPERATION} RateOfActivity[r,l,t,m,y]*YearSplit[l,y] >= TotalTechnologyAnnualActivityLowerLimit[r,t,y] ;
#s.t. AAC1_TotalAnnualTechnologyActivity{r in REGION, t in TECHNOLOGY, y in YEAR}: sum{l in TIMESLICE, m in MODE_OF_OPERATION} RateOfActivity[r,l,t,m,y]*YearSplit[l,y] = TotalTechnologyAnnualActivity[r,t,y];
#
#########                    Total Activity Constraints         ##############
#
s.t. TAC2_TotalModelHorizonTechnologyActivityUpperLimit{r in REGION, t in TECHNOLOGY: TotalTechnologyModelPeriodActivityUpperLimit[r,t]>0}: sum{l in TIMESLICE, m in MODE_OF_OPERATION, y in YEAR} RateOfActivity[r,l,t,m,y]*YearSplit[l,y] <= TotalTechnologyModelPeriodActivityUpperLimit[r,t] ;
s.t. TAC3_TotalModelHorizenTechnologyActivityLowerLimit{r in REGION, t in TECHNOLOGY: TotalTechnologyModelPeriodActivityLowerLimit[r,t]>0}: sum{l in TIMESLICE, m in MODE_OF_OPERATION, y in YEAR} RateOfActivity[r,l,t,m,y]*YearSplit[l,y] >= TotalTechnologyModelPeriodActivityLowerLimit[r,t] ;
#s.t. TAC1_TotalModelHorizonTechnologyActivity{r in REGION, t in TECHNOLOGY}: sum{l in TIMESLICE, m in MODE_OF_OPERATION, y in YEAR} RateOfActivity[r,l,t,m,y]*YearSplit[l,y] = TotalTechnologyModelPeriodActivity[r,t];
#
#########                   Reserve Margin Constraint        ############## NTS: Should change demand for production
#
s.t. RM3_ReserveMargin_Constraint{r in REGION, l in TIMESLICE, y in YEAR}: sum{m in MODE_OF_OPERATION, t in TECHNOLOGY, f in FUEL: OutputActivityRatio[r,t,f,m,y] <>0} RateOfActivity[r,l,t,m,y]*OutputActivityRatio[r,t,f,m,y] * ReserveMarginTagFuel[r,f,y] * ReserveMargin[r,y]<= sum {t in TECHNOLOGY} ((sum{yy in YEAR: y-yy < OperationalLife[r,t] && y-yy>=0} NewCapacity[r,t,yy])+ ResidualCapacity[r,t,y]) * ReserveMarginTagTechnology[r,t,y] * CapacityToActivityUnit[r,t];
#s.t. RM1_ReserveMargin_TechnologiesIncluded_In_Activity_Units{r in REGION, l in TIMESLICE, y in YEAR}: sum {t in TECHNOLOGY} ((sum{yy in YEAR: y-yy < OperationalLife[r,t] && y-yy>=0} NewCapacity[r,t,yy])+ ResidualCapacity[r,t,y]) * ReserveMarginTagTechnology[r,t,y] * CapacityToActivityUnit[r,t]         =         TotalCapacityInReserveMargin[r,y];
#s.t. RM2_ReserveMargin_FuelsIncluded{r in REGION, l in TIMESLICE, y in YEAR}:  sum{m in MODE_OF_OPERATION, t in TECHNOLOGY, f in FUEL: OutputActivityRatio[r,t,f,m,y] <>0} RateOfActivity[r,l,t,m,y]*OutputActivityRatio[r,t,f,m,y] * ReserveMarginTagFuel[r,f,y] = DemandNeedingReserveMargin[r,l,y];
#
#########                   RE Production Target                ############## NTS: Should change demand for production
#
s.t. RE4_EnergyConstraint{r in REGION, y in YEAR}:REMinProductionTarget[r,y]*sum{l in TIMESLICE, f in FUEL} sum{m in MODE_OF_OPERATION, t in TECHNOLOGY: OutputActivityRatio[r,t,f,m,y] <>0} RateOfActivity[r,l,t,m,y]*OutputActivityRatio[r,t,f,m,y]* YearSplit[l,y]*RETagFuel[r,f,y] <= sum{m in MODE_OF_OPERATION, l in TIMESLICE, t in TECHNOLOGY, f in FUEL: OutputActivityRatio[r,t,f,m,y] <>0} RateOfActivity[r,l,t,m,y]*OutputActivityRatio[r,t,f,m,y] * YearSplit[l,y]*RETagTechnology[r,t,y];
#s.t. RE1_FuelProductionByTechnologyAnnual{r in REGION, t in TECHNOLOGY, f in FUEL, y in YEAR}: sum{m in MODE_OF_OPERATION, l in TIMESLICE: OutputActivityRatio[r,t,f,m,y] <>0} RateOfActivity[r,l,t,m,y]*OutputActivityRatio[r,t,f,m,y] * YearSplit[l,y] = ProductionByTechnologyAnnual[r,t,f,y];
#s.t. RE2_TechIncluded{r in REGION, y in YEAR}: sum{m in MODE_OF_OPERATION, l in TIMESLICE, t in TECHNOLOGY, f in FUEL: OutputActivityRatio[r,t,f,m,y] <>0} RateOfActivity[r,l,t,m,y]*OutputActivityRatio[r,t,f,m,y] * YearSplit[l,y]*RETagTechnology[r,t,y] = TotalREProductionAnnual[r,y];
#s.t. RE3_FuelIncluded{r in REGION, y in YEAR}: sum{l in TIMESLICE, f in FUEL} sum{m in MODE_OF_OPERATION, t in TECHNOLOGY: OutputActivityRatio[r,t,f,m,y] <>0} RateOfActivity[r,l,t,m,y]*OutputActivityRatio[r,t,f,m,y]*RETagFuel[r,f,y] = RETotalProductionOfTargetFuelAnnual[r,y];
#s.t. RE5_FuelUseByTechnologyAnnual{r in REGION, t in TECHNOLOGY, f in FUEL, y in YEAR}: sum{m in MODE_OF_OPERATION, l in TIMESLICE: InputActivityRatio[r,t,f,m,y]<>0} RateOfActivity[r,l,t,m,y]*InputActivityRatio[r,t,f,m,y]*YearSplit[l,y] = UseByTechnologyAnnual[r,t,f,y];
#
#########                   Emissions Accounting                ##############
#
s.t. E5_DiscountedEmissionsPenaltyByTechnology{r in REGION, t in TECHNOLOGY, y in YEAR}: sum{e in EMISSION, l in TIMESLICE, m in MODE_OF_OPERATION} EmissionActivityRatio[r,t,e,m,y]*RateOfActivity[r,l,t,m,y]*YearSplit[l,y]*EmissionsPenalty[r,e,y]/((1+DiscountRate[r])^(y-min{yy in YEAR} min(yy)+0.5)) = DiscountedTechnologyEmissionsPenalty[r,t,y];
s.t. E8_AnnualEmissionsLimit{r in REGION, e in EMISSION, y in YEAR}: sum{l in TIMESLICE, t in TECHNOLOGY, m in MODE_OF_OPERATION} EmissionActivityRatio[r,t,e,m,y]*RateOfActivity[r,l,t,m,y]*YearSplit[l,y]+AnnualExogenousEmission[r,e,y] <= AnnualEmissionLimit[r,e,y];
s.t. E9_ModelPeriodEmissionsLimit{r in REGION, e in EMISSION}:  sum{l in TIMESLICE, t in TECHNOLOGY, m in MODE_OF_OPERATION, y in YEAR} EmissionActivityRatio[r,t,e,m,y]*RateOfActivity[r,l,t,m,y]*YearSplit[l,y] + ModelPeriodExogenousEmission[r,e] <= ModelPeriodEmissionLimit[r,e] ;
#s.t. E1_AnnualEmissionProductionByMode{r in REGION, t in TECHNOLOGY, e in EMISSION, m in MODE_OF_OPERATION, y in YEAR}: EmissionActivityRatio[r,t,e,m,y]*sum{l in TIMESLICE} RateOfActivity[r,l,t,m,y]*YearSplit[l,y]=AnnualTechnologyEmissionByMode[r,t,e,m,y];
#s.t. E2_AnnualEmissionProduction{r in REGION, t in TECHNOLOGY, e in EMISSION, m in MODE_OF_OPERATION, y in YEAR}: sum{l in TIMESLICE, m in MODE_OF_OPERATION: EmissionActivityRatio[r,t,e,m,y]<>0} EmissionActivityRatio[r,t,e,m,y]*RateOfActivity[r,l,t,m,y]*YearSplit[l,y] = AnnualTechnologyEmission[r,t,e,y];
#s.t. E3_EmissionsPenaltyByTechAndEmission{r in REGION, t in TECHNOLOGY, e in EMISSION, y in YEAR: EmissionActivityRatio[r,t,e,m,y]<>0}: sum{l in TIMESLICE, m in MODE_OF_OPERATION} EmissionActivityRatio[r,t,e,m,y]*RateOfActivity[r,l,t,m,y]*YearSplit[l,y]*EmissionsPenalty[r,e,y] = AnnualTechnologyEmissionPenaltyByEmission[r,t,e,y];
#s.t. E4_EmissionsPenaltyByTechnology{r in REGION, t in TECHNOLOGY, y in YEAR}: sum{e in EMISSION, l in TIMESLICE, m in MODE_OF_OPERATION} EmissionActivityRatio[r,t,e,m,y]*RateOfActivity[r,l,t,m,y]*YearSplit[l,y]*EmissionsPenalty[r,e,y] = AnnualTechnologyEmissionsPenalty[r,t,y];
#s.t. E6_EmissionsAccounting1{r in REGION, e in EMISSION, y in YEAR}: sum{l in TIMESLICE, t in TECHNOLOGY, m in MODE_OF_OPERATION: EmissionActivityRatio[r,t,e,m,y]<>0} EmissionActivityRatio[r,t,e,m,y]*RateOfActivity[r,l,t,m,y]*YearSplit[l,y] = AnnualEmissions[r,e,y];
#s.t. E7_EmissionsAccounting2{r in REGION, e in EMISSION}: sum{l in TIMESLICE, t in TECHNOLOGY, m in MODE_OF_OPERATION, y in YEAR: EmissionActivityRatio[r,t,e,m,y]<>0} EmissionActivityRatio[r,t,e,m,y]*RateOfActivity[r,l,t,m,y]*YearSplit[l,y] + ModelPeriodExogenousEmission[r,e] = ModelPeriodEmissions[r,e];
#
#
###########################################################################################
#
solve;
#
#########################################################################################################
#                                                                                                                                                                                                                #
#         Summary results tables below are printed to a comma-separated file called (csv_fname)                #
#        For a full set of results please see "Results.txt"                                                                                                        #
#        If you don't want these printed, please comment-out or delete them.                                                                        #
#                                                                                                                                                                                                                #
#########################################################################################################
#
#        table result{(f,t) in s} OUT "...": f~FROM, t~TO, x[f,t]~FLOW;
#        table result{y in YEAR, r in REGION} OUT "CSV" "Output.csv": y~YEARS, r~REGION, TotalDiscountedCost[y,r];
#
####        Summary results         ###
#
###                Total costs and emissions by region        ###
#

param csv_fname symbolic := "SelectedResults.csv";

printf "\n" > (csv_fname);
printf "Summary" >> (csv_fname);
for {r in REGION}  {
	printf ",%s", r >> (csv_fname);
}
printf "\n" >> (csv_fname);
printf "Emissions" >> (csv_fname);
printf "\n" >> (csv_fname);
for {r in REGION}  {
	for {e in EMISSION} {
		printf ",%s", e >> (csv_fname);
		printf ",%g", sum{l in TIMESLICE, t in TECHNOLOGY, m in MODE_OF_OPERATION, y in YEAR: EmissionActivityRatio[r,t,e,m,y]<>0} EmissionActivityRatio[r,t,e,m,y]*RateOfActivity[r,l,t,m,y]*YearSplit[l,y] + ModelPeriodExogenousEmission[r,e] >> (csv_fname);
		printf "\n" >> (csv_fname);
	}
}
printf "\n" >> (csv_fname);
printf "Cost" >> (csv_fname);
for {r in REGION} {
	printf ",%g", sum{t in TECHNOLOGY, y in YEAR}(((((sum{yy in YEAR: y-yy < OperationalLife[r,t] && y-yy>=0} NewCapacity[r,t,yy])+ ResidualCapacity[r,t,y])*FixedCost[r,t,y] + sum{m in MODE_OF_OPERATION, l in TIMESLICE} RateOfActivity[r,l,t,m,y]*YearSplit[l,y]*VariableCost[r,t,m,y])/((1+DiscountRate[r])^(y-min{yy in YEAR} min(yy)+0.5))+CapitalCost[r,t,y] * NewCapacity[r,t,y]/((1+DiscountRate[r])^(y-min{yy in YEAR} min(yy)))+DiscountedTechnologyEmissionsPenalty[r,t,y]-DiscountedSalvageValue[r,t,y]) + sum{s in STORAGE} (CapitalCostStorage[r,s,y] * NewStorageCapacity[r,s,y]/((1+DiscountRate[r])^(y-min{yy in YEAR} min(yy)))-CapitalCostStorage[r,s,y] * NewStorageCapacity[r,s,y]/((1+DiscountRate[r])^(y-min{yy in YEAR} min(yy))))) >> (csv_fname);
}
printf "\n" >> (csv_fname);
#
###  Time Independent demand ###
#
printf "\n" >> (csv_fname);
printf "TID Demand" >> (csv_fname);
printf "\n" >> (csv_fname);
printf "\n" >> (csv_fname);
for {r in REGION}{
	printf ",%s", r >> (csv_fname);
}
printf "\n" >> (csv_fname);
printf "Fuel" >> (csv_fname);
#printf "," >> (csv_fname);
for {y in YEAR}{
	printf ",%g", y >> (csv_fname);
}
printf "\n" >> (csv_fname);
for {r in REGION}{
	for {f in FUEL} {
		printf "%s,", f >> (csv_fname);
		#printf ",%s", f >> (csv_fname);
		for {y in YEAR}{
			#printf "\n" >> (csv_fname);
			#printf "%g", y >> (csv_fname);
			printf "%g,", AccumulatedAnnualDemand[r,f,y] >> (csv_fname);
			#printf "\n" >> (csv_fname);
		}
		printf "\n" >> (csv_fname);
	}
	#printf "\n" >> (csv_fname);
}
#
###  Time Dependent demand ###
#
printf "\n" >> (csv_fname);
printf "Time Dependent Demand (Energy Units)" >> (csv_fname);
printf "\n" >> (csv_fname);
printf "\n" >> (csv_fname);
for {r in REGION}{printf ",%s", r >> (csv_fname);}
printf "\n" >> (csv_fname);
printf "Fuel" >> (csv_fname);
printf ",Timeslice" >> (csv_fname);
for {y in YEAR}{printf ",%g", y >> (csv_fname);}
printf "\n" >> (csv_fname);
for {r in REGION }{
	for {f in FUEL} {
		#printf "%s", f >> (csv_fname);
		#printf "\n" >> (csv_fname);
		for {l in TIMESLICE}{
			printf "%s", f >> (csv_fname);
			printf ",%s", l >> (csv_fname);
			for {y in YEAR}{
				printf ",%g", SpecifiedAnnualDemand[r,f,y]*SpecifiedDemandProfile[r,f,l,y] >> (csv_fname);
			}
			printf "\n" >> (csv_fname);
		}
	}
}
#
###  Time Dependent production ###
#
printf "\n" >> (csv_fname);
printf "Time Dependent Production (Energy Units) Test" >> (csv_fname);
printf "\n" >> (csv_fname);
printf "\n" >> (csv_fname);
for {r in REGION}{printf ",%s", r >> (csv_fname);}
printf "\n" >> (csv_fname);
printf "Fuel" >> (csv_fname);
printf ",Timeslice" >> (csv_fname);
for {y in YEAR}{printf ",%g", y >> (csv_fname);}
printf "\n" >> (csv_fname);
for {r in REGION} {
	for {f in FUEL} {
		#printf "%s", f >> (csv_fname);
		#printf "\n" >> (csv_fname);
		for {l in TIMESLICE}{
			printf "%s", f >> (csv_fname);
			printf ",%s", l >> (csv_fname);
			for {y in YEAR }{
				printf ",%g", sum{m in MODE_OF_OPERATION, t in TECHNOLOGY: OutputActivityRatio[r,t,f,m,y] <>0} RateOfActivity[r,l,t,m,y]*OutputActivityRatio[r,t,f,m,y]*YearSplit[l,y] >> (csv_fname);
			}
			printf "\n" >> (csv_fname);
		}
	}
}
#
#### Total Annual Capacity ###
#
printf "\n" >> (csv_fname);
printf "TotalAnnualCapacity (Capacity Units)" >> (csv_fname);
printf "\n" >> (csv_fname);
printf "Technology" >> (csv_fname);
for {y in YEAR} {printf ",%s", y >> (csv_fname);}
printf "\n" >> (csv_fname);
for {r in REGION} {
	for { t in TECHNOLOGY } {
		printf "%s", t >> (csv_fname);
		for { y in YEAR } {
			printf ",%g", ((sum{yy in YEAR: y-yy < OperationalLife[r,t] && y-yy>=0} NewCapacity[r,t,yy])+ ResidualCapacity[r,t,y]) >> (csv_fname);
		}
		printf "\n" >> (csv_fname);
	}
}
#
#### New Annual Capacity ###
#
printf "\n" >> (csv_fname);
printf "NewCapacity (Capacity Units )" >> (csv_fname);
printf "\n" >> (csv_fname);
printf "Technology" >> (csv_fname);
for {y in YEAR}  {printf ",%s", y >> (csv_fname);}
printf "\n" >> (csv_fname);
for {r in REGION} {
	for { t in TECHNOLOGY }  {
		printf "%s", t >> (csv_fname);
		for { y in YEAR }  {
			printf ",%g", NewCapacity[r,t,y] >> (csv_fname);
		}
		printf "\n" >> (csv_fname);
	}
}
#
### Annual Production ###
#
printf "\n" >> (csv_fname);
printf "\n" >> (csv_fname);
printf "Annual Production (Energy Units)" >> (csv_fname);
printf "\n" >> (csv_fname);
for {r in REGION}{printf ",%s", r >> (csv_fname);}
printf "\n" >> (csv_fname);
printf "Technology" >> (csv_fname);
printf ",Fuel" >> (csv_fname);
for {y in YEAR}{printf",%g",y >> (csv_fname);}
printf "\n" >> (csv_fname);
for{r in REGION}{
	for {t in TECHNOLOGY}{
		#printf "%s", t >> (csv_fname);
		#printf "\n" >> (csv_fname);
		for {f in FUEL }{
			printf "%s", t >> (csv_fname);
			printf ",%s", f >> (csv_fname);
				for {y in YEAR}{printf ",%g", sum{m in MODE_OF_OPERATION, l in TIMESLICE: OutputActivityRatio[r,t,f,m,y] <>0} RateOfActivity[r,l,t,m,y]*OutputActivityRatio[r,t,f,m,y] * YearSplit[l,y] >> (csv_fname);
			}
			printf "\n" >> (csv_fname);
		}
	}
	printf "\n" >> (csv_fname);
}
#
### Annual Use ###
#
#printf "\n" >> (csv_fname);
printf "\n" >> (csv_fname);
printf "Annual Use (Energy Units)" >> (csv_fname);
printf "\n" >> (csv_fname);
for {r in REGION} {
	printf ",%s", r >> (csv_fname);
	printf "\n" >> (csv_fname);
	printf "Technology" >> (csv_fname);
	printf ",Fuel" >> (csv_fname);
	#printf ",Timeslice" >> (csv_fname);
	for {y in YEAR}{printf",%g",y >> (csv_fname);}
	printf "\n" >> (csv_fname);
	for {t in TECHNOLOGY}{
		#printf "%s", t >> (csv_fname);
		for {f in FUEL }{
			printf "%s", t >> (csv_fname);
			printf ",%s", f >> (csv_fname);
			for {y in YEAR}{
				printf ",%g", sum{m in MODE_OF_OPERATION, l in TIMESLICE: InputActivityRatio[r,t,f,m,y]<>0} RateOfActivity[r,l,t,m,y]*InputActivityRatio[r,t,f,m,y]*YearSplit[l,y] >> (csv_fname);
			}
			printf "\n" >> (csv_fname);
		}
		#printf "\n" >> (csv_fname);
	}
}
#
###  Technology Production in each TS ###
#
printf "\n" >> (csv_fname);
printf "ProductionByTechnology (Energy Units)" >> (csv_fname);
printf "\n" >> (csv_fname);
printf "\n" >> (csv_fname);
for {r in REGION} {
	printf ",%s", r >> (csv_fname);
	printf "\n" >> (csv_fname);
	printf "Technology" >> (csv_fname);
	printf ",Fuel" >> (csv_fname);
	printf ",Timeslice" >> (csv_fname);
	for {y in YEAR}{printf ",%g", y >> (csv_fname);}
	printf "\n" >> (csv_fname);
	for {t in TECHNOLOGY} {
		#printf "%s", t >> (csv_fname);
		#printf "\n" >> (csv_fname);
		for {f in FUEL } {
			#printf ",%s", f >> (csv_fname);
			for {l in TIMESLICE}{
				#printf ",%s", l >> (csv_fname);
				printf "%s", t >> (csv_fname);
				printf ",%s", f >> (csv_fname);
				printf ",%s", l >> (csv_fname);
				for { y in YEAR} {
					printf ",%g", sum{m in MODE_OF_OPERATION: OutputActivityRatio[r,t,f,m,y] <>0} RateOfActivity[r,l,t,m,y]*OutputActivityRatio[r,t,f,m,y] * YearSplit[l,y] >> (csv_fname);
				}
				printf "\n" >> (csv_fname);
				#printf "," >> (csv_fname);
			}
			#printf "\n" >> (csv_fname);
		}
		#printf "\n" >> (csv_fname);
	}
}
#
###  Technology Use in each TS ###
#
printf "\n" >> (csv_fname);
printf "Use By Technology (Energy Units)" >> (csv_fname);
printf "\n" >> (csv_fname);
printf "\n" >> (csv_fname);
for {r in REGION} {
	printf ",%s", r >> (csv_fname);
	printf "\n" >> (csv_fname);
	printf "Technology" >> (csv_fname);
	printf ",Fuel" >> (csv_fname);
	printf ",Timeslice" >> (csv_fname);
	for {y in YEAR}{printf ",%g", y >> (csv_fname);}
	printf "\n" >> (csv_fname);
	for {t in TECHNOLOGY} {
		#printf "%s", t >> (csv_fname);
		#printf "," >> (csv_fname);
		#for {f in FUEL}{printf",%s",f >> (csv_fname);
		#for {y in YEAR}{
		#printf ",%g", y >> (csv_fname);
		#}
		#}
		#printf "\n" >> (csv_fname);
		for {f in FUEL} {
			#printf "%s", f >> (csv_fname);
			for {l in TIMESLICE}{
				#printf ",%s", l >> (csv_fname);
				printf "%s", t >> (csv_fname);
				printf ",%s", f >> (csv_fname);
				printf ",%s", l >> (csv_fname);
				for { y in YEAR} {
					printf ",%g", sum{m in MODE_OF_OPERATION: InputActivityRatio[r,t,f,m,y]<>0} RateOfActivity[r,l,t,m,y]*InputActivityRatio[r,t,f,m,y] * YearSplit[l,y] >> (csv_fname);
				}
				printf "\n" >> (csv_fname);
			}
			#printf "\n" >> (csv_fname);
		}
		#printf "\n" >> (csv_fname);
	}
}
#
###  Total Annual Emissions ###
#
printf "\n" >> (csv_fname);
printf "Annual Emissions (Emissions Units)" >> (csv_fname);
printf "\n" >> (csv_fname);
for {r in REGION}{
	printf ",%s", r >> (csv_fname);
	printf "\n" >> (csv_fname);
	printf "," >> (csv_fname);
	for {y in YEAR} {printf ",%s", y >> (csv_fname);}
	printf "\n" >> (csv_fname);
	for {e in EMISSION}{
		printf ",%s", e >> (csv_fname);
		#printf "\n" >> (csv_fname);
		#printf "\n" >> (csv_fname);
		for {y in YEAR }{
			#printf "%g", y >> (csv_fname);
			printf ",%g", sum{l in TIMESLICE, t in TECHNOLOGY, m in MODE_OF_OPERATION: EmissionActivityRatio[r,t,e,m,y]<>0} EmissionActivityRatio[r,t,e,m,y]*RateOfActivity[r,l,t,m,y]*YearSplit[l,y] >> (csv_fname);
			#printf "\n" >> (csv_fname);
		}
		printf "\n" >> (csv_fname);
	}
}
#
### Annual Emissions by Technology ###
#
printf "\n" >> (csv_fname);
printf "Annual Emissions by Technology (Emissions Units)" >> (csv_fname);
printf "\n" >> (csv_fname);
for {r in REGION} {
	printf ",%s", r >> (csv_fname);
	printf "\n" >> (csv_fname);
	printf "Technology" >> (csv_fname);
	printf ",Emission" >> (csv_fname);
	for {y in YEAR} {printf ",%s", y >> (csv_fname);}
	printf "\n" >> (csv_fname);
	for {t in TECHNOLOGY}  {
		#printf "%s", t >> (csv_fname);
		for {e in EMISSION}{
			printf "%s", t >> (csv_fname);
			printf",%s",e >> (csv_fname);
			#}
			#printf "\n" >> (csv_fname);
			#for {e in EMISSION}  {
			#printf "%g", y >> (csv_fname);
			for {y in YEAR}{
				printf ",%g", sum{l in TIMESLICE, m in MODE_OF_OPERATION: EmissionActivityRatio[r,t,e,m,y]<>0} EmissionActivityRatio[r,t,e,m,y]*RateOfActivity[r,l,t,m,y]*YearSplit[l,y] >> (csv_fname);
			}
			printf "\n" >> (csv_fname);
		}
		#printf "\n" >> (csv_fname);
	}
}
end;
