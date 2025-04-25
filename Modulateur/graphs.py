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
         ("normal/sim_", "Normal - K = 32", "yx--"),
         #("normal/big_k/sim_", "Normal - K = 128", "gx--"),
         ("neon_mod/sim_", "Neon modulator - K = 32", "r.-"),
         #("neon_demod/big_k/sim_", "Neon demodulator - K = 128", "b.-"), 
]

for t in types:
    output = "FINAL GRAPHS/neon_modul/block throughput/"+t
    xlabel = "Signal to Noise Ratio (Eb/N0) (dB)"  
    ylabel = "Demodulator throughput (Mbps)"
    x = "Eb/No"
    y = "demodulate_thr"

    plt.figure()
    for elem in files:
        sim = pandas.read_csv(elem[0]+t+"_stats.csv")
        simX = sim[[x]]
        simY = sim[[y]]
        plt.plot(simX, simY, elem[2], label=elem[1])

    plt.xlabel(xlabel)
    plt.ylabel(ylabel)
    plt.yscale("log")
    plt.legend()
    plt.grid()
    plt.savefig(output+".jpg", format="jpg")
    