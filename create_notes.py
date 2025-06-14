import numpy as np
import wave
import struct
import os

# Create sounds directory if it doesn't exist
if not os.path.exists('sounds'):
    os.makedirs('sounds')

# Audio parameters
SAMPLE_RATE = 44100
DURATION = 0.5  # seconds
AMPLITUDE = 32767  # Max amplitude for 16-bit audio
FADE_TIME = 0.1  # seconds for fade in/out

# Note frequencies (C4 to C5)
NOTES = {
    'C4': 261.63,
    'D4': 293.66,
    'E4': 329.63,
    'F4': 349.23,
    'G4': 392.00,
    'A4': 440.00,
    'B4': 493.88,
    'C5': 523.25
}

def generate_sine_wave(frequency, duration, sample_rate=SAMPLE_RATE):
    t = np.linspace(0, duration, int(sample_rate * duration), False)
    tone = AMPLITUDE * np.sin(2 * np.pi * frequency * t)
    
    # Create fade in/out
    fade_len = int(FADE_TIME * sample_rate)
    fade_in = np.linspace(0, 1, fade_len)
    fade_out = np.linspace(1, 0, fade_len)
    
    # Apply fade in/out
    tone[:fade_len] *= fade_in
    tone[-fade_len:] *= fade_out
    
    return tone.astype(np.int16)

def save_wave_file(filename, audio_data, sample_rate=SAMPLE_RATE):
    with wave.open(filename, 'w') as wav_file:
        # Set parameters
        nchannels = 1  # Mono
        sampwidth = 2  # 2 bytes per sample (16-bit)
        
        # Set wav parameters
        wav_file.setnchannels(nchannels)
        wav_file.setsampwidth(sampwidth)
        wav_file.setframerate(sample_rate)
        
        # Write data
        for sample in audio_data:
            wav_file.writeframes(struct.pack('h', sample))

def main():
    print("Generating note files...")
    for note_name, frequency in NOTES.items():
        print(f"Generating {note_name} ({frequency} Hz)")
        audio_data = generate_sine_wave(frequency, DURATION)
        filename = os.path.join('sounds', f'{note_name}.wav')
        save_wave_file(filename, audio_data)
    print("Done! All note files have been generated in the 'sounds' directory.")

if __name__ == "__main__":
    main() 