import matplotlib.pyplot as plt
import pandas

# for grace to not forget : had to import these libs to virt env with 
# /Users/gracewilcox/Documents/sorbonne/printemps\ 2025/PPSE/.venv/bin/python -m pip install matplotlib
# and to run
# /Users/gracewilcox/Documents/sorbonne/printemps\ 2025/PPSE/.venv/bin/python graphs.py

## Files = (namefile, legend, print format)
## with print format being first the dot style (x, +, . ...) and then the line style (usually - for continuous or -- for dashed)
types = ["hard", "soft_32", "soft_64", "soft_96", "soft_128"]

files = [ 
         ("FINAL SIMS/normal/sim_", "Normal - K = 32", "yx--"),
         ("FINAL SIMS/normal/big_k/sim_", "Normal - K = 128", "gx--"),
         ("FINAL SIMS/bit_pack/sim_", "Bit packing - K = 32", "r.-"),
         ("FINAL SIMS/bit_pack/big_k/sim_", "Bit packing - K = 128", "b.-"), 
]

for t in types:
    output = "FINAL GRAPHS/neon_modul/block throughput/"+t
    xlabel = "Signal to Noise Ratio (Eb/N0) (dB)"  
    ylabel = "Demodulator throughput (Mbps)"
    x = "Eb/No"
    y1 = "gen_thr"
    y2 = "encode_thr"
    y3 = "mod_thr"

    plt.figure()
    for elem in files:
        sim = pandas.read_csv(elem[0]+t+"_stats.csv")
        simX = sim[[x]]
        simY = sim[[y1]]
        plt.plot(simX, simY, elem[2], label=elem[1])
        simY = sim[[y2]]
        plt.plot(simX, simY, elem[2], label=elem[1])
        simY = sim[[y3]]
        plt.plot(simX, simY, elem[2], label=elem[1])

    plt.xlabel(xlabel)
    plt.ylabel(ylabel)
    plt.yscale("log")
    plt.legend()
    plt.grid()
    plt.savefig("FINAL GRAPHS/bit_pack/block_throughput/"+t+".jpg", format="jpg")
    