import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

df1 = pd.read_csv("/Users/yw18581/work/theranostics/rn222_repetition_runs/rn222_2mmfully_10reps_10k/outputs/2mm_2mm_10k_baseline_rep01_photon_boundaries.csv")
df2 = pd.read_csv("/Users/yw18581/work/theranostics/rn222_repetition_runs/rn222_2mmfully_10reps_10k/outputs/2mm_2mm_10k_90diffRn222_rep01_photon_boundaries.csv")
df3 = pd.read_csv("/Users/yw18581/work/theranostics/rn222_repetition_runs/rn222_2mmfully_10reps_10k/outputs/2mm_2mm_10k_10diffRn222_rep01_photon_boundaries.csv")

data = [df1, df2, df3]
labels = ["baseline", "90Rn222", "10Rn222"]


plt.figure()

bins_energy = np.linspace(0, 3, 40)

for df, label in zip(data, labels):

    filtered = df[
        (df["volumeName"] == "physPhotonDetectorPosY") &
        (df["boundary"] == "enter")
    ]

    energy = filtered["kineticEnergy_MeV"]

    plt.hist(
        energy,
        bins=bins_energy,
        histtype="step",
        label=label
    )

plt.xlabel("Kinetic Energy (MeV)")
plt.ylabel("Count")
plt.title("Energy comparison")
plt.legend()
plt.show()


plt.figure()

for df, label in zip(data, labels):

    filtered = df[
        (df["volumeName"] == "physPhotonDetectorPosY") &
        (df["boundary"] == "enter")
    ]

    zpos = filtered["z_mm"]

    above_600 = zpos[zpos > 600]

    print(f"\n{label}: {len(above_600)} photons with z_mm > 600")

    bins_z = np.linspace(zpos.min(), zpos.max(), 40)

    plt.hist(
        zpos,
        bins=bins_z,
        histtype="step",
        label=label
    )

plt.xlabel("z position (mm)")
plt.ylabel("Count")
plt.title("Z position comparison")
plt.legend()
plt.show()



counts_above_600 = []

for df in data:

    filtered = df[
        (df["volumeName"] == "physPhotonDetectorPosY") &
        (df["boundary"] == "enter")
    ]

    zpos = filtered["z_mm"]

    counts_above_600.append((zpos > 600).sum())

plt.figure()

plt.bar(labels, counts_above_600)

plt.xlabel("Dataset")
plt.ylabel("Photons with z_mm > 600")
plt.title("Number of photons above 600 mm")

plt.show()

