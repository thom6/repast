/* Cycle_Network_Model.h */

#ifndef Cycle_Network_MODEL
#define Cycle_Network_MODEL

#include <boost/mpi.hpp>
#include "repast_hpc/Schedule.h"
#include "repast_hpc/Properties.h"
#include "repast_hpc/SharedContext.h"
#include "repast_hpc/AgentRequest.h"
#include "repast_hpc/TDataSource.h"
#include "repast_hpc/SVDataSet.h"
#include "repast_hpc/SharedNetwork.h"

#include "Cycle_Network_Network.h"
#include "Cycle_Network_Agent.h"


class Region{
private:
	string ID;
	int dedicatedRoadSpace;
	int noOfCyclistsPerDay;
	int noOfDriversPerDay;
	int driverMultiplier;
	int hilliness;
	double percievedRoadSafety;
	bool electricBikes;
public:
	Region(string rID, int rDedicatedRoadSpace, int rDriverMultiplier, int rHilliness, bool rElectricBikes);
	void calcPervcievedRoadSafety();
	bool getEB();
	double getPRS();
	void resetCyclists();
	void incCyclists();
	void resetDrivers();
	void incDrivers();
	void increaseDRS(){				dedicatedRoadSpace++;}
	void buildEB(){					electricBikes = true;}
	string getID(){					return ID;}
	double getPercOfCyclistsPerDay(){		return double(noOfCyclistsPerDay)/double(noOfCyclistsPerDay+noOfDriversPerDay);}
	double getHilliness(){				return hilliness;}
};

/* Agent Package Provider */
class RepastHPCDemoAgentPackageProvider {
	
private:
    repast::SharedContext<RepastHPCDemoAgent>* agents;
	
public:
	
    RepastHPCDemoAgentPackageProvider(repast::SharedContext<RepastHPCDemoAgent>* agentPtr);
	
    void providePackage(RepastHPCDemoAgent * agent, std::vector<RepastHPCDemoAgentPackage>& out);
	
    void provideContent(repast::AgentRequest req, std::vector<RepastHPCDemoAgentPackage>& out);
	
};

/* Agent Package Receiver */
class RepastHPCDemoAgentPackageReceiver {
	
private:
    repast::SharedContext<RepastHPCDemoAgent>* agents;
	
public:
	
    RepastHPCDemoAgentPackageReceiver(repast::SharedContext<RepastHPCDemoAgent>* agentPtr);
	
    RepastHPCDemoAgent * createAgent(RepastHPCDemoAgentPackage package);
	
    void updateAgent(RepastHPCDemoAgentPackage package);
	
};


/* Data Collection */
class DataSource_AgentTotals : public repast::TDataSource<int>{
private:
	repast::SharedContext<RepastHPCDemoAgent>* context;

public:
	DataSource_AgentTotals(repast::SharedContext<RepastHPCDemoAgent>* c);
	int getData();
};
	

class DataSource_AgentCTotals : public repast::TDataSource<int>{
private:
	repast::SharedContext<RepastHPCDemoAgent>* context;
	
public:
	DataSource_AgentCTotals(repast::SharedContext<RepastHPCDemoAgent>* c);
	int getData();
};

class RepastHPCDemoModel{
	int stopAt;
	int countOfAgents;
	int countOfRegions;
	std::vector<Region> Regions;
	repast::Properties* props;
	repast::SharedContext<RepastHPCDemoAgent> context;
	
	RepastHPCDemoAgentPackageProvider* provider;
	RepastHPCDemoAgentPackageReceiver* receiver;
    
    DemoModelCustomEdgeContentManager<RepastHPCDemoAgent> edgeContentManager;

	repast::SVDataSet* agentValues;
	repast::SharedNetwork<RepastHPCDemoAgent, DemoModelCustomEdge<RepastHPCDemoAgent>, DemoModelCustomEdgeContent<RepastHPCDemoAgent>, DemoModelCustomEdgeContentManager<RepastHPCDemoAgent> >* agentNetwork;
	
public:
	RepastHPCDemoModel(std::string propsFile, int argc, char** argv, boost::mpi::communicator* comm);
	~RepastHPCDemoModel();
	void init(std::string propsFile);
	void connectAgentNetwork();
	void moveAgents();
	void doSomething();
	void countRegions();
	void buildInfrastructure();
	void initSchedule(repast::ScheduleRunner& runner);
	void recordResults();
};

#endif
