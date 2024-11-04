#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <verilated.h>
#include <verilated_fst_c.h>
#include "Vtop.h"
#define MAX_SIM_TIME 20
vluint64_t sim_time = 0;

void init(Vtop* top)
{
    sim_time = 0;
    top -> clk = 0;
    top -> rst_n = 0;
    top -> a = 0;
    top -> b = 0;
}

int main(int argc, char ** argv)
{
    Verilated::commandArgs(argc, argv);
    Verilated::traceEverOn(true);
    Vtop* top = new Vtop("top");
    VerilatedFstC* fstc = new VerilatedFstC();
    top -> trace(fstc, 5, 0);
    fstc -> open("./obj_dir/wave.fst");
    init(top);

    while (sim_time < MAX_SIM_TIME)
    {   
        top->clk ^= 1;    
        if (sim_time == 1)
            top->rst_n = 1;
        if (top->clk == 0 && top->rst_n == 1)
        {
            int a = rand() % 4;
            int b = rand() % 4;
            top->a = a;
            top->b = b;    
        }
        top->eval();
        fstc->dump(sim_time);
        sim_time++;     
    }

    fstc -> close();
    delete top;
    return 0;
}
