import matplotlib.pyplot as plt
import pandas

# for grace to not forget : had to import these libs to virt env with 
# /Users/gracewilcox/Documents/sorbonne/printemps\ 2025/PPSE/.venv/bin/python -m pip install matplotlib
# and to run
# /Users/gracewilcox/Documents/sorbonne/printemps\ 2025/PPSE/.venv/bin/python graphs.py

## Files = (namefile, legend, print format)
## with print format being first the dot style (x, +, . ...) and then the line style (usually - for continuous or -- for dashed)
files = [ 
         ("sim_test_demod.csv", "neon demod", "s-"),
         ("./TP4/Task6/sim_hard8.csv", "normal demod", "^-"),
        # ("sim_hard_neon_stats.csv", "Neon", "x-")
]

output = "testing_demod_neon"
xlabel = "Signal to Noise Ratio (Eb/N0) (dB)"  
ylabel = "BER"
x = "Eb/No"
y = "BER"

for elem in files:
    sim = pandas.read_csv(elem[0])
    simX = sim[[x]]
    simY = sim[[y]]
    print(simX, simY)

    plt.plot(simX, simY, elem[2], label=elem[1])

plt.xlabel(xlabel)
plt.ylabel(ylabel)
plt.yscale("log")
plt.legend()
plt.grid()
plt.savefig(output+".jpg", format="jpg")