# 📦 Automated Packing System

A finite state machine (FSM) controlled automated packing system built on the 
KL46Z microcontroller in C++ using Keil Studio Cloud. The system fills a 
user-configured number of boxes with a user-configured number of pieces via 
an 8-bit DIP switch input.

## Features
- FSM-based control with 4 defined states
- 8-bit DIP switch input (two 4-bit fields for box count and piece count)
- Conveyor and nozzle safety interlock — never operate simultaneously
- Global emergency stop returning system to idle instantly
- LED status indicators for each subsystem (active/inactive)
- Configurable up to 15 boxes × 15 pieces

## Hardware
| Component | Details |
|---|---|
| Microcontroller | NXP FRDM-KL46Z |
| Input | 8-bit DIP switch (D4–D11) |
| Start Button | SW1 (PTC12) — internal pull-up |
| Stop / E-Stop | SW3 (PTC3) — internal pull-up |
| Conveyor LEDs | Green (D0), Red (D1) — 560Ω series resistors |
| Nozzle LEDs | Green (D2), Red (D3) — 560Ω series resistors |

## FSM States
| State | Description |
|---|---|
| IDLE | Both LEDs red, awaiting start input |
| MOVE_BOX | Conveyor active for 5 seconds, nozzle closed |
| FILL_BOX | Conveyor stopped, nozzle dispenses 1 piece/second |
| DONE | All boxes filled, both LEDs red |

## How It Works
1. User sets box count (N) and piece count (M) on the DIP switch
2. START button triggers FSM out of idle
3. Conveyor moves each box into position (5 seconds)
4. Nozzle dispenses M pieces at 1 second per piece
5. Loop repeats N times, then system returns to idle
6. STOP button acts as global emergency stop at any point

## Safety
- Conveyor and nozzle are never active simultaneously
- Emergency stop halts all activity and returns to idle instantly
- All outputs operate within KL46Z pin limits (2.5mA per LED, 10mA total)

## Future Improvements
- LCD display for real-time box and piece count feedback
- Sensor feedback for physical box detection
- Serial monitor logging for diagnostics
