/* Cycle_Network_Model.cpp */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <vector>
#include <boost/mpi.hpp>
#include "repast_hpc/AgentId.h"
#include "repast_hpc/RepastProcess.h"
#include "repast_hpc/Utilities.h"
#include "repast_hpc/Properties.h"
#include "repast_hpc/initialize_random.h"
#include "repast_hpc/SVDataSetBuilder.h"

#include "Cycle_Network_Model.h"


BOOST_CLASS_EXPORT_GUID(repast::SpecializedProjectionInfoPacket<DemoModelCustomEdgeContent<RepastHPCDemoAgent> >, "SpecializedProjectionInfoPacket_CUSTOM_EDGE");

RepastHPCDemoAgentPackageProvider::RepastHPCDemoAgentPackageProvider(repast::SharedContext<RepastHPCDemoAgent>* agentPtr): agents(agentPtr){ }

void RepastHPCDemoAgentPackageProvider::providePackage(RepastHPCDemoAgent * agent, std::vector<RepastHPCDemoAgentPackage>& out){
    repast::AgentId id = agent->getId();
    RepastHPCDemoAgentPackage package(id.id(), id.startingRank(), id.agentType(), id.currentRank(), agent->getC(), agent->getTotal());
    out.push_back(package);
}

void RepastHPCDemoAgentPackageProvider::provideContent(repast::AgentRequest req, std::vector<RepastHPCDemoAgentPackage>& out){
    std::vector<repast::AgentId> ids = req.requestedAgents();
    for(size_t i = 0; i < ids.size(); i++){
        providePackage(agents->getAgent(ids[i]), out);
    }
}


RepastHPCDemoAgentPackageReceiver::RepastHPCDemoAgentPackageReceiver(repast::SharedContext<RepastHPCDemoAgent>* agentPtr): agents(agentPtr){}

RepastHPCDemoAgent * RepastHPCDemoAgentPackageReceiver::createAgent(RepastHPCDemoAgentPackage package){
    repast::AgentId id(package.id, package.rank, package.type, package.currentRank);
    return new RepastHPCDemoAgent(id, package.c, package.total);
}

void RepastHPCDemoAgentPackageReceiver::updateAgent(RepastHPCDemoAgentPackage package){
    repast::AgentId id(package.id, package.rank, package.type);
    RepastHPCDemoAgent * agent = agents->getAgent(id);
    agent->set(package.currentRank, package.c, package.total);
}



DataSource_AgentTotals::DataSource_AgentTotals(repast::SharedContext<RepastHPCDemoAgent>* c) : context(c){ }

int DataSource_AgentTotals::getData(){
	int sum = 0;
	repast::SharedContext<RepastHPCDemoAgent>::const_local_iterator iter    = context->localBegin();
	repast::SharedContext<RepastHPCDemoAgent>::const_local_iterator iterEnd = context->localEnd();
	while( iter != iterEnd) {
		sum+= (*iter)->getTotal();
		iter++;
	}
	return sum;
}

DataSource_AgentCTotals::DataSource_AgentCTotals(repast::SharedContext<RepastHPCDemoAgent>* c) : context(c){ }

int DataSource_AgentCTotals::getData(){
	int sum = 0;
	repast::SharedContext<RepastHPCDemoAgent>::const_local_iterator iter    = context->localBegin();
	repast::SharedContext<RepastHPCDemoAgent>::const_local_iterator iterEnd = context->localEnd();
	while( iter != iterEnd) {
		sum+= (*iter)->getC();
		iter++;
	}
	return sum;
}



RepastHPCDemoModel::RepastHPCDemoModel(std::string propsFile, int argc, char** argv, boost::mpi::communicator* comm): context(comm){
	props = new repast::Properties(propsFile, argc, argv, comm);
	stopAt = repast::strToInt(props->getProperty("stop.at"));
	countOfAgents = repast::strToInt(props->getProperty("count.of.agents"));
	countOfRegions = repast::strToInt(props->getProperty("count.of.regions"));
	initializeRandom(*props, comm);
	if(repast::RepastProcess::instance()->rank() == 0) props->writeToSVFile("./output/record.csv");
	provider = new RepastHPCDemoAgentPackageProvider(&context);
	receiver = new RepastHPCDemoAgentPackageReceiver(&context);
	
  agentNetwork = new repast::SharedNetwork<RepastHPCDemoAgent, DemoModelCustomEdge<RepastHPCDemoAgent>, DemoModelCustomEdgeContent<RepastHPCDemoAgent>, DemoModelCustomEdgeContentManager<RepastHPCDemoAgent> >("agentNetwork", false, &edgeContentManager);
	context.addProjection(agentNetwork);
	
	// Data collection
	// Create the data set builder
	std::string fileOutputName("./output/agent_total_data.csv");
	repast::SVDataSetBuilder builder(fileOutputName.c_str(), ",", repast::RepastProcess::instance()->getScheduleRunner().schedule());
	
	// Create the individual data sets to be added to the builder
	DataSource_AgentTotals* agentTotals_DataSource = new DataSource_AgentTotals(&context);
	builder.addDataSource(createSVDataSource("Total", agentTotals_DataSource, std::plus<int>()));

	DataSource_AgentCTotals* agentCTotals_DataSource = new DataSource_AgentCTotals(&context);
	builder.addDataSource(createSVDataSource("C", agentCTotals_DataSource, std::plus<int>()));

	// Use the builder to create the data set
	agentValues = builder.createDataSet();
	
}

RepastHPCDemoModel::~RepastHPCDemoModel(){
	delete props;
	delete provider;
	delete receiver;
	delete agentValues;

}

void RepastHPCDemoModel::init(std::string propsFile){
	int rank = repast::RepastProcess::instance()->rank();
	for(int i = 0; i < countOfAgents; i++){
		repast::AgentId id(i,rank,0);
		id.currentRank(rank);
		RepastHPCDemoAgent* agent = new RepastHPCDemoAgent(id,floor((double(i)/countOfAgents)*countOfRegions));
		context.addAgent(agent);
	}
	std::ofstream output;
	output.open("recordedData.csv");
	for(int i = 0; i < countOfRegions; i++){
		string rID = (props->getProperty("ID" + std::to_string(i)));
		int rDedicatedRoadSpace = repast::strToInt(props->getProperty("DRS" + std::to_string(i)));
		int rDriverMultiplier = repast::strToInt(props->getProperty("DM" + std::to_string(i)));
		int rHilliness = repast::strToInt(props->getProperty("H" + std::to_string(i)));
		bool rElectricBikes = repast::strToInt(props->getProperty("EB" + std::to_string(i)));
		Region *Regionn = new Region(rID, rDedicatedRoadSpace, rDriverMultiplier, rHilliness, rElectricBikes);
		Regions.push_back(*Regionn);
		output << rID << ",";
	}

	output << std::endl;
	output.close();
}


void RepastHPCDemoModel::connectAgentNetwork(){

	for(int i = 0; i < countOfAgents-1; i++){
		repast::AgentId agentFrom(i, 0, 0);
		RepastHPCDemoAgent* fromAgent = context.getAgent(agentFrom);
		repast::AgentId agentTo(i+1, 0, 0);
		RepastHPCDemoAgent* toAgent = context.getAgent(agentTo);
		boost::shared_ptr<DemoModelCustomEdge<RepastHPCDemoAgent> > demoEdge(new DemoModelCustomEdge<RepastHPCDemoAgent>(fromAgent, toAgent, rand() % 10, rand() % 10));
		agentNetwork->addEdge(demoEdge);
		std::cout << "CONNECTING: " << fromAgent->getId() << " to " << toAgent->getId() << std::endl;
	}
	repast::AgentId agentFrom(countOfAgents-1, 0, 0);
	RepastHPCDemoAgent* fromAgent = context.getAgent(agentFrom);
	repast::AgentId agentTo(0, 0, 0);
	RepastHPCDemoAgent* toAgent = context.getAgent(agentTo);
	boost::shared_ptr<DemoModelCustomEdge<RepastHPCDemoAgent> > demoEdge(new DemoModelCustomEdge<RepastHPCDemoAgent>(fromAgent, toAgent, rand() % 10, rand() % 10));
	agentNetwork->addEdge(demoEdge);
	std::cout << "CONNECTING: " << fromAgent->getId() << " to " << toAgent->getId() << std::endl;
	
	for(int i = 0; i < countOfAgents; i++){
		repast::AgentId agentFrom(rand()%countOfAgents, 0, 0);
		RepastHPCDemoAgent* fromAgent = context.getAgent(agentFrom);
		repast::AgentId agentTo(rand()%countOfAgents, 0, 0);
		RepastHPCDemoAgent* toAgent = context.getAgent(agentTo);
		boost::shared_ptr<DemoModelCustomEdge<RepastHPCDemoAgent> > demoEdge(new DemoModelCustomEdge<RepastHPCDemoAgent>(fromAgent, toAgent, rand() % 10, rand() % 10));
		agentNetwork->addEdge(demoEdge);
		std::cout << "CONNECTING: " << fromAgent->getId() << " to " << toAgent->getId() << std::endl;
	}
}



void RepastHPCDemoModel::doSomething(){
	std::cout << " TICK " << repast::RepastProcess::instance()->getScheduleRunner().currentTick() << std::endl;
	for(int i = 0; i< countOfRegions; i++){
		Regions[i].resetCyclists();
		Regions[i].resetDrivers();
	}
	std::cout << "AGENTS:" << std::endl;
	for(int i = 0; i < countOfAgents; i++){
		repast::AgentId toDisplay(i, 0, 0);
		RepastHPCDemoAgent* agent = context.getAgent(toDisplay);
		//if(agent != 0) std::cout << agent->getId() << " " << agent->cycle() << " " << agent->popular()  << std::endl;
		if(agent != 0) {
			//std::cout << agent->cycle() << " ";
			if (agent->cycle() == 1) Regions[agent->getr()].incCyclists();
			else Regions[agent->getr()].incDrivers();
		}
	}
        std::cout<< endl;
	for(int i = 0; i< countOfRegions; i++){
		Regions[i].calcPervcievedRoadSafety();
	}
	
	std::vector<RepastHPCDemoAgent*> agents;
	context.selectAgents(countOfAgents, agents);
	std::vector<RepastHPCDemoAgent*>::iterator it = agents.begin();
	while(it != agents.end()){
		(*it)->play(agentNetwork);
		it++;
    }
	it = agents.begin();
	while(it != agents.end()){
		//std::cout<<Regions[1].getEB()<<endl;
		(*it)->updateCycle((Regions[(*it)->getr()].getEB()),(Regions[(*it)->getr()].getPRS()),(Regions[(*it)->getr()].getHilliness()));
		it++;
    }
	
}

void RepastHPCDemoModel::countRegions(){
	std::cout << " TICK " << repast::RepastProcess::instance()->getScheduleRunner().currentTick() << std::endl;
	std::ofstream output;
	output.open("recordedData.csv", ofstream::app);
	for(int i = 0; i< countOfRegions; i++){
		std::cout<< Regions[i].getID()<< ": " <<Regions[i].getPercOfCyclistsPerDay()*100 << "%" << std::endl;

		output << Regions[i].getPercOfCyclistsPerDay() << ",";
	}
	 output << std::endl;
	
}

void RepastHPCDemoModel::buildInfrastructure(){
	//Regions[4].buildEB();
	for (int i = 0; i < 50; i++){ 	
		Regions[4].increaseDRS();
	}
	Regions[2].buildEB();
	for (int i = 0; i < 50; i++){ 	
		Regions[2].increaseDRS();
	}
}

void RepastHPCDemoModel::initSchedule(repast::ScheduleRunner& runner){
	runner.scheduleEvent(1, repast::Schedule::FunctorPtr(new repast::MethodFunctor<RepastHPCDemoModel> (this, &RepastHPCDemoModel::connectAgentNetwork)));
	runner.scheduleEvent(2,2, repast::Schedule::FunctorPtr(new repast::MethodFunctor<RepastHPCDemoModel> (this, &RepastHPCDemoModel::doSomething)));
	runner.scheduleEvent(3,2, repast::Schedule::FunctorPtr(new repast::MethodFunctor<RepastHPCDemoModel> (this, &RepastHPCDemoModel::countRegions)));
	runner.scheduleEvent(120, repast::Schedule::FunctorPtr(new repast::MethodFunctor<RepastHPCDemoModel> (this, &RepastHPCDemoModel::buildInfrastructure)));
	runner.scheduleEndEvent(repast::Schedule::FunctorPtr(new repast::MethodFunctor<RepastHPCDemoModel> (this, &RepastHPCDemoModel::recordResults)));
	runner.scheduleStop(stopAt);
	
	// Data collection
	runner.scheduleEvent(1.5, 5, repast::Schedule::FunctorPtr(new repast::MethodFunctor<repast::DataSet>(agentValues, &repast::DataSet::record)));
	runner.scheduleEvent(10.6, 10, repast::Schedule::FunctorPtr(new repast::MethodFunctor<repast::DataSet>(agentValues, &repast::DataSet::write)));
	runner.scheduleEndEvent(repast::Schedule::FunctorPtr(new repast::MethodFunctor<repast::DataSet>(agentValues, &repast::DataSet::write)));
}

void RepastHPCDemoModel::recordResults(){
	if(repast::RepastProcess::instance()->rank() == 0){
		props->putProperty("Result","Passed");
		std::vector<std::string> keyOrder;
		keyOrder.push_back("RunNumber");
		keyOrder.push_back("stop.at");
		keyOrder.push_back("Result");
		props->writeToSVFile("./output/results.csv", keyOrder);
    }
}
Region::Region(string rID, int rDedicatedRoadSpace, int rDriverMultiplier, int rHilliness, bool rElectricBikes){
	ID = rID;
	dedicatedRoadSpace = rDedicatedRoadSpace;
	noOfCyclistsPerDay = 0;
	noOfDriversPerDay = 0;
	driverMultiplier = rDriverMultiplier;
	hilliness = rHilliness;
	percievedRoadSafety = 1;
	electricBikes = rElectricBikes;	
	std::cout<<"Create Region: "<<ID<<endl;
}

void Region::calcPervcievedRoadSafety(){
	percievedRoadSafety=(getPercOfCyclistsPerDay())*50 + (double(dedicatedRoadSpace)/10)*50;
	//std :: cout << (double(noOfCyclistsPerDay)/double(noOfDriversPerDay*driverMultiplier+noOfCyclistsPerDay)) << " " << (double(100-hilliness)/100)<< " " << (double(dedicatedRoadSpace)/100) << " " << percievedRoadSafety << std :: endl;
}
bool Region::getEB(){
	return electricBikes;
}
double Region::getPRS(){
	return percievedRoadSafety;
}

void Region::resetCyclists(){
	noOfCyclistsPerDay = 0;
}

void Region::incCyclists(){
	noOfCyclistsPerDay++;
}
void Region::resetDrivers(){
	noOfDriversPerDay = 0;
}

void Region::incDrivers(){
	noOfDriversPerDay++;
}
