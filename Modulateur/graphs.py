import matplotlib.pyplot as plt
import pandas

## Files = (namefile, legend, print format)
## with print format being first the dot style (x, +, . ...) and then the line style (usually - for continuous or -- for dashed)
files = [("sim_2.csv", "Coderate 0.25", "x-"), ("sim_3.csv", "Coderate 0.33", "x-"), 
         ("sim_4.csv", "Coderate = 0.5", "x-"), ("sim_5.csv", "Coderate = 1", "x-")]
output = "test"
xlabel = "Signal to Noise Ratio (EB/N0) (dB)"
ylabel = "ylabel"
x = " Es/No"
y = " FER"

for elem in files:
    sim = pandas.read_csv(elem[0])
    simX = sim[[x]]
    simY = sim[[y]]
    plt.plot(simX, simY, elem[2], label=elem[1])

plt.yscale("log")
plt.xlabel(xlabel)
plt.ylabel(ylabel)
plt.legend()
plt.grid()
plt.savefig(output+".jpg", format="jpg")