/* Cycle_Network_Agent.cpp */

#include "Cycle_Network_Agent.h"

RepastHPCDemoAgent::RepastHPCDemoAgent(repast::AgentId id, int newr): id_(id), c(100), total(200), iCycle(rand() % 2), popularity(5), r(newr){}

RepastHPCDemoAgent::RepastHPCDemoAgent(repast::AgentId id, double newC, double newTotal): id_(id), c(newC), total(newTotal), iCycle(rand() % 2), popularity(5){ }

RepastHPCDemoAgent::~RepastHPCDemoAgent(){ }


void RepastHPCDemoAgent::set(int currentRank, double newC, double newTotal){
    id_.currentRank(currentRank);
    c     = newC;
    total = newTotal;
}

bool RepastHPCDemoAgent::cycle(){
	return iCycle;
}

double RepastHPCDemoAgent::popular(){
	return popularity;
}

void RepastHPCDemoAgent::play(repast::SharedNetwork<RepastHPCDemoAgent,
                              DemoModelCustomEdge<RepastHPCDemoAgent>,
                              DemoModelCustomEdgeContent<RepastHPCDemoAgent>,
                              DemoModelCustomEdgeContentManager<RepastHPCDemoAgent> > *network){
    std::vector<RepastHPCDemoAgent*> agentsToPlay;
    network->successors(this, agentsToPlay);

    std::vector<RepastHPCDemoAgent*>::iterator agentToPlay = agentsToPlay.begin();
    int netSize        = 1;
    while(agentToPlay != agentsToPlay.end()){
        boost::shared_ptr<DemoModelCustomEdge<RepastHPCDemoAgent> > edge = network->findEdge(this, *agentToPlay);


        popularity += (iCycle ?
						 ((*agentToPlay)->cycle() ?  10 : 4) :    
						 ((*agentToPlay)->cycle() ? 6 : 1));  
	netSize++;
        agentToPlay++;
    }
    popularity=popularity/(netSize-1);
    
}

void RepastHPCDemoAgent::updateCycle(bool EB, double PRS,int H){
	int random=rand() %100;
	double chance= ((popularity-1)*1)+((PRS/100)*2)+(5*(100-(double(H)*(!EB)))/100);
	//std::cout<<(popularity-1)*1<<" "<<(PRS/100)*2<<" "<<(5*(100-(double(H)*(!EB)))/100) << " " << chance << !EB <<std::endl;
	iCycle = chance>random;
	
}

/* Serializable Agent Package Data */

RepastHPCDemoAgentPackage::RepastHPCDemoAgentPackage(){ }

RepastHPCDemoAgentPackage::RepastHPCDemoAgentPackage(int _id, int _rank, int _type, int _currentRank, double _c, double _total):
id(_id), rank(_rank), type(_type), currentRank(_currentRank), c(_c), total(_total){ }
