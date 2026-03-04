
In Coollab you can make your effects react to audio!

![audio config window](./img/Audio/audio_config_window.png)

In order to configure the audio, you need to open the <span class="icon-music"></span> *Audio* config window, either from the <span class="icon-rocket"></span> Commands palette:

![open audio config via command](./img/Audio/open_audio_config_command.png)

or from the <span class="icon-cog"></span> cog button on audio nodes:

![open audio config via button](./img/Audio/open_audio_config_button.png)

## Audio inputs

You can receive your audio either from a <span class="icon-file-music"></span> `File` or from a <span class="icon-mic"></span> `Device` (microphone / audio stream).

![input selection](img/Audio/input_selection.png)

### File

You can select any audio file from your computer (.mp3, .wav, *etc.*).

![file selection](img/Audio/file_selection.png)

You then have a few options:
- <span class="icon-loop"></span> You can make the audio loop
- <span class="icon-volume-medium"></span> You can mute the audio. This will not affect your audio-reactive nodes, it will just mean that the audio isn't played by Coollab. But your nodes will still consider that the audio is playing at full volume.
- You can control the *Playback Volume* of the audio. This will not affect your audio-reactive nodes, it will just affect the volume played by Coollab. But your nodes will still consider that the audio is playing at full volume.

### Device

![device selection](img/Audio/device_selection.png)

You can either select *Use default device: xxx* and Coollab will automatically use your default device (e.g. the integrated microphone of your laptop, or the microphone that you have plugged in), and automatically switch when the default device changes (e.g. when you plug / unplug a microphone).

Or you can select a specific device, and Coollab will keep using this one, and give you an error if it becomes unavailable (e.g. your microphone gets unplugged).

You can then tweak the *Volume Amplifier* to make sure that the audio is well calibrated and is neither too low nor too loud.

## Audio features

You can configure each audio feature on the <span class="icon-music"></span> *Audio* config window, and then receive their values from audio nodes in your graph.

### Volume

![feature volume](img/Audio/feature_volume.png)

The *Window size* controls over how long of a section of the audio the volume will be averaged. A longer duration will result in smoother variations, but will be slower to compute.

You can access it with the node "*Volume (Audio)*". It is a number that you will typically use to drive some parameters of your nodes:

![node volume](img/Audio/node_volume.png)

### Waveform

![feature waveform](img/Audio/feature_waveform.png)

The waveform is the raw amplitude of the audio signal, plotted over time.

The *Window size* controls how big of a section of the audio will be visualized at once. A longer duration will result in more apparent scrolling, but will better show the shape of the wave.

You can access it with the node "*Waveform (Audio)*". You will typically visualize it as a function (with either "*Fill Function*" or "*Function to Shape*"), or apply it on top of a curve with "*Add Displacement*":

![node waveform](img/Audio/node_waveform.png)

### Spectrum

![feature spectrum](img/Audio/feature_spectrum.png)

The spectrum shows how much low and high pitches there is in your audio at the current time. (Low pitch on the left, high pitch on the right).

The *Window size* controls over how long of a section of the audio the spectrum will be averaged. A longer duration will result in smoother variations, but will be slower to compute.

You can access it with the node "*Spectrum (Audio)*". You will typically visualize it as a function (with either "*Fill Function*" or "*Function to Shape*"), or apply it on top of a curve with "*Add Displacement*":

![node spectrum](img/Audio/node_spectrum.png)