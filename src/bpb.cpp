#include "bpb.hpp"

bpb::bpb(unsigned int t, unsigned int z): size(t), n_bits(z){
    max = (1<<n_bits) - 1; // max = 3
    msb = (1 << (n_bits - 1)); // msb = 2
    state = 0; // pode ser tirado e colocado automaticamente na inicialização abaixo
    c_predictions = 0;
    c_hits = 0;
    bp_buffer.resize(size);
}

bool bpb::bpb_predict(unsigned int pc){
    c_predictions++;
    return (bp_buffer[pc % size]&msb);
}

void bpb::bpb_update_state(unsigned int pc, bool taken, bool hit){
    int i = pc % size;
    int state_aux = bp_buffer[i];

    c_hits += hit;
        
    if(taken){
        state_aux += (state_aux < max); // = ++state_aux>max?max:state_aux;
    }else{
        state_aux -= (state_aux > 0); // = --state_aux<0?0:state_aux;
    }

    bp_buffer[i] = state_aux;    
}

float bpb::bpb_get_hit_rate(){
    return ((float)c_hits / (float)c_predictions) * 100;
}

int bpb::get_bpb_size(){
    return size;
}