import matplotlib.pyplot as plt
import pandas

# for grace to not forget : had to import these libs to virt env with 
# /Users/gracewilcox/Documents/sorbonne/printemps\ 2025/PPSE/.venv/bin/python -m pip install matplotlib
# and to run
# /Users/gracewilcox/Documents/sorbonne/printemps\ 2025/PPSE/.venv/bin/python graphs.py

## Files = (namefile, legend, print format)
## with print format being first the dot style (x, +, . ...) and then the line style (usually - for continuous or -- for dashed)
files = [ 
         ("sim_hard.csv", "Float", "p-"),
         ("sim_hard8.csv", "Fixed", "s-"),
         ("sim_hard_neon.csv", "Neon", "x-"),
         ("sim_hard_random.csv", "Random", ".-")
]

output = "hard_comp"
xlabel = "Signal to Noise Ratio (Eb/N0) (dB)"  
ylabel = "Bit Error Rate"
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