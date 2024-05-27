import h5py
import numpy as np
import matplotlib.pyplot as plt

# Load data from HDF5 file
with h5py.File('data.h5', 'r') as f:
    data = f['/data'][:]

# Reshape data into array of samples x channels
data = data.reshape(-1, 16)

# Plot each channel in the time domain
fig, ax = plt.subplots(8, 2, sharex=True, sharey=True, figsize=(10, 10))
colors = ['b', 'g', 'r', 'c', 'm', 'y', 'k', 'orange', 'purple', 'brown', 'pink', 'gray', 'olive', 'teal', 'navy', 'salmon']
for i in range(16):
    row = i // 2
    col = i % 2
    ax[row, col].plot(data[:, i], color=colors[i])
    ax[row, col].set_title(f'Channel {i}')
plt.suptitle('Time Domain')
plt.tight_layout()
plt.show()

# Plot each channel in the frequency domain
fft_data = np.abs(np.fft.fft(data, axis=0))
freq = np.fft.fftfreq(data.shape[0], d=0.01)
fig, ax = plt.subplots(8, 2, sharex=True, sharey=True, figsize=(10, 10))
for i in range(16):
    row = i // 2
    col = i % 2
    ax[row, col].plot(freq[:len(freq)//2], fft_data[:len(freq)//2, i], color=colors[i])
    ax[row, col].set_title(f'Channel {i}')
    ax[row, col].set_xlim(0, 200)
plt.suptitle('Frequency Domain')
plt.tight_layout()
plt.show()
