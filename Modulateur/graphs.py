import matplotlib.pyplot as plt
import pandas

# for grace to not forget : had to import these libs to virt env with 
# /Users/gracewilcox/Documents/sorbonne/printemps\ 2025/PPSE/.venv/bin/python -m pip install matplotlib
# and to run
# /Users/gracewilcox/Documents/sorbonne/printemps\ 2025/PPSE/.venv/bin/python graphs.py

## Files = (namefile, legend, print format)
## with print format being first the dot style (x, +, . ...) and then the line style (usually - for continuous or -- for dashed)
files = [ 
         ("sim_soft_stats.csv", "Float", "s-"),
         ("sim_soft8_stats.csv", "Fixed", "^-"),
         ("sim_soft_neon_stats.csv", "Neon", "x-")
]

output = "decode_soft"
xlabel = "Signal to Noise Ratio (Eb/N0) (dB)"  
ylabel = "Average time for decoding (microseconds)"
x = "Eb/No"
y = "decode_avg"

for elem in files:
    sim = pandas.read_csv(elem[0])
    simX = sim[[x]]
    simY = sim[[y]]
    print(simX, simY)

    plt.plot(simX, simY, elem[2], label=elem[1])

plt.xlabel(xlabel)
plt.ylabel(ylabel)
# plt.yscale("log")
plt.legend()
plt.grid()
plt.savefig(output+".jpg", format="jpg")