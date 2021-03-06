/* Cycle_Network_Agent.h */

#ifndef Cycle_Network_AGENT
#define Cycle_Network_AGENT

#include "repast_hpc/AgentId.h"
#include "repast_hpc/SharedContext.h"
#include "repast_hpc/SharedNetwork.h"
#include "Cycle_Network_Network.h"

/* Agents */
class RepastHPCDemoAgent{
	
private:
    repast::AgentId   id_;
    double              c;
    double          total;
    bool	   iCycle;
    double     popularity;
    int 		r;

public:
    RepastHPCDemoAgent(repast::AgentId id, int newr);
	RepastHPCDemoAgent(){}
    RepastHPCDemoAgent(repast::AgentId id, double newC, double newTotal);
	
    ~RepastHPCDemoAgent();
	
    /* Required Getters */
    virtual repast::AgentId& getId(){                   return id_;    }
    virtual const repast::AgentId& getId() const {      return id_;    }
	
    /* Getters specific to this kind of Agent */
    double getC(){                                      return c;      }
    double getTotal(){                                  return total;  }
    int getr(){						return r;      }
    /* Setter */
    void set(int currentRank, double newC, double newTotal);
    void setr(int newr){				r = newr;      }
	
    /* Actions */
    bool cycle();
    double popular();                                                 
    void play(repast::SharedNetwork<RepastHPCDemoAgent,
              DemoModelCustomEdge<RepastHPCDemoAgent>,
              DemoModelCustomEdgeContent<RepastHPCDemoAgent>,
              DemoModelCustomEdgeContentManager<RepastHPCDemoAgent> > *network);
    void updateCycle(bool EB, double PRS,int H);
	
};

/* Serializable Agent Package */
struct RepastHPCDemoAgentPackage {
	
public:
    int    id;
    int    rank;
    int    type;
    int    currentRank;
    double c;
    double total;
	
    /* Constructors */
    RepastHPCDemoAgentPackage(); // For serialization
    RepastHPCDemoAgentPackage(int _id, int _rank, int _type, int _currentRank, double _c, double _total);
	
    /* For archive packaging */
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version){
        ar & id;
        ar & rank;
        ar & type;
        ar & currentRank;
        ar & c;
        ar & total;
    }
	
};


#endif
