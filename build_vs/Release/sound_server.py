import sounddevice as sd
import numpy as np
import sys
import time
from threading import Thread, Lock
import json

# Audio parameters
SAMPLE_RATE = 44100
AMPLITUDE = 0.3
ATTACK_TIME = 0.01
RELEASE_TIME = 0.1

# Global state
active_notes = set()
note_lock = Lock()

def generate_sine_wave(frequency, duration):
    t = np.linspace(0, duration, int(SAMPLE_RATE * duration), False)
    tone = AMPLITUDE * np.sin(2 * np.pi * frequency * t)
    
    # Apply simple envelope
    attack_samples = int(ATTACK_TIME * SAMPLE_RATE)
    release_samples = int(RELEASE_TIME * SAMPLE_RATE)
    
    attack = np.linspace(0, 1, attack_samples)
    release = np.linspace(1, 0, release_samples)
    sustain = np.ones(len(tone) - attack_samples - release_samples)
    
    envelope = np.concatenate([attack, sustain, release])
    return tone * envelope

def audio_callback(outdata, frames, time, status):
    if status:
        print(status)
    
    with note_lock:
        if not active_notes:
            outdata.fill(0)
            return
            
        # Mix all active notes
        mixed = np.zeros(frames)
        for freq in active_notes:
            samples = generate_sine_wave(freq, frames/SAMPLE_RATE)
            mixed += samples[:frames]
        
        # Normalize
        if len(active_notes) > 0:
            mixed /= len(active_notes)
        
        outdata[:, 0] = mixed

def start_audio():
    stream = sd.OutputStream(
        channels=1,
        callback=audio_callback,
        samplerate=SAMPLE_RATE
    )
    stream.start()
    return stream

def main():
    print("Starting sound server...")
    stream = start_audio()
    
    try:
        while True:
            try:
                line = sys.stdin.readline().strip()
                if not line:
                    continue
                    
                data = json.loads(line)
                command = data.get('command')
                
                if command == 'play':
                    frequencies = data.get('frequencies', [])
                    with note_lock:
                        active_notes.clear()
                        active_notes.update(frequencies)
                
                elif command == 'stop':
                    with note_lock:
                        active_notes.clear()
                
            except json.JSONDecodeError:
                print("Invalid JSON received")
            except Exception as e:
                print(f"Error: {e}")
                
    except KeyboardInterrupt:
        print("\nStopping sound server...")
    finally:
        stream.stop()
        stream.close()

if __name__ == "__main__":
    main() 