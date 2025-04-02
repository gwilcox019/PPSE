import matplotlib.pyplot as plt
import pandas

# for grace to not forget : had to import these libs to virt env with 
# /Users/gracewilcox/Documents/sorbonne/printemps\ 2025/PPSE/.venv/bin/python -m pip install matplotlib
# and to run
# /Users/gracewilcox/Documents/sorbonne/printemps\ 2025/PPSE/.venv/bin/python graphs.py

## Files = (namefile, legend, print format)
## with print format being first the dot style (x, +, . ...) and then the line style (usually - for continuous or -- for dashed)
files = [("testsim_1_zeroes_stats.csv", "Random numbers", "x-")]
output = "compare_sim1"
xlabel = "Signal to Noise Ratio (EB/N0) (dB)"  
ylabel = "Average generation time"
x = "SNR"
y = "gen_avg"

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