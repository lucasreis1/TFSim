#include <vector>
#include <iostream>

using namespace std;

class bpb {

public:
    bpb(unsigned int t, unsigned int z);
    bool bpb_predict(unsigned int pc);
    void bpb_update_state(unsigned int pc, bool taken, bool hit);
    float bpb_get_hit_rate();
    int get_bpb_size();

private:
    std::vector<int> bp_buffer;
    int size, n_bits, max, msb, state, c_predictions, c_hits;
};