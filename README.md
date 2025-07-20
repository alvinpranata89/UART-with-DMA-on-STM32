# UART-with-DMA-on-STM32
Demonstrating the UART communication with DMA
---
This project demonstrates how I implement UART (Universal Asynchronous Receiver-Transmitter) communication using DMA (Direct Memory Access) on an STM32F401 Nucleo-64 board. The application is written in C and programmed using bare-metal register-level access, without relying on HAL (Hardware Abstraction Layer) libraries.

The primary function of this code is to create a simple echo system. The microcontroller continuously listens for incoming data on UART2. When a message is received, it uses DMA to transmit the same message back to the sender, confirming its reception.

- Hardware Used: STM32F401 Nucleo-64 Board
- IDE: STM32CubeIDE
- Flashing Utility: STM32CubeProgrammer or ST-Link utility to load the compiled binary onto the Nucleo board.
- Serial Terminal: A program like PuTTY, Tera Term, or Minicom to interact with the board.
- Baud Rate: 115200
- Data Bits: 8
- Parity: None
- Stop Bits: 1

How It Works
---
The application leverages the DMA controller to handle data transfers between memory and the UART peripheral, which significantly reduces CPU overhead compared to interrupt-driven or polling-based methods.

1. Initialization:
    - main.c starts by initializing the necessary peripherals.
    - uart2_rxtx_init() configures the GPIO pins (PA2 for TX, PA3 for RX) to their alternate function for UART2 and enables the UART2 peripheral. It also configures the UART for both transmitting and receiving and enables the DMA requests for both.
    - dma1_init() enables the clock for the DMA1 controller.

2. DMA Configuration:
    - UART RX (DMA1 Stream 5): dma1_stream5_uart_rx_config() sets up DMA1 Stream 5 to handle incoming UART data.
    - The peripheral address is set to the UART2 Data Register (USART2->DR).
    - The memory address is set to a buffer (uart_data_buffer).
    - The transfer is configured in circular mode, allowing the DMA to automatically restart the transfer from the beginning of the buffer once it's full. This is ideal for continuous data reception.
    - An interrupt is enabled to fire upon transfer completion.
    - UART TX (DMA1 Stream 6): dma1_stream6_uart_tx_config() sets up DMA1 Stream 6 for outgoing data.
    - The peripheral address is the UART2 Data Register.
    - The memory address points to the message buffer (msg_buff).
    - The direction is set from memory to peripheral.
    - An interrupt is enabled for transfer completion.

3. Execution Flow:
    - After initialization, the program sends a "Initialization complete.." message via DMA to the serial terminal. It waits for the dma_tx_cmplt flag to be set by the DMA TX interrupt handler, ensuring the message is fully sent.
    - The program then enters an infinite loop (while(1)).
    - Inside the loop, it continuously checks the dma_rx_cmplt flag. This flag is set to 1 by the DMA1_Stream5_IRQHandler when the pre-defined number of bytes (UART_DATA_BUFF_SIZE) has been received into uart_data_buffer.
    - Once a message is received, the program formats a new message string ("Message received : [your_message]") and triggers a new DMA transmission to echo it back.
    - It then waits for the echo transmission to complete before listening for the next message.

How to Use
--
1. Clone the Repository:

     git clone <your-repo-url>

2. Build the Project:
  Navigate to the project directory and use your toolchain's command to compile the source files and link them into a final .bin or .elf file.

3. Flash the Board:
  Use STM32CubeProgrammer or another flashing tool to upload the compiled binary to your Nucleo-F401RE board.

4. Connect Serial Terminal:
  Open your preferred serial terminal program.

5. Find the COM port associated with the ST-Link Virtual COM Port.
  Configure the connection with a baud rate of 115200.

Test the Application:
--
Once you reset the board, you should see the message:

    Initialization complete..

Type a message of up to 5 characters (as defined by UART_DATA_BUFF_SIZE) and press Enter.

The board will echo the message back to you. For example, if you send hello, you will see:

    Message received : hello
