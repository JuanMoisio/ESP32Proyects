# ESP32 RFID Enroll Project

This project allows you to enroll RFID cards using an ESP32 microcontroller. It connects to a Wi-Fi network and serves a web page where you can manage enrolled RFID cards. The project also provides visual feedback through LEDs, indicating whether a card is enabled or disabled.

## Features

- Connects to a specified Wi-Fi network.
- Serves a web page for enrolling RFID cards.
- Displays a list of enrolled RFID numbers.
- Green LED indicates an enabled card.
- Red LED indicates a disabled card.

## Project Structure

```
esp32-rfid-enroll
├── src
│   ├── main.cpp         # Entry point of the application
│   ├── wifi.cpp         # Wi-Fi connection logic
│   ├── webserver.cpp     # Web server functionality
│   ├── rfid.cpp         # RFID reading and enrollment
│   ├── led.cpp          # LED control logic
│   └── cards.cpp        # Management of enrolled RFID cards
├── include
│   ├── wifi.h           # Wi-Fi connectivity declarations
│   ├── webserver.h      # Web server declarations
│   ├── rfid.h           # RFID operations declarations
│   ├── led.h            # LED control declarations
│   └── cards.h          # Enrolled cards management declarations
├── platformio.ini       # PlatformIO configuration file
└── README.md            # Project documentation
```

## Setup Instructions

1. Clone the repository or download the project files.
2. Open the project in your preferred IDE.
3. Configure the `platformio.ini` file with your ESP32 board settings.
4. Update the Wi-Fi credentials in the `wifi.cpp` file.
5. Upload the code to your ESP32 board.
6. Connect to the ESP32's IP address using a web browser to access the enrollment page.

## Usage

- Once the ESP32 is connected to Wi-Fi, navigate to the provided IP address in your web browser.
- Use the web interface to enroll new RFID cards.
- The green LED will light up for enabled cards, while the red LED will indicate disabled cards.

## License

This project is open-source and available for modification and distribution under the MIT License.