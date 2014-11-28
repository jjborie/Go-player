#ifndef __MCTS_GO__

#define __MCTS_GO__

#include <random>
#include "mcts_uct.hpp"
#include "mcts_rave.hpp"
#include "state_go.hpp"
#include "moverecorder_go.hpp"

struct EvalNod : EvalNode<ValGo,DataGo> {
    ValGo operator()(ValGo v_nodo,ValGo v_final,DataGo dat_nodo)
    {
        if(v_final == dat_nodo.player)
            return v_nodo+1;
        return v_nodo;
    }
};

class SimulationWithDomainKnowledge: public SimulationAndRetropropagationRave<ValGo,DataGo,StateGo,EvalNod,MoveRecorderGo> {
    protected:
        int mov_counter;
        int mov_limit;
        int _fill_board_n;
        double _long_game_coeff;
        int _limit_atari;
        void get_possible_moves(StateGo *state,std::vector<DataGo> &v,std::uniform_int_distribution<int> &mov_dist);
    public:
        SimulationWithDomainKnowledge(int number_fill_board_attemps,double long_game_coeff,int limit_atari): SimulationAndRetropropagationRave<ValGo,DataGo,StateGo,EvalNod,MoveRecorderGo>(),_fill_board_n(number_fill_board_attemps), _long_game_coeff(long_game_coeff), _limit_atari(limit_atari) {};
        ValGo simulate(StateGo *state);
};

class SimulationAndRetropropagationRaveGo: public SimulationAndRetropropagationRave<ValGo,DataGo,StateGo,EvalNod,MoveRecorderGo> {
    protected:
        double _long_game_coeff;
    public:
        SimulationAndRetropropagationRaveGo(double long_game_coeff): SimulationAndRetropropagationRave<ValGo,DataGo,StateGo,EvalNod,MoveRecorderGo>(), _long_game_coeff(long_game_coeff) {};
        ValGo simulate(StateGo *state);
};

class SimulationTotallyRandomGo: public Simulation<ValGo,StateGo> {
    private:
        double _long_game_coeff;
        std::mt19937 mt;
    public:
        SimulationTotallyRandomGo(double long_game_coeff);
        ValGo simulate(StateGo *state);
};

template <class Node>
class SelectResMostRobustOverLimit: public SelectRes<DataGo,Node> {
    private:
        double _limit;
    public:
        SelectResMostRobustOverLimit(double limit):_limit(limit){};
        DataGo select_res(Node *node);
};

/////////////////////////////////////////////////////////////////////////////////////////


template <class Node>
DataGo SelectResMostRobustOverLimit<Node>::select_res(Node *node)
{
    assert(!node->children.empty());
    unsigned long max_visits = node->children[0]->visits;
    Node *max_node = node->children[0];
    for(int i=1;i<node->children.size();i++)
        if(node->children[i]->visits > max_visits){
            max_node = node->children[i];
            max_visits = node->children[i]->visits;
        }
    if(max_node->visits!=0 && (max_node->value / max_node->visits) < _limit)
        return RESIGN(max_node->data.player);
    return max_node->data;
}

#endif //__MCTS_GO__
