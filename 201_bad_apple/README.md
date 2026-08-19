# PC-STM32-OLED Streamer

This program streams `.mp4` file from your computer to the board via USART2, and from there the board sends the frames to a 128x64 monochrome OLED display via SPI.

## Architecture & Pipeline

The system is designed as a non-blocking, interrupt-driven pipeline where the CPU acts strictly as a high-level overseer.

### PC-side transcoder (Python)

Code itself (`streamer.py`) is mostly written by Gemini (Say "Thank you, Gemini"). What is important is that is scales the video down and matches vertical addressing format the OLED display will draw the incoming data in. A byte encodes vertical column of 8 pixels, and the display renders incoming data in horizontal pages (128x8) -> Frame must be presented as sequence of 8 *banners* which make the picture. 

Because `128 * 64 = 8192 bits = 1024 bytes`, we are sending a `1030 bytes` sized packets to accomodate some metadata. Exact packet layout is

```
[4 bytes Magic Header][1 byte seq number][1024 bytes payload][1 byte checksum]
```

where:

1. Magic Header is `0xBA 0xDA 0x55 0xAA` and signals start of packet.
2. Sequence number is just index of the packet (0-255 with `%`) in the sequence to easier determine, if we dropped some packets or not.
3. Payload (duh)
4. Checksum to confirm noise/corruption did not mess up our packet. It is calculated by `XOR` reduction over the Payload bytes. If some Payload byte gets corrupted, MCU will end up with different checksum, disregarding the frame.

This packet is then send over `/dev/ttyACM0`, targeting `1 000 000` baud, which should be enough to handle `60.0` FPS, though default is movie `24.0` FPS

### MCU-side data ingress

To receive USART2 stream, DMA (`DMA1_Channel2`) is configured and used so that CPU does not have to do the grunt work (of receiving the bytes and moving them to circular buffer) itself, only polling to see if there is new frame and processing it if that is the case.

DMA is setup with DMAMUX to intercept RX signal from USART2 (when byte flies in over the wire) and move the byte to RAM circular ring buffer `rx_buffer`, sized at `8240 bytes`. This means we can store up to 8 packets (frames) for processing before we start writing over them.

CPU only polls to see if there is at least 1 frame worth of data in the buffer via `Stream_ExtractFrame` function in the main loop. If that is the case and payload is not corrupted, it tries to send it via SPI (see below).

### MCU-side data Egress

To sent frames to the OLED display, another DMA (`DMA1_Channel1`) is configured to shove byte sized payload pieces to the SPI1 data register (DR). It is configured so that jumps in byte sized increments 1024 bytes from given pointer, then produces interrupt (handled in `DMA1_Channel1_IRQHandler`) to signal transfer complete. 

To know when to pipe in next byte to the SPI1 DR register (previous one must be sent first...duh), DMA is setup with DMAMUX to intercept SPI1 TX signal generated to request new byte.

To make sure we dont try to send another frame, while mid-transfer of the previous one, we track a `static volatile` flag, which is queried by the CPU in the main loop via `OLED_IsTransferBusy` function. This is also reason for the interrupt, as it fires upon completion and inside its handler, we reset this flag, among other things, such as de-selecting the display by pulling up its CS wire.

## Core Embedded Concepts

This project abandons beginner concepts (like blocking `while` loops) in favor of professional paradigms:

* **Deterministic Timing:** Replacing CPU-halting `__NOP()` loops with a 1ms SysTick interrupt for asynchronous state management.
* **CPU as Orchestrator:** Utilizing the STM32G4's DMAMUX to handle raw data movement entirely in hardware, keeping CPU usage near zero.
* **Double Buffering:** Allocating active RX and TX frame arrays in RAM and swapping pointers to completely eliminate screen tearing.
* **Fault Tolerance:** Implementing packet structure and checksums to ensure that a dropped serial byte doesn't permanently corrupt the display alignment.

## Getting Started

**Hardware Pinout:**

* **PA2 / PA3:** USART2 TX/RX (Routed automatically via ST-Link USB)
* **PA5 / PA7:** SPI1 SCK / MOSI (AF5)
* **PB6 / PA9 / PC7:** OLED CS / DC / RES

**Running the Project:**

1. Flash the compiled `.elf` to the STM32 using the provided custom Makefile and OpenOCD.
2. Install Python dependencies (inside `venv` or not, I am not your mom): `pip install opencv-python-headless pyserial numpy`.
3. Run `python3 streamer.py` to begin streaming the video.

---

## Notes

- Make sure baud rates match between the python script and MCU-side. You can set it at the top of the script and `usart_dma.c`. By default they match, targetting `1 000 000`.