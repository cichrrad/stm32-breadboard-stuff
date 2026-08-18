import cv2
import serial
import time
import numpy as np
import sys

# DONT FORGET TO SAY "THANKS, GEMINI"

# --- CONFIGURATION ---
VIDEO_PATH = "./bad_apple.mp4"
COM_PORT = "/dev/ttyACM0"       # Change to ST-Link VCP port
BAUD_RATE = 1000000             # Max ~1'000'000 for my board, MUST match the baud rate you divide with in usart_dma.c
TARGET_FPS = 24.0

MAGIC_HEADER = bytes([0xBA, 0xDA, 0x55, 0xAA])
FRAME_INTERVAL = 1.0 / TARGET_FPS

# Pre-calculate bitwise powers for fast Numpy packing (LSB is top pixel)
POWERS_OF_2 = np.array([1, 2, 4, 8, 16, 32, 64, 128], dtype=np.uint8).reshape(8, 1)

def main():
    try:
        ser = serial.Serial(COM_PORT, BAUD_RATE, write_timeout=0)
    except Exception as e:
        print(f"Error opening {COM_PORT}: {e}")
        sys.exit(1)

    cap = cv2.VideoCapture(VIDEO_PATH)
    if not cap.isOpened():
        print(f"Error: Could not open {VIDEO_PATH}")
        sys.exit(1)

    print(f"Streaming at {TARGET_FPS} FPS over {COM_PORT} at {BAUD_RATE} baud...")
    
    seq = 0
    frames_sent = 0
    start_time = time.perf_counter()

    while True:
        ret, frame = cap.read()
        if not ret:
            # Loop the video
            cap.set(cv2.CAP_PROP_POS_FRAMES, 0)
            continue

        # 1. Image Processing: Resize to 128x64 and convert to grayscale
        gray = cv2.cvtColor(cv2.resize(frame, (128, 64)), cv2.COLOR_BGR2GRAY)
        
        # 2. Binarize (Threshold at 127) - Resulting array contains 0s and 1s
        _, bw = cv2.threshold(gray, 127, 1, cv2.THRESH_BINARY)
        
        # 3. Pack into OLED vertical addressing format using Numpy
        # Reshape to (8 pages, 8 rows/page, 128 cols)
        reshaped = bw.reshape(8, 8, 128)
        
        # Multiply by powers of 2 and sum along the column to compress 8 rows into 1 byte
        packed = (reshaped * POWERS_OF_2).sum(axis=1).astype(np.uint8)
        payload = packed.tobytes()
        
        # 4. Checksum calculation (Fast XOR using numpy)
        checksum = np.bitwise_xor.reduce(packed.ravel())
        
        # 5. Assemble Protocol Frame
        packet = bytearray(MAGIC_HEADER)
        packet.append(seq)
        packet.extend(payload)
        packet.append(checksum)
        
        # 6. Blast to COM port
        ser.write(packet)
        seq = (seq + 1) % 256
        
        # 7. Precise pacing algorithm
        frames_sent += 1
        target_time = start_time + (frames_sent * FRAME_INTERVAL)
        
        # Release CPU briefly if we are far ahead
        while (time.perf_counter() < target_time - 0.002):
            time.sleep(0.001) 
            
        # Spin-lock for the last ~2ms for absolute microsecond precision
        while time.perf_counter() < target_time:
            pass 

if __name__ == "__main__":
    main()