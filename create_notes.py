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

# Note frequencies (C4 to C5)
NOTES = {
    'C4': 261.63, 'D4': 293.66, 'E4': 329.63, 'F4': 349.23,
    'G4': 392.00, 'A4': 440.00, 'B4': 493.88, 'C5': 523.25
}

def generate_sine_wave(frequency, duration, harmonics=None):
    t = np.linspace(0, duration, int(SAMPLE_RATE * duration), False)
    tone = np.zeros_like(t)
    
    if harmonics is None:
        harmonics = [(1.0, 1)]  # Default: just fundamental frequency
        
    for amplitude_mult, freq_mult in harmonics:
        tone += amplitude_mult * np.sin(2 * np.pi * frequency * freq_mult * t)
    
    return tone

def apply_envelope(tone, attack=0.1, decay=0.1, sustain=0.7, release=0.1):
    total_length = len(tone)
    attack_length = int(attack * total_length)
    decay_length = int(decay * total_length)
    release_length = int(release * total_length)
    sustain_length = total_length - attack_length - decay_length - release_length
    
    envelope = np.concatenate([
        np.linspace(0, 1, attack_length),
        np.linspace(1, sustain, decay_length),
        np.ones(sustain_length) * sustain,
        np.linspace(sustain, 0, release_length)
    ])
    
    return tone * envelope

def create_piano_sound(frequency, duration):
    # Piano-like harmonics
    harmonics = [
        (1.0, 1),    # fundamental
        (0.6, 2),    # 1st harmonic
        (0.4, 3),    # 2nd harmonic
        (0.2, 4),    # 3rd harmonic
    ]
    tone = generate_sine_wave(frequency, duration, harmonics)
    tone = apply_envelope(tone, attack=0.02, decay=0.1, sustain=0.3, release=0.3)
    return tone

def create_synth_sound(frequency, duration):
    # Synth-like harmonics with detuning
    harmonics = [
        (1.0, 1),        # fundamental
        (0.5, 1.01),     # slightly detuned fundamental
        (0.5, 0.99),     # slightly detuned fundamental
        (0.3, 2),        # 1st harmonic
    ]
    tone = generate_sine_wave(frequency, duration, harmonics)
    tone = apply_envelope(tone, attack=0.1, decay=0.1, sustain=0.6, release=0.2)
    return tone

def create_bell_sound(frequency, duration):
    # Bell-like harmonics
    harmonics = [
        (1.0, 1),    # fundamental
        (0.7, 2.4),  # minor third
        (0.5, 3),    # fifth
        (0.3, 4.7),  # seventh
    ]
    tone = generate_sine_wave(frequency, duration, harmonics)
    tone = apply_envelope(tone, attack=0.01, decay=0.1, sustain=0.2, release=0.5)
    return tone

def save_wave_file(filename, audio_data):
    # Normalize and convert to 16-bit integers
    audio_data = np.int16(audio_data * AMPLITUDE)
    
    with wave.open(filename, 'w') as wav_file:
        wav_file.setnchannels(1)  # Mono
        wav_file.setsampwidth(2)  # 2 bytes per sample
        wav_file.setframerate(SAMPLE_RATE)
        for sample in audio_data:
            wav_file.writeframes(struct.pack('h', sample))

def main():
    instruments = {
        'piano': create_piano_sound,
        'synth': create_synth_sound,
        'bell': create_bell_sound
    }
    
    for instrument in instruments:
        instrument_dir = os.path.join('sounds', instrument)
        if not os.path.exists(instrument_dir):
            os.makedirs(instrument_dir)
            
        print(f"\nGenerating {instrument} sounds...")
        for note_name, frequency in NOTES.items():
            print(f"  {note_name} ({frequency} Hz)")
            audio_data = instruments[instrument](frequency, DURATION)
            filename = os.path.join(instrument_dir, f'{note_name}.wav')
            save_wave_file(filename, audio_data)
    
    print("\nDone! All instrument sounds have been generated in the 'sounds' directory.")

if __name__ == "__main__":
    main() 