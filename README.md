# BLE Mesh LED/Button Control Example

This project demonstrates a custom vendor model implementation for LED control using Bluetooth Mesh with nRF52840 Development Kits. It consists of two applications:
- Light Server: Controls LEDs based on received commands
- Button Client: Sends LED control commands based on button presses

## Requirements

- 3x nRF52840 Development Kits
- nRF Connect SDK v2.8.0
- nRF Mesh mobile app (for provisioning)
- Visual Studio Code with nRF Connect extension (recommended)

## Project Structure

```
mesh/
├── light_server/       # LED controller application
│   ├── src/
│   ├── include/
│   ├── CMakeLists.txt
│   └── prj.conf
├── button_client/      # Button controller application
│   ├── src/
│   ├── include/
│   ├── CMakeLists.txt
│   └── prj.conf
└── README.md
```

## Building

### Light Server (LED Controller)

```bash
cd light_server
west build -b nrf52840dk/nrf52840 -p always
west flash
```

### Button Client (Button Controller)

```bash
cd button_client
west build -b nrf52840dk/nrf52840 -p always
west flash
```

## Setup Instructions

1. Flash two boards with the Light Server firmware
2. Flash one board with the Button Client firmware
3. Use nRF Mesh mobile app to provision all three boards:
   - Add them to the same network
   - Configure publish/subscribe addresses
   - Set up group addresses for LED control

## Usage

### Button Client Board
- Press Button 1-4: Toggles corresponding LED on server boards
- LEDs 1-4: Mirror the state of server LEDs

### Light Server Boards
- LEDs 1-4: Controlled by button presses from client
- Automatically sends status updates back to client

## Vendor Model Details

- Company ID: 0x0059 (Nordic Semiconductor)
- Model IDs:
  - Server: 0x0000
  - Client: 0x0001
- Operations:
  - LED Set (0x00)
  - LED Get (0x01)
  - LED Status (0x02)
  - Button Press (0x03)

## Debugging

- Enable serial logging to monitor mesh operations
- Use RTT Viewer for real-time logging
- Check provisioning status through LED patterns
- Monitor button press and LED state changes

## Troubleshooting

1. Provisioning Issues:
   - Ensure all devices are powered and in range
   - Check if devices are already provisioned
   - Reset devices if necessary

2. Communication Issues:
   - Verify publish/subscribe addresses
   - Check if devices are in the same network
   - Ensure proper key distribution

3. LED Control Issues:
   - Verify LED initialization
   - Check message routing
   - Monitor status responses

## Power Management

- Devices operate in normal power mode
- No specific power optimization implemented
- Consider implementing LPN mode for battery-powered nodes

## Security Considerations

- Basic mesh security through provisioning
- Network and application keys provide basic security
- No additional security layers implemented
