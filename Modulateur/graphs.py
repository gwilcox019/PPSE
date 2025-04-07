import matplotlib.pyplot as plt
import pandas

# for grace to not forget : had to import these libs to virt env with 
# /Users/gracewilcox/Documents/sorbonne/printemps\ 2025/PPSE/.venv/bin/python -m pip install matplotlib
# and to run
# /Users/gracewilcox/Documents/sorbonne/printemps\ 2025/PPSE/.venv/bin/python graphs.py

## Files = (namefile, legend, print format)
## with print format being first the dot style (x, +, . ...) and then the line style (usually - for continuous or -- for dashed)
files = [
         ("sim_1_2_4.csv", "Sim1-s4f2", "x-"),
         ("sim_1_2_5.csv", "Sim1-s5f2", "x-"),
         ("sim_1_2_6.csv", "Sim1-s6f2", "x-"),
         ("sim_1_3_4.csv", "Sim1-s4f3", "x-"),
         ("sim_1_3_5.csv", "Sim1-s5f3", "x-"),
         ("sim_1_3_6.csv", "Sim1-s6f3", "x-"),
         ("sim_1_4_5.csv", "Sim1-s5f4", "x-"),
         ("sim_1_4_6.csv", "Sim1-s6f4", "x-"),
         ("sim_1_5_6.csv", "Sim1-s6f5", "x-"),
         ]
output = "compare_simidkman"
xlabel = "Signal to Noise Ratio (EB/N0) (dB)"  
ylabel = "Average generation time"
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
plt.legend()
plt.grid()
plt.savefig(output+".jpg", format="jpg")