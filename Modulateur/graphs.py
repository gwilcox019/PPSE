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
         ("FINAL SIMS/normal/sim_", "Normal - K = 32", "plum", "x", "--"),
         ("FINAL SIMS/normal/big_k/sim_", "Normal - K = 128", "purple", "o", "--"),
         ("FINAL SIMS/neon_mod/sim_", "Neon mod - K = 32", "cyan", "x", "-"),
         ("FINAL SIMS/neon_mod/big_k/sim_", "Neon mod - K = 128", "darkcyan", "o", "-"), 
]

for t in types:
    #output = "FINAL GRAPHS/neon_modul/block throughput/"+t
    xlabel = "Signal to Noise Ratio (Eb/N0) (dB)"  
    ylabel = "Modulator throughput (Mbps)"
    x = "Eb/No"
    y1 = "bpsk_thr"

    plt.figure()
    for elem in files:
        sim = pandas.read_csv(elem[0]+t+"_stats.csv")
        simX = sim[[x]]
        simY = sim[[y1]]
        plt.plot(simX, simY, color=elem[2], marker=elem[3], linestyle=elem[4], label=elem[1])
        #simY = sim[[y2]]
        #plt.plot(simX, simY, color=elem[2], marker=elem[3], linestyle=elem[4], label=elem[1])
        #simY = sim[[y3]]
        #plt.plot(simX, simY, color=elem[2], marker="o", linestyle=elem[4], label="FER "+elem[1])

    plt.xlabel(xlabel)
    plt.ylabel(ylabel)
    plt.yscale("log")
    plt.grid()
    plt.legend()
    plt.savefig("FINAL GRAPHS/neon_modul/block throughput/"+t+".jpg", format="jpg")
    