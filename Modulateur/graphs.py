import matplotlib.pyplot as plt
import pandas

# for grace to not forget : had to import these libs to virt env with 
# /Users/gracewilcox/Documents/sorbonne/printemps\ 2025/PPSE/.venv/bin/python -m pip install matplotlib
# and to run
# /Users/gracewilcox/Documents/sorbonne/printemps\ 2025/PPSE/.venv/bin/python graphs.py

## Files = (namefile, legend, print format)
## with print format being first the dot style (x, +, . ...) and then the line style (usually - for continuous or -- for dashed)
files = [ 
         ("sim_mod_stats.csv", "Neon demod", "r"),
         ("sim_modnor_stats.csv", "Normal demod", "b")
]

output = "mod time"
xlabel = "Signal to Noise Ratio (Eb/N0) (dB)"  
ylabel = "Avg modulation time"
x = "Eb/No"
y1 = "bpsk_avg"
#y2 = "BER"

for elem in files:
    sim = pandas.read_csv(elem[0])
    simX = sim[[x]]
    simY = sim[[y1]]
    plt.plot(simX, simY, elem[2]+"o-", label=elem[1])

    #simY = sim[[y2]]
    #plt.plot(simX, simY, elem[2]+"x-", label="BER "+elem[1])

plt.xlabel(xlabel)
plt.ylabel(ylabel)
#plt.yscale("log")
plt.legend()
plt.grid()
plt.savefig(output+".jpg", format="jpg")