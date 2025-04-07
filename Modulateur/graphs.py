import matplotlib.pyplot as plt
import pandas

# for grace to not forget : had to import these libs to virt env with 
# /Users/gracewilcox/Documents/sorbonne/printemps\ 2025/PPSE/.venv/bin/python -m pip install matplotlib
# and to run
# /Users/gracewilcox/Documents/sorbonne/printemps\ 2025/PPSE/.venv/bin/python graphs.py

## Files = (namefile, legend, print format)
## with print format being first the dot style (x, +, . ...) and then the line style (usually - for continuous or -- for dashed)
files = [("TP3/sim_2.csv", "Float - R = 1/4", "x-"),
         ("TP3/sim_3.csv", "Float - R = 1/3", "s-"),
         ("TP3/sim_4.csv", "Float - R = 1/2", "p-"),
         ("TP3/sim_5.csv", "Float - R = 1", ".-"),
         ("sim_2_2_5.csv", "Fixed - R = 1/4", "x-"),
         ("sim_3_3_5.csv", "Fixed - R = 1/3", "s-"),
         ("sim_4_4_5.csv", "Fixed - R = 1/2", "p-"),
         ("sim_5_4_5.csv", "Fixed - R = 1", ".-")
]

output = "graph2"
xlabel = "Signal to Noise Ratio (Eb/N0) (dB)"  
ylabel = "Frame Error Rate"
x = "Eb/No"
y = "FER"

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