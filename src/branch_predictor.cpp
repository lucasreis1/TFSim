#include "branch_predictor.hpp"

branch_predictor::branch_predictor(unsigned int t): n_bits(t)
{
    max = (1<<n_bits)-1;
    state = 0;
    c_predictions = 0;
    c_hits = 0;
}

bool branch_predictor::predict()
{
    c_predictions++;
    return state&(1<<(n_bits-1));
}
void branch_predictor::update_state(bool taken, bool hit)
{
    c_hits += hit;

    if(taken)
    {
        state = ++state>max?max:state; 
    }
    else if(state)
    {
        state = --state<0?0:state;
    }
}

float branch_predictor::get_predictor_hit_rate() {
    return (float) c_hits / (float) c_predictions * 100;
}
