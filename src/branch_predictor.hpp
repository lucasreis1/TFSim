class branch_predictor
{
public:
    branch_predictor(unsigned int t);
    bool predict();
    void update_state(bool taken, bool hit);
    float get_predictor_hit_rate();
    
private:
    int n_bits,max,state, c_predictions, c_hits;
};
