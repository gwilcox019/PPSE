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
         ("normal/sim_", "Normal - K = 32", "y--"),
         ("normal/big_k/sim_", "Normal - K = 128", "g--"),
         ("neon_monitor/sim_", "With threads - K = 32", "r-"),
         ("neon_monitor/big_k/sim_", "With threads - K = 128", "b-"), 
]

for t in types:
    output = "FINAL GRAPHS/threads/"+t
    xlabel = "Signal to Noise Ratio (Eb/N0) (dB)"  
    ylabel = "Error Rates"
    x = "Eb/No"
    y1 = "BER"
    y2 = "FER"

    plt.figure()
    for elem in files:
        sim = pandas.read_csv(elem[0]+t+".csv")
        simX = sim[[x]]
        simY = sim[[y1]]
        plt.plot(simX, simY, "o"+elem[2], label="BER "+elem[1])
        simY = sim[[y2]]
        plt.plot(simX, simY, "s"+elem[2], label="FER "+elem[1])

    plt.xlabel(xlabel)
    plt.ylabel(ylabel)
    plt.yscale("log")
    plt.legend()
    plt.grid()
    plt.savefig(output+".jpg", format="jpg")
    