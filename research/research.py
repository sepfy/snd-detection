import librosa
import librosa.display
import matplotlib.pyplot as plt
import numpy as np
import sys
fname = sys.argv[1]
samples, sample_rate = librosa.load(fname)
print(samples.shape, sample_rate)
fig = plt.figure(figsize=[4,4])
ax = fig.add_subplot(111)
ax.axes.get_xaxis().set_visible(False)
ax.axes.get_yaxis().set_visible(False)
ax.set_frame_on(False)
S = librosa.feature.melspectrogram(y=samples, sr=sample_rate)
print(librosa.amplitude_to_db(S, ref=np.max).shape)
librosa.display.specshow(librosa.amplitude_to_db(S, ref=np.max))
plt.show()
