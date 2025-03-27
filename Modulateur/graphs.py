import matplotlib.pyplot as plt
import pandas
import argparse

# Doesn't work snif snif
# parser = argparse.ArgumentParser()
# parser.add_argument("-x")
# parser.add_argument("-y")
# parser.add_argument("-dest")
# parser.add_argument('-files', nargs='*', dest='files', action='append', type="str")

# args = parser.parse_args()
# files = args.files
# x = args.x
# y = args.y
# dest = args.dest

# for elem in files:
#     sim = pandas.read_csv(elem)
#     xaxis = sim[[x]]
#     yaxis = sim[[y]]
#     plt.plot(xaxis, yaxis, label=x)

# plt.yscale("log")
# plt.xlabel("Signal to Noise Ratio (EB/N0) (dB)")
# plt.ylabel("Error rate")
# plt.legend()
# plt.grid()
# plt.savefig(dest+".jpg", format="jpg")


# Graph1
# sim1 = pandas.read_csv("sim_1.csv")
# sim1_EB = sim1[["Eb/No"]]
# sim1_BER = sim1[[" BER"]]
# sim1_FER = sim1[[" FER"]]
# sim2 = pandas.read_csv("sim_2.csv")
# sim2_EB = sim2[["Eb/No"]]
# sim2_BER = sim2[[" BER"]]
# sim2_FER = sim2[[" FER"]]

# plt.plot(sim1_EB, sim1_BER, "bo--", label="BER Hard")
# plt.plot(sim1_EB, sim1_FER, "bx-", label="FER Hard")
# plt.plot(sim2_EB, sim2_BER, "go--", label="BER Soft")
# plt.plot(sim2_EB, sim2_FER, "gx-", label="FER Soft")
# plt.yscale("log")
# plt.xlabel("Signal to Noise Ratio (EB/N0) (dB)")
# plt.ylabel("Error rate")
# plt.legend()
# plt.grid()
# plt.savefig("graph1.jpg", format="jpg")


# Graph 2
sim2 = pandas.read_csv("sim_2.csv")
sim2_EB = sim2[["Eb/No"]]
sim2_FER = sim2[[" FER"]]
sim3 = pandas.read_csv("sim_3.csv")
sim3_EB = sim3[["Eb/No"]]
sim3_FER = sim3[[" FER"]]
sim4 = pandas.read_csv("sim_4.csv")
sim4_EB = sim4[["Eb/No"]]
sim4_FER = sim4[[" FER"]]
sim5 = pandas.read_csv("sim_5.csv")
sim5_EB = sim5[["Eb/No"]]
sim5_FER = sim5[[" FER"]]

plt.plot(sim2_EB, sim2_FER, "x-", label="Coderate 0.25")
plt.plot(sim3_EB, sim3_FER, "x-", label="Coderate 0.33")
plt.plot(sim4_EB, sim4_FER, "x-", label="Coderate 0.5")
plt.plot(sim5_EB, sim5_FER, "x-", label="Coderate 1")
plt.yscale("log")
plt.xlabel("Signal to Noise Ratio (EB/N0) (dB)")
plt.ylabel("FER")
plt.legend()
plt.grid()
plt.savefig("graph2.jpg", format="jpg")

# Make it a script : take into parameters an array of cols we want and an array of files we want to use