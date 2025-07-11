# ESP32 Multi-Device Control System

This project implements a **multifunctional control and monitoring system** using an **ESP32**, providing an interactive serial interface to operate LEDs, a servo motor, a buzzer, and to read sensor data. The system also features a real-time clock (RTC) to timestamp events and manage schedules.

---
## High level Circuit
![image](https://github.com/user-attachments/assets/0e728b7d-38e8-4b7c-b26d-4917cefc86a7)

![image](https://github.com/user-attachments/assets/0ac52d4d-cc9b-423d-8a3f-1f74e0f3ab4a)



## 🚀 Features

✅ **PWM LED Control (GPIO23)**  
- **Automatic mode**: LED brightness controlled by a potentiometer (GPIO34).  
- **Manual mode**: User-defined PWM value (0–255).  

✅ **Servo Motor Control (GPIO19)**  
- Adjustable position between **0–180 degrees** via commands.

✅ **Additional LEDs and Buzzer**  
- **LED2 (GPIO5)** and **LED3 (GPIO12)** can be turned on/off.  
- **Buzzer (GPIO27)** can be activated for alerts.

✅ **Push Button Input (GPIO18)**  
- Interrupt-based detection of button presses.
- Keeps count and timestamps of each event.

✅ **Real-Time Clock (RTC)**  
- Allows date and time configuration.
- Timestamps events such as button presses.

✅ **Dual Serial Interfaces**
- **Serial Monitor (USB)** and **Hardware Serial** (`RX=16, TX=17`) for command input and logs.

---

## 🛠️ Hardware Components

- **ESP32** Development Board
- **LEDs**
  - LED1 (PWM): GPIO23
  - LED2: GPIO5
  - LED3: GPIO12
- **Potentiometer** (Analog input on GPIO34)
- **Servo Motor** (PWM control on GPIO19)
- **Buzzer** (GPIO27)
- **Push Button** (GPIO18 with internal pull-up)

---

## 📖 Available Commands

| Command                | Description                                      |
|------------------------|--------------------------------------------------|
| `status`               | Display the current status of all devices       |
| `led1`                 | Show current PWM value applied to LED1          |
| `led1manual`           | Set manual PWM value for LED1 (0–255)           |
| `auto`                 | Return LED1 control to potentiometer            |
| `pot`                  | Read potentiometer value                        |
| `servo`                | Show current servo position (degrees)           |
| `servo <angle>`        | Move servo to specified angle (0–180)            |
| `led2`                 | Display LED2 state                              |
| `led2 on` / `led2 off` | Turn LED2 on/off (with buzzer alert when ON)    |
| `led3`                 | Display LED3 state                              |
| `led3 on` / `led3 off` | Turn LED3 on/off (with buzzer alert when ON)    |
| `buzzer`               | Activate buzzer for 3 seconds                   |
| `botao`                | Show button press count and last timestamp      |
| `sethora`              | Configure date and time (format: DD/MM/YYYY HH:MM:SS) |
| `help`                 | Display this list of commands                   |

---

## 🖥️ Serial Interfaces

- **USB Serial Monitor:** Logs and command input.
- **HardwareSerial (UART2):**
  - **RX:** GPIO16
  - **TX:** GPIO17
  - Baudrate: `115200`

---

## 🧭 How to Use

1. Flash the firmware to your ESP32.
2. Open the Serial Monitor at `115200 baud`.
3. Type commands and press **Enter**.
4. For the HardwareSerial port, connect an external serial device on GPIO16 (RX) and GPIO17 (TX).
5. Use `help` to display the list of commands.

---

## 📌 Example Usage

- **Check potentiometer value:**
pot

- **Set LED1 brightness manually to 128:**
led1manual

*(then enter `128` when prompted)*

- **Move servo to 90 degrees:**
servo 90


- **Activate the buzzer:**
buzzer


- **Set current date and time:**
sethora 05/07/2025 15:30:00


---

## 📂 Project Structure

- **setup()**
- Initializes pins, PWM channels, interrupts, and serial ports.
- **loop()**
- Reads inputs, processes commands, updates outputs, and handles button events.

---

## 💡 Notes

- All PWM outputs use the `ledc` API.
- The button interrupt includes software debounce.
- Time and date are retained in the RTC as long as the ESP32 remains powered.


---

<img width="538" height="403" alt="image" src="https://github.com/user-attachments/assets/3bf24a5c-10ef-40d4-a196-1402a609c611" />

