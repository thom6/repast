/* Cycle_Network_Agent.cpp */

#include "Cycle_Network_Agent.h"

RepastHPCDemoAgent::RepastHPCDemoAgent(repast::AgentId id): id_(id), c(100), total(200), iCycle(rand() % 2), popularity(5), r(rand()%9){}

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
    popularity	       = 0;
    while(agentToPlay != agentsToPlay.end()){
        boost::shared_ptr<DemoModelCustomEdge<RepastHPCDemoAgent> > edge = network->findEdge(this, *agentToPlay);
        double edgeWeight = edge->weight();
        int confidence = edge->getConfidence();
                       // Do I cooperate?
        popularity += (iCycle ?
						 ((*agentToPlay)->cycle() ?  10 : 4) :     // If I cooperated, did my opponent?
						 ((*agentToPlay)->cycle() ? 6 : 1));     // If I didn't cooperate, did my opponent?
	netSize++;
        agentToPlay++;
    }
    popularity=popularity/(netSize-1);
}

void RepastHPCDemoAgent::updateCycle(bool EB, double PRS,int H){
	int random=rand() %100;
	double chance= (popularity/10)*2+(PRS/100)*1+(1*(1-(double(H)*(~EB)))/100);
	iCycle = chance>random;
	//std::cout<< (popularity/10)*40+(PRS/100)*30+(30*EB) <<std::endl; (popularity/10)*20+(PRS/100)*40+((40*(100-double(H))*(1-EB))/100)
}

/* Serializable Agent Package Data */

RepastHPCDemoAgentPackage::RepastHPCDemoAgentPackage(){ }

RepastHPCDemoAgentPackage::RepastHPCDemoAgentPackage(int _id, int _rank, int _type, int _currentRank, double _c, double _total):
id(_id), rank(_rank), type(_type), currentRank(_currentRank), c(_c), total(_total){ }
