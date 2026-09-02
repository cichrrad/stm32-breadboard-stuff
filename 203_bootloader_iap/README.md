# Single-Bank Bootloader

This program serves to employ a basic in-app programming single-bank bootloader. It uses oled display (over SPI) to render UI to the user and expects a USER button press within a grace period after power-on. If button was pressed, user can flash `.bin` (at most `32kb`) of an app over USART2. Bootloader then writes this app to flash, and passes control to the app, *booting* into it. If the grace period was ignored and no app is to be found (from flash done previously), then it enters a `while(1)` loop, informing the user to reset and flash app.

I chose this because I got curious about how updates to low level/smart home stuff are done remotely. While a major simplification, this project conceptually follows very similar process to over-the-air updates (although here, it is over-the-wire). It also acts similar to what the MCU actually does on power on, as it has its own bootloader (so this is actually app being launched by a bootloader which was launched by a much better MCU bootloader). In production, this would of course be much more involved, mainly it would most certainly be Double-Bank Bootloader (A/B partitions to always have fallback version), and after getting the binary for the app, actual checksum and confirmation process would occur to make sure we are booting something unaltered.

## DEMO

### [Bootloader demos in one `.mp4`](./assets/bootloader_demos.mp4)

### Pictures

- ![demo_grace_period](./assets/demo_grace_period.jpg)

- ![demo_no_app_found](./assets/demo_no_app_found.jpg)

- ![demo_waiting_for_bin](./assets/demo_waiting_for_bin.jpg)

- ![demo_bin_transfer_complete](./assets/demo_bin_transfer_complete.jpg)

- ![demo_flash_done](./assets/demo_flash_done.jpg)

- ![demo_in_app1](./assets/demo_in_app1.jpg)

- ![demo_in_app3](./assets/demo_in_app3.jpg)

---

## Overview & Architecture

- There are 2 main parts - `bootloader` dir with the main program, and `update_app` dir with 3 mock apps to flash onto the MCU.

### Mock apps

- Mock apps simply write a specific message to display so it can be verified specific app was flashed. To build them, use `Makefile` macros `make build-x` where `x = 1/2/3`. This produces their `.elf` and `.bin` files. 

- You can flash them directly via `make flash-x`, but that is expected to not work, because `update_app/linker.ld` places them at an offset at `0x08008000`, leaving `32kb` for the bootloader. Since it starts at different address than what the MCU bootloader looks at (`0x08000000`), it wont run.

- Another important step done in the `Reset_Handler` is to explicitly reconfigure `SCB` (System Control Block) of the MCU to look for the vector table at an offset, because our app is at an offset itself:
```c
    SCB->VTOR = 0x08008000;
```
If we dont do this, MCU would look at the table for the stack pointer and jump back guided by the *default* (at no offset) bootloaders vector table.

- Last crucial thing is that first thing in `main` we run `__enable_irq`, to enable interrupts, assuming they were disabled for the handing of the control from the bootloader. Enabling them at the start of any program is (always?) good, because worst case scenario its a no-op, and I'd assume majority of apps wont break and even need interrupts to be enabled to work...duh.


### Bootloader 

- Main part of this project is the bootloader itself. It is set up like any other program, at the default address `0x08000000` so its the first thing to run.

- Conceptually, it is very simple - it just displays UI to relay that it is waiting for a grace period for the user to press USER button to initiate process of flashing new app binary, which it will write into flash memory at `0x08008000`, and then give it control by running its `Reset_Handler` function. We know where it is, given that we know where the app memory starts and we know that first 4 bytes are the stack pointer, and the next 4 bytes are the `Reset_handler` itself (this is setup in `startup.c` by having this order when we put it in the `.isr_vector` section).

- To detect the button press, we poll for input data register of the button pin in while loop within the grace period (see `local_source/button.c/h`). Polling happens every ~1ms or so (guided by SysTick) to not fully spin up for no reason.

- **If button was not pressed**, then the bootloader assumes some app is flashed already and tries to jump to it. To have some sanity check to protect before just blindly jumping around in memory, a check is in place to at least confirm stack pointer at `0x08008000` is within RAM memory space. This could **certainly** be improved, as it is really naive fragile approach, for rather obvious reasons. A proper way would entail having a non-volatile persistant flag signaling app was flashed successfully and is present. This flag would be set after flashing the app binary + checking checksum in case the wire gets iffy or the binary was tampered with. If this check fails, then the bootloader falls into infinite loop, informing the user that no default app was found and that they should reset and flash something.

- **If button was pressed**, then `DMA1_channel2` is enabled to listen on `USART2` for the binary coming over the wire (see `local_source/usart.c/h`). DMA is configured to move incoming bytes from the `USART2` RX register to a `32kb` buffer `rx_buffer`. As binaries can have various sizes (for instance, mock apps in `update_app` have `~3,7kb` when compiled with `-Os`), we need to be able to know when transmition ends to stop. This is done via `USART2` `IDLE` flag, which is set if nothing came over the wire for a full byte timeframe (that is actually 10 bits - `[0]` to signal byte is coming,`[byte]`, and `[1]` to signal byte frame end. Silence would look like `[1][11111111][1]`). If no data are coming in, then this flag gets set and we can use it to determine end of transmition.

- **Once binary is recieved in the buffer**, it needs to be flashed, as in written into flash memory, aligned with the starting address of the app (`0x08008000`).This is done in `Flash_Write_App` (see `local_source/flash.c/h`).

- This is actually kind of scary, as this memory is non-volatile and wears easily (it is rated to `~10 000` erase/write cycles from what I read), meaning one rogue pointer / bug in a while loop could result in degrading it in less than a minute. Because of this, it is *locked* by default and must explicitly be *unlocked* to be able to write into it.

- We unlock it by writing 2 specific keys in specific order into `FLASH->KEYR` registers. Once unlocked, we need to zero out memory for our new binary, and we can only do so by wiping whole pages (`2kb`). We calculate how many pages to erase based on the size of the binary, then do so in a loop, selecting each and setting erase bit to signal it for wipe. 

- As another layer of protection (I suppose), we must write to the memory only with 2 32-bit words (sequence of two 32-bit word writes).

- After we are done flashing, we lock the memory again and try to jump to our newly flashed app.

---

## TODOs

- Add some form of checksum to confirm binary was send well over the wire
- Add non-volatile flag to denote the flash memory is populated with app from previous flash (now it works as long as we dont overrite memory at that 32k offset reserved for Bootloader itslef, but it is very much risky to just jump there as long as stack pointer seems to point to sensible location)
- add chunking loop so that binary bigger than the buffer can be uploaded (instead of 32kb buffer for binary, lets have 2x16kb buffers or something, and once one is filled, swap to empty one and write the full one to memory in the meantime?)
- There are things that could be more robust. For instance I think that in the flashing process, we could and maybe should include some synchronization to make sure the 2 sequential writes really happen in order (I think this would heavily depend on optimizations). Another thing is that we should pad the round-up with `0xFFFFFFFF` or something, as if we round up for binary which will sit at the end of memory next to restricted region, we will go out of bounds possibly.