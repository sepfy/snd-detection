import librosa
import librosa.display
import matplotlib.pyplot as plt
from sklearn import svm
import numpy as np
import sys
import scipy.misc
import os

def get_melspec(f, label, source):

  files = [os.path.join(source,f) for f in os.listdir(source)]
  for fname in files[:10]:
    print(fname)
    samples, sample_rate = librosa.load(fname)
    samples = samples[:65250]
    S = librosa.feature.melspectrogram(y=samples, sr=sample_rate, n_mels=128)
    specstrum = S
    #specstrum = librosa.amplitude_to_db(S, ref=np.max)
    specstrum.resize(128*128)
    #batch = np.concatenate((batch, specstrum), 0)
    #for
    #f.write(
    f.write(str(label)+" ")
    for i in range(1, 128*128+1):
      f.write(str(i) + ":" + str(round(specstrum[i-1],4)) + " ")
      #print(specstrum.shape)
    f.write("\n")


f = open("train.csv", "w")
source   = "train/cry/"
get_melspec(f, 1, source)

source   = "train/none/"
get_melspec(f, 2, source)


#y = []
#label = 1.0
#for i in range(n):
#  y.append(label)
'''
label = 0.0
source   = "train/cry/"
batch, n = get_melspec(batch, source)
for i in range(n):
  y.append(label)



print(batch.shape)
print(y)
clf = svm.SVC()
clf.fit(batch, y)

# Validation
batch = np.empty((0, 128*128), dtype=np.float32)
y = []
label = 0.0
source   = "test/cry/"
batch, n = get_melspec(batch, source)
for i in range(n):
  y.append(label)

label += 1.0
source   = "test/none/"
batch, n = get_melspec(batch, source)
for i in range(n):
  y.append(label)


print((clf.predict(batch) == y).sum()/batch.shape[0])
'''
#print(files)

'''
fname = sys.argv[1]
print(samples.shape, sample_rate)
#samples = samples[:100250]
fig = plt.figure(figsize=[4,4])
ax = fig.add_subplot(111)
ax.axes.get_xaxis().set_visible(False)
ax.axes.get_yaxis().set_visible(False)
ax.set_frame_on(False)
import time

#start = time.time()
#mfcc = librosa.feature.mfcc(y=samples, sr=sample_rate)
#print(time.time() - start)
#print(mfcc.shape)
start = time.time()
S = librosa.feature.melspectrogram(y=samples, sr=sample_rate, n_mels=128)
S = librosa.feature.melspectrogram(y=samples, sr=sample_rate, n_mels=10)
amplitude_spec = librosa.amplitude_to_db(S, ref=np.max)
print(time.time() - start)
print(amplitude_spec.shape)


print(amplitude_spec.shape)
from PIL import Image
from skimage import img_as_ubyte
m = np.absolute(amplitude_spec).max()
amplitude_spec = amplitude_spec/m
#print(amplitude_spec)
#mean = np.mean(amplitude_spec)
#print(mean)
#std = np.std(amplitude_spec)
#amplitude_spec = (amplitude_spec - mean)/std
#print(amplitude_spec)
amplitude_spec = img_as_ubyte(amplitude_spec)
print(amplitude_spec)
im = Image.fromarray(amplitude_spec)
im.save("your_file.png")
#np.save("1.npy", amplitude_spec)
#o = np.load("1.npy")
#scipy.misc.toimage(amplitude_spec).save("test.png")
#librosa.display.specshow(o)
#plt.show()

'''
