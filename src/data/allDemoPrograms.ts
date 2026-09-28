// Central registry of all Advayam demo programs across all supported frameworks
export type ProgramLanguage = "Arduino" | "ESP-IDF" | "FreeRTOS";

export interface DemoProgramItem {
  id: string;
  title: string;
  language: ProgramLanguage;
  category: string;
  level: "Beginner" | "Intermediate" | "Advanced";
  filename: string;
  codePath: string;
  description: string;
  existingHref?: string;
}

export const ALL_DEMO_PROGRAMS: DemoProgramItem[] = [
  {
    "id": "ard-1",
    "title": "Ambient Lux Data Logger",
    "language": "Arduino",
    "category": "Light & Optical",
    "level": "Beginner",
    "filename": "Ambient Lux Data Logger.ino",
    "codePath": "/advayam-codes/arduino/Ambient%20Lux%20Data%20Logger.ino",
    "description": "Ambient Lux Data Logger demo project running on Advayam ESP32 development hardware using the Arduino framework."
  },
  {
    "id": "ard-2",
    "title": "Angle Based HSV Colour Wheel",
    "language": "Arduino",
    "category": "Sensors & Actuators",
    "level": "Intermediate",
    "filename": "Angle Based HSV Colour Wheel.ino",
    "codePath": "/advayam-codes/arduino/Angle%20Based%20HSV%20Colour%20Wheel.ino",
    "description": "This program maps the board's physical tilt angle to HSV hue. Rotating the board through 360 degrees produces a corresponding 360-degree color cycle across the entire ..."
  },
  {
    "id": "ard-3",
    "title": "Anti-Theft Alarm",
    "language": "Arduino",
    "category": "Sensors & Actuators",
    "level": "Intermediate",
    "filename": "Anti-Theft Alarm.ino",
    "codePath": "/advayam-codes/arduino/Anti-Theft%20Alarm.ino",
    "description": "This program arms an anti-theft alarm using the push button. Once armed, movement causes an escalating multi-tone siren."
  },
  {
    "id": "ard-4",
    "title": "Auto Dimming Matrix Display",
    "language": "Arduino",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "Auto Dimming Matrix Display.ino",
    "codePath": "/advayam-codes/arduino/Auto%20Dimming%20Matrix%20Display.ino",
    "description": "Auto Dimming Matrix Display demo project running on Advayam ESP32 development hardware using the Arduino framework."
  },
  {
    "id": "ard-5",
    "title": "Automatic Brightness Colour Temperature Simulator",
    "language": "Arduino",
    "category": "Light & Optical",
    "level": "Beginner",
    "filename": "Automatic Brightness Colour Temperature Simulator.ino",
    "codePath": "/advayam-codes/arduino/Automatic%20Brightness%20Colour%20Temperature%20Simulator.ino",
    "description": "Automatic Brightness Colour Temperature Simulator demo project running on Advayam ESP32 development hardware using the Arduino framework."
  },
  {
    "id": "ard-6",
    "title": "Automatic Text Orientation",
    "language": "Arduino",
    "category": "Motion & IMU",
    "level": "Beginner",
    "filename": "Automatic Text Orientation.ino",
    "codePath": "/advayam-codes/arduino/Automatic%20Text%20Orientation.ino",
    "description": "This program automatically rotates scrolling text according to the physical orientation of the board. The accelerometer detects the board orientation and the text is r..."
  },
  {
    "id": "ard-7",
    "title": "Blind Spot Proximity Warning",
    "language": "Arduino",
    "category": "Games & Simulations",
    "level": "Beginner",
    "filename": "Blind Spot Proximity Warning.ino",
    "codePath": "/advayam-codes/arduino/Blind%20Spot%20Proximity%20Warning.ino",
    "description": "Blind Spot Proximity Warning demo project running on Advayam ESP32 development hardware using the Arduino framework."
  },
  {
    "id": "ard-8",
    "title": "Button Hold Pitch Modulation",
    "language": "Arduino",
    "category": "Audio & Buzzer",
    "level": "Beginner",
    "filename": "Button Hold Pitch Modulation.ino",
    "codePath": "/advayam-codes/arduino/Button%20Hold%20Pitch%20Modulation.ino",
    "description": "This program measures how long the push button on GPIO 17 is held and increases the buzzer frequency as the hold time becomes longer. Releasing the button stops the bu..."
  },
  {
    "id": "ard-9",
    "title": "Button Long Press Factory Reset",
    "language": "Arduino",
    "category": "Input & Touch",
    "level": "Beginner",
    "filename": "Button Long Press Factory Reset.ino",
    "codePath": "/advayam-codes/arduino/Button%20Long%20Press%20Factory%20Reset.ino",
    "description": "This program detects how long a button connected to GPIO 17 is held. A normal short press does nothing. While the button is being held, the LED connected to GPIO 45 bl..."
  },
  {
    "id": "ard-10",
    "title": "Button Multi-Command Decoder",
    "language": "Arduino",
    "category": "Input & Touch",
    "level": "Beginner",
    "filename": "Button Multi-Command Decoder.ino",
    "codePath": "/advayam-codes/arduino/Button%20Multi-Command%20Decoder.ino",
    "description": "This program detects single, double, and triple button clicks using a button connected to GPIO 17. Depending on the number of clicks, the LED connected to GPIO 45 blin..."
  },
  {
    "id": "ard-11",
    "title": "Buzzer SOS Morse Code",
    "language": "Arduino",
    "category": "Audio & Buzzer",
    "level": "Beginner",
    "filename": "Buzzer SOS Morse Code.ino",
    "codePath": "/advayam-codes/arduino/Buzzer%20SOS%20Morse%20Code.ino",
    "description": "Buzzer SOS Morse Code demo project running on Advayam ESP32 development hardware using the Arduino framework."
  },
  {
    "id": "ard-12",
    "title": "Buzzer Tone Pitch Ladder",
    "language": "Arduino",
    "category": "Audio & Buzzer",
    "level": "Beginner",
    "filename": "Buzzer Tone Pitch Ladder.ino",
    "codePath": "/advayam-codes/arduino/Buzzer%20Tone%20Pitch%20Ladder.ino",
    "description": "Buzzer Tone Pitch Ladder This program uses a push button to cycle through 8 musical notes, from C4 to C5. Each time the button is pressed, the buzzer plays the next no..."
  },
  {
    "id": "ard-13",
    "title": "Conway'S Game Of Life",
    "language": "Arduino",
    "category": "Games & Simulations",
    "level": "Advanced",
    "filename": "Conway's Game of Life.ino",
    "codePath": "/advayam-codes/arduino/Conway's%20Game%20of%20Life.ino",
    "description": "This program runs Conway's Game of Life on the 5x5 RGB LED matrix connected to GPIO 7. The initial cells are randomized and the simulation continuously evolves on the ..."
  },
  {
    "id": "ard-14",
    "title": "Daylight Exposure Meter",
    "language": "Arduino",
    "category": "Light & Optical",
    "level": "Beginner",
    "filename": "Daylight Exposure Meter.ino",
    "codePath": "/advayam-codes/arduino/Daylight%20Exposure%20Meter.ino",
    "description": "Daylight Exposure Meter demo project running on Advayam ESP32 development hardware using the Arduino framework."
  },
  {
    "id": "ard-15",
    "title": "Double Tap Detector",
    "language": "Arduino",
    "category": "Input & Touch",
    "level": "Beginner",
    "filename": "Double Tap Detector.ino",
    "codePath": "/advayam-codes/arduino/Double%20Tap%20Detector.ino",
    "description": "This program detects two rapid acceleration spikes as a double tap. A verified double tap unlocks the system and lights the center matrix pixel green."
  },
  {
    "id": "ard-16",
    "title": "Earthquake Detector",
    "language": "Arduino",
    "category": "Motion & IMU",
    "level": "Beginner",
    "filename": "Earthquake detector.ino",
    "codePath": "/advayam-codes/arduino/Earthquake%20detector.ino",
    "description": "This program learns a quiet acceleration baseline and monitors mechanical vibration around that baseline. Strong vibrations trigger the buzzer on GPIO 16."
  },
  {
    "id": "ard-17",
    "title": "ESP32 Hardware Watchdog Toggle",
    "language": "Arduino",
    "category": "Games & Simulations",
    "level": "Beginner",
    "filename": "ESP32 Hardware Watchdog Toggle.ino",
    "codePath": "/advayam-codes/arduino/ESP32%20Hardware%20Watchdog%20Toggle.ino",
    "description": "This program uses the ESP32 Task Watchdog Timer to monitor the main loop. The User LED on GPIO 45 toggles whenever the loop executes successfully and feeds the watchdog."
  },
  {
    "id": "ard-18",
    "title": "Flip To Mute",
    "language": "Arduino",
    "category": "Sensors & Actuators",
    "level": "Beginner",
    "filename": "Flip to Mute.ino",
    "codePath": "/advayam-codes/arduino/Flip%20to%20Mute.ino",
    "description": "This program demonstrates flip-to-mute behavior. A buzzer alarm runs until the board is turned face down. Face-down orientation immediately silences the buzzer."
  },
  {
    "id": "ard-19",
    "title": "Free Fall Detector",
    "language": "Arduino",
    "category": "Motion & IMU",
    "level": "Beginner",
    "filename": "Free Fall Detector.ino",
    "codePath": "/advayam-codes/arduino/Free%20Fall%20Detector.ino",
    "description": "This program detects free-fall using the LSM6DS accelerometer. When total acceleration drops close to zero, the buzzer on GPIO 16 sounds immediately. GPIO 38 is used a..."
  },
  {
    "id": "ard-20",
    "title": "G-Force Overload Detector",
    "language": "Arduino",
    "category": "Motion & IMU",
    "level": "Beginner",
    "filename": "G-Force Overload Detector.ino",
    "codePath": "/advayam-codes/arduino/G-Force%20Overload%20Detector.ino",
    "description": "This program monitors total acceleration. If acceleration exceeds 2.5g, the matrix flashes red and the buzzer produces a high-frequency warning chirp."
  },
  {
    "id": "ard-21",
    "title": "Gesture Colour Changer",
    "language": "Arduino",
    "category": "Input & Touch",
    "level": "Beginner",
    "filename": "Gesture Colour Changer.ino",
    "codePath": "/advayam-codes/arduino/Gesture%20Colour%20Changer.ino",
    "description": "This program detects a quick left-to-right flick of the board. Each detected gesture changes the RGB matrix to the next color in a predefined palette."
  },
  {
    "id": "ard-22",
    "title": "Gyro-Based Spin Meter",
    "language": "Arduino",
    "category": "Motion & IMU",
    "level": "Intermediate",
    "filename": "Gyro-based Spin Meter.ino",
    "codePath": "/advayam-codes/arduino/Gyro-based%20Spin%20Meter.ino",
    "description": "This program measures rotation around the Z axis. A bright pixel travels around the matrix edge. Its speed increases as the measured angular velocity increases."
  },
  {
    "id": "ard-23",
    "title": "Hand Proximity Reaction Game",
    "language": "Arduino",
    "category": "Games & Simulations",
    "level": "Advanced",
    "filename": "Hand Proximity Reaction Game.ino",
    "codePath": "/advayam-codes/arduino/Hand%20Proximity%20Reaction%20Game.ino",
    "description": "Hand Proximity Reaction Game demo project running on Advayam ESP32 development hardware using the Arduino framework."
  },
  {
    "id": "ard-24",
    "title": "Inclinometer Audio Feedback",
    "language": "Arduino",
    "category": "Audio & Buzzer",
    "level": "Beginner",
    "filename": "Inclinometer Audio Feedback.ino",
    "codePath": "/advayam-codes/arduino/Inclinometer%20Audio%20Feedback.ino",
    "description": "This program measures the board's pitch relative to a 45-degree target. The buzzer beeps increasingly quickly as the board approaches the target angle."
  },
  {
    "id": "ard-25",
    "title": "Laser Tripwire Alarm",
    "language": "Arduino",
    "category": "Sensors & Actuators",
    "level": "Intermediate",
    "filename": "Laser Tripwire Alarm.ino",
    "codePath": "/advayam-codes/arduino/Laser%20Tripwire%20Alarm.ino",
    "description": "Laser Tripwire Alarm demo project running on Advayam ESP32 development hardware using the Arduino framework."
  },
  {
    "id": "ard-26",
    "title": "LED Heartbeat",
    "language": "Arduino",
    "category": "Light & Optical",
    "level": "Beginner",
    "filename": "LED Heartbeat.ino",
    "codePath": "/advayam-codes/arduino/LED%20Heartbeat.ino",
    "description": "This program simulates a human heartbeat using the User LED on GPIO 45. The LED performs two fading pulses followed by a short pause, creating a repeating heartbeat-li..."
  },
  {
    "id": "ard-27",
    "title": "LED Matrix Analog Clock Mimic",
    "language": "Arduino",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "LED Matrix Analog Clock Mimic.ino",
    "codePath": "/advayam-codes/arduino/LED%20Matrix%20Analog%20Clock%20Mimic.ino",
    "description": "LED Matrix Analog Clock Mimic demo project running on Advayam ESP32 development hardware using the Arduino framework."
  },
  {
    "id": "ard-28",
    "title": "LED Matrix Binary Counter",
    "language": "Arduino",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "LED Matrix Binary Counter.ino",
    "codePath": "/advayam-codes/arduino/LED%20Matrix%20Binary%20Counter.ino",
    "description": "8-bit Binary Counter Each press of the button increases an 8-bit counter by 1. The first 8 LEDs of the matrix represent the binary value: - LED ON = 1 - LED DIM = 0 Th..."
  },
  {
    "id": "ard-29",
    "title": "LED Matrix Catch The Falling Fruit Game",
    "language": "Arduino",
    "category": "LED Matrix",
    "level": "Advanced",
    "filename": "LED Matrix Catch the Falling Fruit Game.ino",
    "codePath": "/advayam-codes/arduino/LED%20Matrix%20Catch%20the%20Falling%20Fruit%20Game.ino",
    "description": "LED Matrix Catch The Falling Fruit Game demo project running on Advayam ESP32 development hardware using the Arduino framework."
  },
  {
    "id": "ard-30",
    "title": "LED Matrix Checkerboard Strobe",
    "language": "Arduino",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "LED Matrix Checkerboard Strobe.ino",
    "codePath": "/advayam-codes/arduino/LED%20Matrix%20Checkerboard%20Strobe.ino",
    "description": "This program alternates two complementary colors across the 5x5 RGB matrix in a checkerboard pattern, creating a strobe effect."
  },
  {
    "id": "ard-31",
    "title": "LED Matrix Coin Flip",
    "language": "Arduino",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "LED Matrix Coin Flip.ino",
    "codePath": "/advayam-codes/arduino/LED%20Matrix%20Coin%20Flip.ino",
    "description": "Coin Flip Simulator Press the button to flip the coin. The matrix shows a spinning coin animation, then stops on HEADS or TAILS and plays a chime."
  },
  {
    "id": "ard-32",
    "title": "LED Matrix Color Mixer",
    "language": "Arduino",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "LED Matrix Color Mixer.ino",
    "codePath": "/advayam-codes/arduino/LED%20Matrix%20Color%20Mixer.ino",
    "description": "This program mixes Red, Green, and Blue components on the RGB matrix using the push button on GPIO 17. Short presses cycle through 8 brightness levels. A long press co..."
  },
  {
    "id": "ard-33",
    "title": "LED Matrix Colour Temp Simulator",
    "language": "Arduino",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "LED Matrix Colour Temp Simulator.ino",
    "codePath": "/advayam-codes/arduino/LED%20Matrix%20Colour%20Temp%20Simulator.ino",
    "description": "LED Matrix Colour Temp Simulator demo project running on Advayam ESP32 development hardware using the Arduino framework."
  },
  {
    "id": "ard-34",
    "title": "LED Matrix Diagonal Colour Wave",
    "language": "Arduino",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "LED Matrix Diagonal Colour Wave.ino",
    "codePath": "/advayam-codes/arduino/LED%20Matrix%20Diagonal%20Colour%20Wave.ino",
    "description": "This program creates a moving diagonal rainbow wave across the 5x5 RGB LED matrix connected to GPIO 7."
  },
  {
    "id": "ard-35",
    "title": "LED Matrix Digital Bubble Meter",
    "language": "Arduino",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "LED Matrix Digital Bubble Meter.ino",
    "codePath": "/advayam-codes/arduino/LED%20Matrix%20Digital%20Bubble%20Meter.ino",
    "description": "This program uses the accelerometer to create a digital bubble level. The green center LED represents the bubble. The bubble moves according to board tilt and stays ce..."
  },
  {
    "id": "ard-36",
    "title": "LED Matrix Digital Dice",
    "language": "Arduino",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "LED Matrix Digital Dice.ino",
    "codePath": "/advayam-codes/arduino/LED%20Matrix%20Digital%20Dice.ino",
    "description": "Digital Dice Roller Pressing the button starts an animated dice-rolling effect. After the animation, a random number from 1 to 6 is selected and displayed on the 5x5 N..."
  },
  {
    "id": "ard-37",
    "title": "LED Matrix Digital Stopwatch",
    "language": "Arduino",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "LED Matrix Digital Stopwatch.ino",
    "codePath": "/advayam-codes/arduino/LED%20Matrix%20Digital%20Stopwatch.ino",
    "description": "Stopwatch on Matrix This program uses a push button to start and stop a stopwatch. The LED matrix progressively fills with one LED for each elapsed second. After 25 se..."
  },
  {
    "id": "ard-38",
    "title": "LED Matrix Expanding Color Wave",
    "language": "Arduino",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "LED Matrix Expanding Color Wave.ino",
    "codePath": "/advayam-codes/arduino/LED%20Matrix%20Expanding%20Color%20Wave.ino",
    "description": "This program animates expanding square frames on the 5x5 RGB LED matrix, starting from the center and expanding toward the outer boundary."
  },
  {
    "id": "ard-39",
    "title": "LED Matrix Fire Simulation",
    "language": "Arduino",
    "category": "LED Matrix",
    "level": "Advanced",
    "filename": "LED Matrix Fire Simulation.ino",
    "codePath": "/advayam-codes/arduino/LED%20Matrix%20Fire%20Simulation.ino",
    "description": "This program creates a flickering fire effect on the 5x5 RGB LED matrix using a simulated heat map. Heat is converted into red, orange, and yellow colors."
  },
  {
    "id": "ard-40",
    "title": "LED Matrix Heatmap Generator",
    "language": "Arduino",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "LED Matrix Heatmap Generator.ino",
    "codePath": "/advayam-codes/arduino/LED%20Matrix%20Heatmap%20Generator.ino",
    "description": "This program displays the LMT87 temperature as a vertical heatmap on the 5x5 RGB matrix. Lower temperatures produce blue colors while higher temperatures progress thro..."
  },
  {
    "id": "ard-41",
    "title": "LED Matrix Horizontal Scanner",
    "language": "Arduino",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "LED Matrix Horizontal Scanner.ino",
    "codePath": "/advayam-codes/arduino/LED%20Matrix%20Horizontal%20Scanner.ino",
    "description": "Matrix Column Scanner Each button press moves a solid vertical bar one column to the right. After the fifth column, it returns to the first. Includes software button d..."
  },
  {
    "id": "ard-42",
    "title": "LED Matrix Hourglass",
    "language": "Arduino",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "LED Matrix Hourglass.ino",
    "codePath": "/advayam-codes/arduino/LED%20Matrix%20Hourglass.ino",
    "description": "5x5 Falling Sand Timer - Sand starts at the TOP row, CENTER pixel - Each grain falls downward - Grains pile up naturally - Matrix gradually fills with sand - All 25 pi..."
  },
  {
    "id": "ard-43",
    "title": "LED Matrix Maze Game",
    "language": "Arduino",
    "category": "LED Matrix",
    "level": "Advanced",
    "filename": "LED Matrix Maze Game.ino",
    "codePath": "/advayam-codes/arduino/LED%20Matrix%20Maze%20Game.ino",
    "description": "LED Matrix Maze Game demo project running on Advayam ESP32 development hardware using the Arduino framework."
  },
  {
    "id": "ard-44",
    "title": "LED Matrix Memory Game",
    "language": "Arduino",
    "category": "LED Matrix",
    "level": "Advanced",
    "filename": "LED Matrix Memory Game.ino",
    "codePath": "/advayam-codes/arduino/LED%20Matrix%20Memory%20Game.ino",
    "description": "LED Matrix Memory Game demo project running on Advayam ESP32 development hardware using the Arduino framework."
  },
  {
    "id": "ard-45",
    "title": "LED Matrix Minesweeper",
    "language": "Arduino",
    "category": "LED Matrix",
    "level": "Advanced",
    "filename": "LED Matrix Minesweeper.ino",
    "codePath": "/advayam-codes/arduino/LED%20Matrix%20Minesweeper.ino",
    "description": "LED Matrix Minesweeper demo project running on Advayam ESP32 development hardware using the Arduino framework."
  },
  {
    "id": "ard-46",
    "title": "LED Matrix Night Light",
    "language": "Arduino",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "LED Matrix Night Light.ino",
    "codePath": "/advayam-codes/arduino/LED%20Matrix%20Night%20Light.ino",
    "description": "LED Matrix Night Light demo project running on Advayam ESP32 development hardware using the Arduino framework."
  },
  {
    "id": "ard-47",
    "title": "LED Matrix NTP Scrolling Clock",
    "language": "Arduino",
    "category": "LED Matrix",
    "level": "Advanced",
    "filename": "LED Matrix NTP Scrolling Clock.ino",
    "codePath": "/advayam-codes/arduino/LED%20Matrix%20NTP%20Scrolling%20Clock.ino",
    "description": "NTP Clock - GMT+5:30 - Upside-Down Scrolling Time This program connects the ESP32-S3 to Wi-Fi and gets the current time from an NTP server. The clock uses Indian Stand..."
  },
  {
    "id": "ard-48",
    "title": "LED Matrix Pacman Chase Animation",
    "language": "Arduino",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "LED Matrix Pacman Chase animation.ino",
    "codePath": "/advayam-codes/arduino/LED%20Matrix%20Pacman%20Chase%20animation.ino",
    "description": "This program creates a simple Pacman-style chase animation. A yellow Pacman pixel is chased by a red ghost pixel across the 5x5 RGB matrix."
  },
  {
    "id": "ard-49",
    "title": "LED Matrix Perlin Noise Gradient Generator",
    "language": "Arduino",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "LED Matrix Perlin Noise Gradient Generator.ino",
    "codePath": "/advayam-codes/arduino/LED%20Matrix%20Perlin%20Noise%20Gradient%20Generator.ino",
    "description": "This program generates smoothly changing procedural color gradients across the 5x5 RGB matrix using interpolated noise."
  },
  {
    "id": "ard-50",
    "title": "LED Matrix Pixel Collision Simulator",
    "language": "Arduino",
    "category": "LED Matrix",
    "level": "Advanced",
    "filename": "LED Matrix Pixel Collision Simulator.ino",
    "codePath": "/advayam-codes/arduino/LED%20Matrix%20Pixel%20Collision%20Simulator.ino",
    "description": "LED Matrix Pixel Collision Simulator demo project running on Advayam ESP32 development hardware using the Arduino framework."
  },
  {
    "id": "ard-51",
    "title": "LED Matrix Pixel Particle Fountain",
    "language": "Arduino",
    "category": "LED Matrix",
    "level": "Advanced",
    "filename": "LED Matrix Pixel Particle Fountain.ino",
    "codePath": "/advayam-codes/arduino/LED%20Matrix%20Pixel%20Particle%20Fountain.ino",
    "description": "This program creates a particle fountain from the bottom-center of the 5x5 RGB matrix. Particles rise upward while drifting left or right and gradually fade."
  },
  {
    "id": "ard-52",
    "title": "LED Matrix Pong Game",
    "language": "Arduino",
    "category": "LED Matrix",
    "level": "Advanced",
    "filename": "LED Matrix Pong Game.ino",
    "codePath": "/advayam-codes/arduino/LED%20Matrix%20Pong%20Game.ino",
    "description": "LED Matrix Pong Game demo project running on Advayam ESP32 development hardware using the Arduino framework."
  },
  {
    "id": "ard-53",
    "title": "LED Matrix Radar Sweep Animation",
    "language": "Arduino",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "LED Matrix Radar Sweep Animation.ino",
    "codePath": "/advayam-codes/arduino/LED%20Matrix%20Radar%20Sweep%20Animation.ino",
    "description": "This program simulates a radar sweep rotating around the center pixel of the 5x5 matrix. Green trails fade behind the rotating radar line."
  },
  {
    "id": "ard-54",
    "title": "LED Matrix Rhythm Game",
    "language": "Arduino",
    "category": "LED Matrix",
    "level": "Advanced",
    "filename": "LED Matrix Rhythm Game.ino",
    "codePath": "/advayam-codes/arduino/LED%20Matrix%20Rhythm%20Game.ino",
    "description": "LED Matrix Rhythm Game demo project running on Advayam ESP32 development hardware using the Arduino framework."
  },
  {
    "id": "ard-55",
    "title": "LED Matrix Rolling Marble Simulator",
    "language": "Arduino",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "LED Matrix Rolling Marble Simulator.ino",
    "codePath": "/advayam-codes/arduino/LED%20Matrix%20Rolling%20Marble%20Simulator.ino",
    "description": "This program simulates a rolling marble on the 5x5 matrix. Board tilt accelerates the marble while simulated friction gradually slows it down."
  },
  {
    "id": "ard-56",
    "title": "LED Matrix Rotating Colour Wheel",
    "language": "Arduino",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "LED Matrix Rotating Colour Wheel.ino",
    "codePath": "/advayam-codes/arduino/LED%20Matrix%20Rotating%20Colour%20Wheel.ino",
    "description": "This program generates a rotating rainbow color wheel across all 25 pixels of the RGB matrix using HSV color space."
  },
  {
    "id": "ard-57",
    "title": "LED Matrix Sand Falling Animation",
    "language": "Arduino",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "LED Matrix Sand Falling Animation.ino",
    "codePath": "/advayam-codes/arduino/LED%20Matrix%20Sand%20Falling%20Animation.ino",
    "description": "This program simulates sand particles on the 5x5 matrix. Board tilt determines which direction the sand falls."
  },
  {
    "id": "ard-58",
    "title": "LED Matrix Simulated VU Meter",
    "language": "Arduino",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "LED Matrix Simulated VU Meter.ino",
    "codePath": "/advayam-codes/arduino/LED%20Matrix%20Simulated%20VU%20Meter.ino",
    "description": "This program simulates a five-bar audio VU meter on the 5x5 RGB matrix."
  },
  {
    "id": "ard-59",
    "title": "LED Matrix Snake Game",
    "language": "Arduino",
    "category": "LED Matrix",
    "level": "Advanced",
    "filename": "LED Matrix Snake Game.ino",
    "codePath": "/advayam-codes/arduino/LED%20Matrix%20Snake%20Game.ino",
    "description": "LED Matrix Snake Game demo project running on Advayam ESP32 development hardware using the Arduino framework."
  },
  {
    "id": "ard-60",
    "title": "LED Matrix Space Invaders Game",
    "language": "Arduino",
    "category": "LED Matrix",
    "level": "Advanced",
    "filename": "LED Matrix Space Invaders Game.ino",
    "codePath": "/advayam-codes/arduino/LED%20Matrix%20Space%20Invaders%20Game.ino",
    "description": "LED Matrix Space Invaders Game demo project running on Advayam ESP32 development hardware using the Arduino framework."
  },
  {
    "id": "ard-61",
    "title": "LED Matrix Spiral Fill",
    "language": "Arduino",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "LED Matrix Spiral Fill.ino",
    "codePath": "/advayam-codes/arduino/LED%20Matrix%20Spiral%20Fill.ino",
    "description": "This program illuminates every pixel of the 5x5 RGB matrix one-by-one in a clockwise spiral from the outside toward the center."
  },
  {
    "id": "ard-62",
    "title": "LED Matrix Spirit Level",
    "language": "Arduino",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "LED Matrix Spirit Level.ino",
    "codePath": "/advayam-codes/arduino/LED%20Matrix%20Spirit%20Level.ino",
    "description": "This program provides a precision digital spirit level. Green means the board is level. Yellow indicates a small error and red indicates a larger tilt."
  },
  {
    "id": "ard-63",
    "title": "LED Matrix Starfield Animation",
    "language": "Arduino",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "LED Matrix Starfield Animation.ino",
    "codePath": "/advayam-codes/arduino/LED%20Matrix%20Starfield%20Animation.ino",
    "description": "LED Matrix Starfield Animation demo project running on Advayam ESP32 development hardware using the Arduino framework."
  },
  {
    "id": "ard-64",
    "title": "LED Matrix Strobe Light",
    "language": "Arduino",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "LED Matrix Strobe Light.ino",
    "codePath": "/advayam-codes/arduino/LED%20Matrix%20Strobe%20Light.ino",
    "description": "LED Matrix Strobe Light This program uses a push button to control a strobe effect on a 25-pixel NeoPixel LED matrix. The first button press turns the strobe ON at 1 H..."
  },
  {
    "id": "ard-65",
    "title": "LED Matrix Swipe Detector",
    "language": "Arduino",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "LED Matrix Swipe Detector.ino",
    "codePath": "/advayam-codes/arduino/LED%20Matrix%20Swipe%20Detector.ino",
    "description": "LED Matrix Swipe Detector demo project running on Advayam ESP32 development hardware using the Arduino framework."
  },
  {
    "id": "ard-66",
    "title": "LED Matrix Temperature Controlled Dynamic Smiley Faces",
    "language": "Arduino",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "LED Matrix Temperature Controlled Dynamic Smiley Faces.ino",
    "codePath": "/advayam-codes/arduino/LED%20Matrix%20Temperature%20Controlled%20Dynamic%20Smiley%20Faces.ino",
    "description": "This program displays happy, neutral, or sad faces on the RGB matrix depending on the LMT87 temperature. The face is displayed upside down. As temperature increases: C..."
  },
  {
    "id": "ard-67",
    "title": "LED Matrix Temperature Controlled Fire Simulation",
    "language": "Arduino",
    "category": "LED Matrix",
    "level": "Advanced",
    "filename": "LED Matrix Temperature Controlled Fire Simulation.ino",
    "codePath": "/advayam-codes/arduino/LED%20Matrix%20Temperature%20Controlled%20Fire%20Simulation.ino",
    "description": "This program creates a temperature-controlled fire effect on the 5x5 RGB LED matrix using the LMT87 sensor on GPIO 1. Below 30 C the matrix remains off. Above 30 C the..."
  },
  {
    "id": "ard-68",
    "title": "LED Matrix Tetris Drop Animation",
    "language": "Arduino",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "LED Matrix Tetris Drop Animation.ino",
    "codePath": "/advayam-codes/arduino/LED%20Matrix%20Tetris%20Drop%20Animation.ino",
    "description": "This program animates simple falling tetromino blocks on the 5x5 RGB matrix."
  },
  {
    "id": "ard-69",
    "title": "LED Matrix Text Scroller",
    "language": "Arduino",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "LED Matrix Text Scroller.ino",
    "codePath": "/advayam-codes/arduino/LED%20Matrix%20Text%20Scroller.ino",
    "description": "This program scrolls an arbitrary text message horizontally across the 5x5 RGB matrix one pixel at a time. Letters A-Z, numbers 0-9, and spaces are supported."
  },
  {
    "id": "ard-70",
    "title": "LED Matrix Toggle State",
    "language": "Arduino",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "LED Matrix Toggle State.ino",
    "codePath": "/advayam-codes/arduino/LED%20Matrix%20Toggle%20State.ino",
    "description": "Button Toggle with Audible Feedback Each button press toggles the LED matrix ON or OFF. A short buzzer click provides audible feedback for each button press."
  },
  {
    "id": "ard-71",
    "title": "LED Matrix Touchless Trashcan Mimic",
    "language": "Arduino",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "LED Matrix Touchless Trashcan Mimic.ino",
    "codePath": "/advayam-codes/arduino/LED%20Matrix%20Touchless%20Trashcan%20Mimic.ino",
    "description": "LED Matrix Touchless Trashcan Mimic demo project running on Advayam ESP32 development hardware using the Arduino framework."
  },
  {
    "id": "ard-72",
    "title": "LED Matrix Tug Of War Game",
    "language": "Arduino",
    "category": "LED Matrix",
    "level": "Advanced",
    "filename": "LED Matrix Tug of War Game.ino",
    "codePath": "/advayam-codes/arduino/LED%20Matrix%20Tug%20of%20War%20Game.ino",
    "description": "LED Matrix Tug Of War Game demo project running on Advayam ESP32 development hardware using the Arduino framework."
  },
  {
    "id": "ard-73",
    "title": "LED Matrix Vertical Scanner",
    "language": "Arduino",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "LED Matrix Vertical Scanner.ino",
    "codePath": "/advayam-codes/arduino/LED%20Matrix%20Vertical%20Scanner.ino",
    "description": "Matrix Row Scanner Each button press moves a solid horizontal bar one row upward. After the fifth row, it returns to the first. Includes software button debouncing."
  },
  {
    "id": "ard-74",
    "title": "LED Matrix Whack-A-Mole",
    "language": "Arduino",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "LED Matrix Whack-a-Mole.ino",
    "codePath": "/advayam-codes/arduino/LED%20Matrix%20Whack-a-Mole.ino",
    "description": "LED Matrix Whack-A-Mole demo project running on Advayam ESP32 development hardware using the Arduino framework."
  },
  {
    "id": "ard-75",
    "title": "LED Matrix WiFi Signal Strength Indicator",
    "language": "Arduino",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "LED Matrix WiFi Signal Strength Indicator.ino",
    "codePath": "/advayam-codes/arduino/LED%20Matrix%20WiFi%20Signal%20Strength%20Indicator.ino",
    "description": "This program connects to Wi-Fi and displays the signal strength as 1 to 4 bars on the 5x5 RGB matrix based on Wi-Fi RSSI."
  },
  {
    "id": "ard-76",
    "title": "Matrix Rain Effect",
    "language": "Arduino",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "Matrix Rain Effect.ino",
    "codePath": "/advayam-codes/arduino/Matrix%20Rain%20Effect.ino",
    "description": "Green digital rain effect on a 5x5 RGB LED matrix."
  },
  {
    "id": "ard-77",
    "title": "Overheat Alarm",
    "language": "Arduino",
    "category": "Thermal & Climate",
    "level": "Intermediate",
    "filename": "Overheat Alarm.ino",
    "codePath": "/advayam-codes/arduino/Overheat%20Alarm.ino",
    "description": "This program monitors temperature using an LMT87 temperature sensor connected to GPIO 1. The measured temperature is displayed on the Serial Monitor. If the temperatur..."
  },
  {
    "id": "ard-78",
    "title": "Proximity Based Greeting",
    "language": "Arduino",
    "category": "Input & Touch",
    "level": "Beginner",
    "filename": "Proximity Based Greeting.ino",
    "codePath": "/advayam-codes/arduino/Proximity%20Based%20Greeting.ino",
    "description": "Proximity Based Greeting demo project running on Advayam ESP32 development hardware using the Arduino framework."
  },
  {
    "id": "ard-79",
    "title": "Proximity Counter",
    "language": "Arduino",
    "category": "Input & Touch",
    "level": "Beginner",
    "filename": "Proximity Counter.ino",
    "codePath": "/advayam-codes/arduino/Proximity%20Counter.ino",
    "description": "Proximity Counter demo project running on Advayam ESP32 development hardware using the Arduino framework."
  },
  {
    "id": "ard-80",
    "title": "Proximity Depth Gauge",
    "language": "Arduino",
    "category": "Input & Touch",
    "level": "Beginner",
    "filename": "Proximity Depth Gauge.ino",
    "codePath": "/advayam-codes/arduino/Proximity%20Depth%20Gauge.ino",
    "description": "Proximity Depth Gauge demo project running on Advayam ESP32 development hardware using the Arduino framework."
  },
  {
    "id": "ard-81",
    "title": "Proximity Distance Theremin",
    "language": "Arduino",
    "category": "Audio & Buzzer",
    "level": "Intermediate",
    "filename": "Proximity Distance Theremin.ino",
    "codePath": "/advayam-codes/arduino/Proximity%20Distance%20Theremin.ino",
    "description": "Proximity Distance Theremin demo project running on Advayam ESP32 development hardware using the Arduino framework."
  },
  {
    "id": "ard-82",
    "title": "Reaction Time Game",
    "language": "Arduino",
    "category": "Games & Simulations",
    "level": "Advanced",
    "filename": "Reaction Time Game.ino",
    "codePath": "/advayam-codes/arduino/Reaction%20Time%20Game.ino",
    "description": "Reaction Timer Game The ESP32 waits for the user to press the button, then waits for a random amount of time before sounding the buzzer. When the buzzer sounds, the pl..."
  },
  {
    "id": "ard-83",
    "title": "Roll And Pitch Reader",
    "language": "Arduino",
    "category": "Audio & Buzzer",
    "level": "Beginner",
    "filename": "Roll and Pitch Reader.ino",
    "codePath": "/advayam-codes/arduino/Roll%20and%20Pitch%20Reader.ino",
    "description": "This program calculates board roll and pitch from the accelerometer. Values are streamed as CSV over UART for real-time plotting. Output format: time_ms,roll,pitch"
  },
  {
    "id": "ard-84",
    "title": "Shadow Puppetry Brightness Detector",
    "language": "Arduino",
    "category": "Light & Optical",
    "level": "Beginner",
    "filename": "Shadow Puppetry Brightness Detector.ino",
    "codePath": "/advayam-codes/arduino/Shadow%20Puppetry%20Brightness%20Detector.ino",
    "description": "Shadow Puppetry Brightness Detector demo project running on Advayam ESP32 development hardware using the Arduino framework."
  },
  {
    "id": "ard-85",
    "title": "Shake To Shuffle Matrix Randomizer",
    "language": "Arduino",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "Shake to Shuffle Matrix Randomizer.ino",
    "codePath": "/advayam-codes/arduino/Shake%20to%20Shuffle%20Matrix%20Randomizer.ino",
    "description": "This program detects a strong shake using the accelerometer. A rapid acceleration change triggers a randomized burst of colors across the 5x5 RGB matrix."
  },
  {
    "id": "ard-86",
    "title": "Six Orientation Detector",
    "language": "Arduino",
    "category": "Motion & IMU",
    "level": "Beginner",
    "filename": "Six Orientation Detector.ino",
    "codePath": "/advayam-codes/arduino/Six%20Orientation%20Detector.ino",
    "description": "This program determines which of six directions the board is facing. The matrix displays a colored pixel indicating Up, Down, Left, Right, Forward, or Backward."
  },
  {
    "id": "ard-87",
    "title": "Step Counter",
    "language": "Arduino",
    "category": "Motion & IMU",
    "level": "Beginner",
    "filename": "Step Counter.ino",
    "codePath": "/advayam-codes/arduino/Step%20Counter.ino",
    "description": "This program estimates walking steps from acceleration peaks. Each detected step increments the counter and displays the current count in binary on the 5x5 matrix."
  },
  {
    "id": "ard-88",
    "title": "Temperature High-Low Latch",
    "language": "Arduino",
    "category": "Thermal & Climate",
    "level": "Intermediate",
    "filename": "Temperature High-Low Latch.ino",
    "codePath": "/advayam-codes/arduino/Temperature%20High-Low%20Latch.ino",
    "description": "This program monitors the temperature using an LMT87 sensor connected to GPIO 1 and keeps track of the temperature range measured since the board was started. It conti..."
  },
  {
    "id": "ard-89",
    "title": "Temperature Reading Comparison",
    "language": "Arduino",
    "category": "Thermal & Climate",
    "level": "Beginner",
    "filename": "Temperature Reading Comparison.ino",
    "codePath": "/advayam-codes/arduino/Temperature%20Reading%20Comparison.ino",
    "description": "This program compares the LSM6DS internal temperature sensor with the LMT87 analog temperature sensor on GPIO 1. Both temperatures are printed over UART for comparison."
  },
  {
    "id": "ard-90",
    "title": "Temperature To Pitch Converter",
    "language": "Arduino",
    "category": "Audio & Buzzer",
    "level": "Beginner",
    "filename": "Temperature to Pitch Converter.ino",
    "codePath": "/advayam-codes/arduino/Temperature%20to%20Pitch%20Converter.ino",
    "description": "This program reads the LMT87 temperature sensor on GPIO 1 and maps the measured temperature to a buzzer frequency on GPIO 16. Higher temperatures produce higher pitch."
  },
  {
    "id": "ard-91",
    "title": "Tilt Balance Stability Game",
    "language": "Arduino",
    "category": "Games & Simulations",
    "level": "Advanced",
    "filename": "Tilt Balance Stability Game.ino",
    "codePath": "/advayam-codes/arduino/Tilt%20Balance%20Stability%20Game.ino",
    "description": "This program challenges the user to keep the board level. The center LED remains green while stable. The player must keep the bubble inside the center area for 15 seco..."
  },
  {
    "id": "ard-92",
    "title": "Tilt Controlled Dice Roller",
    "language": "Arduino",
    "category": "Games & Simulations",
    "level": "Beginner",
    "filename": "Tilt Controlled Dice Roller.ino",
    "codePath": "/advayam-codes/arduino/Tilt%20Controlled%20Dice%20Roller.ino",
    "description": "This program creates a virtual six-sided die. Shaking or tilting the board makes the die roll. When the board becomes flat again, the result settles between 1 and 6."
  },
  {
    "id": "ard-93",
    "title": "Tilt Controlled Maze Game",
    "language": "Arduino",
    "category": "Games & Simulations",
    "level": "Advanced",
    "filename": "Tilt Controlled Maze Game.ino",
    "codePath": "/advayam-codes/arduino/Tilt%20Controlled%20Maze%20Game.ino",
    "description": "This program creates a simple tilt-controlled maze game. Tilt the board to move the green player through the maze. Blue pixels are walls and red is the goal."
  },
  {
    "id": "ard-94",
    "title": "Tilt To Sound Synthesizer",
    "language": "Arduino",
    "category": "Audio & Buzzer",
    "level": "Beginner",
    "filename": "Tilt to Sound Synthesizer.ino",
    "codePath": "/advayam-codes/arduino/Tilt%20to%20Sound%20Synthesizer.ino",
    "description": "This program turns board tilt into sound. Pitch is controlled by board pitch and buzzer duty cycle is controlled by board roll."
  },
  {
    "id": "ard-95",
    "title": "Touchless Page Turner",
    "language": "Arduino",
    "category": "Input & Touch",
    "level": "Beginner",
    "filename": "Touchless Page Turner.ino",
    "codePath": "/advayam-codes/arduino/Touchless%20Page%20Turner.ino",
    "description": "Touchless Page Turner demo project running on Advayam ESP32 development hardware using the Arduino framework."
  },
  {
    "id": "ard-96",
    "title": "Touchless Proximity Switch",
    "language": "Arduino",
    "category": "Input & Touch",
    "level": "Beginner",
    "filename": "Touchless Proximity Switch.ino",
    "codePath": "/advayam-codes/arduino/Touchless%20Proximity%20Switch.ino",
    "description": "Touchless Proximity Switch demo project running on Advayam ESP32 development hardware using the Arduino framework."
  },
  {
    "id": "ard-97",
    "title": "Touchless Volume Control Knob",
    "language": "Arduino",
    "category": "Input & Touch",
    "level": "Beginner",
    "filename": "Touchless Volume Control Knob.ino",
    "codePath": "/advayam-codes/arduino/Touchless%20Volume%20Control%20Knob.ino",
    "description": "Touchless Volume Control Knob demo project running on Advayam ESP32 development hardware using the Arduino framework."
  },
  {
    "id": "esp-1",
    "title": "Ambient Lux Data Logger",
    "language": "ESP-IDF",
    "category": "Light & Optical",
    "level": "Beginner",
    "filename": "Ambient Lux Data Logger.c",
    "codePath": "/advayam-codes/esp-idf/Ambient%20Lux%20Data%20Logger.c",
    "description": "Ambient Lux Data Logger demo project running on Advayam ESP32 development hardware using the ESP-IDF framework."
  },
  {
    "id": "esp-2",
    "title": "Angle Based HSV Colour Wheel",
    "language": "ESP-IDF",
    "category": "Sensors & Actuators",
    "level": "Intermediate",
    "filename": "Angle Based HSV Colour Wheel.c",
    "codePath": "/advayam-codes/esp-idf/Angle%20Based%20HSV%20Colour%20Wheel.c",
    "description": "Angle Based HSV Colour Wheel demo project running on Advayam ESP32 development hardware using the ESP-IDF framework."
  },
  {
    "id": "esp-3",
    "title": "Anti-Theft Alarm",
    "language": "ESP-IDF",
    "category": "Sensors & Actuators",
    "level": "Intermediate",
    "filename": "Anti-Theft Alarm.c",
    "codePath": "/advayam-codes/esp-idf/Anti-Theft%20Alarm.c",
    "description": "Anti-Theft Alarm demo project running on Advayam ESP32 development hardware using the ESP-IDF framework."
  },
  {
    "id": "esp-4",
    "title": "Auto Dimming Matrix Display",
    "language": "ESP-IDF",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "Auto Dimming Matrix Display.c",
    "codePath": "/advayam-codes/esp-idf/Auto%20Dimming%20Matrix%20Display.c",
    "description": "Auto Dimming Matrix Display demo project running on Advayam ESP32 development hardware using the ESP-IDF framework."
  },
  {
    "id": "esp-5",
    "title": "Automatic Brightness Colour Temperature",
    "language": "ESP-IDF",
    "category": "Light & Optical",
    "level": "Beginner",
    "filename": "Automatic Brightness Colour Temperature.c",
    "codePath": "/advayam-codes/esp-idf/Automatic%20Brightness%20Colour%20Temperature.c",
    "description": "Automatic Brightness Colour Temperature demo project running on Advayam ESP32 development hardware using the ESP-IDF framework."
  },
  {
    "id": "esp-6",
    "title": "Automatic Text Orientation",
    "language": "ESP-IDF",
    "category": "Motion & IMU",
    "level": "Beginner",
    "filename": "Automatic Text Orientation.c",
    "codePath": "/advayam-codes/esp-idf/Automatic%20Text%20Orientation.c",
    "description": "Automatic Text Orientation demo project running on Advayam ESP32 development hardware using the ESP-IDF framework."
  },
  {
    "id": "esp-7",
    "title": "Blind Spot Proximity Warning",
    "language": "ESP-IDF",
    "category": "Games & Simulations",
    "level": "Beginner",
    "filename": "Blind Spot Proximity Warning.c",
    "codePath": "/advayam-codes/esp-idf/Blind%20Spot%20Proximity%20Warning.c",
    "description": "Blind Spot Proximity Warning demo project running on Advayam ESP32 development hardware using the ESP-IDF framework."
  },
  {
    "id": "esp-8",
    "title": "Button Hold Pitch Modulation",
    "language": "ESP-IDF",
    "category": "Audio & Buzzer",
    "level": "Beginner",
    "filename": "Button Hold Pitch Modulation.c",
    "codePath": "/advayam-codes/esp-idf/Button%20Hold%20Pitch%20Modulation.c",
    "description": "Button Hold Pitch Modulation demo project running on Advayam ESP32 development hardware using the ESP-IDF framework."
  },
  {
    "id": "esp-9",
    "title": "Button Long Press Factory Reset",
    "language": "ESP-IDF",
    "category": "Input & Touch",
    "level": "Beginner",
    "filename": "Button Long Press Factory Reset.c",
    "codePath": "/advayam-codes/esp-idf/Button%20Long%20Press%20Factory%20Reset.c",
    "description": "This program detects how long a button connected to GPIO 17 is held. A normal short press does nothing. While the button is being held, the LED connected to GPIO 45 bl..."
  },
  {
    "id": "esp-10",
    "title": "Button Multi-Command Decoder",
    "language": "ESP-IDF",
    "category": "Input & Touch",
    "level": "Beginner",
    "filename": "Button Multi-Command Decoder.c",
    "codePath": "/advayam-codes/esp-idf/Button%20Multi-Command%20Decoder.c",
    "description": "This program detects single, double, and triple button clicks using a button connected to GPIO 17. Depending on the number of clicks, the LED connected to GPIO 45 blin..."
  },
  {
    "id": "esp-11",
    "title": "Buzzer SOS Morse Code",
    "language": "ESP-IDF",
    "category": "Audio & Buzzer",
    "level": "Beginner",
    "filename": "Buzzer SOS Morse Code.c",
    "codePath": "/advayam-codes/esp-idf/Buzzer%20SOS%20Morse%20Code.c",
    "description": "Buzzer SOS Morse Code demo project running on Advayam ESP32 development hardware using the ESP-IDF framework."
  },
  {
    "id": "esp-12",
    "title": "Buzzer Tone Pitch Ladder",
    "language": "ESP-IDF",
    "category": "Audio & Buzzer",
    "level": "Beginner",
    "filename": "Buzzer Tone Pitch Ladder.c",
    "codePath": "/advayam-codes/esp-idf/Buzzer%20Tone%20Pitch%20Ladder.c",
    "description": "Buzzer Tone Pitch Ladder This program uses a push button to cycle through 8 musical notes, from C4 to C5. Each time the button is pressed, the buzzer plays the next no..."
  },
  {
    "id": "esp-13",
    "title": "Conway'S Game Of Life",
    "language": "ESP-IDF",
    "category": "Games & Simulations",
    "level": "Advanced",
    "filename": "Conway's Game of Life.c",
    "codePath": "/advayam-codes/esp-idf/Conway's%20Game%20of%20Life.c",
    "description": "This program runs Conway's Game of Life on the 5x5 RGB LED matrix connected to GPIO 7. The initial cells are randomized and the simulation continuously evolves on the ..."
  },
  {
    "id": "esp-14",
    "title": "Daylight Exposure Meter",
    "language": "ESP-IDF",
    "category": "Light & Optical",
    "level": "Beginner",
    "filename": "Daylight Exposure Meter.c",
    "codePath": "/advayam-codes/esp-idf/Daylight%20Exposure%20Meter.c",
    "description": "Daylight Exposure Meter demo project running on Advayam ESP32 development hardware using the ESP-IDF framework."
  },
  {
    "id": "esp-15",
    "title": "Double Tap Detector",
    "language": "ESP-IDF",
    "category": "Input & Touch",
    "level": "Beginner",
    "filename": "Double Tap Detector.c",
    "codePath": "/advayam-codes/esp-idf/Double%20Tap%20Detector.c",
    "description": "This program detects two rapid acceleration spikes as a double tap. A verified double tap unlocks the system and lights the center matrix pixel green."
  },
  {
    "id": "esp-16",
    "title": "Earthquake Detector",
    "language": "ESP-IDF",
    "category": "Motion & IMU",
    "level": "Beginner",
    "filename": "Earthquake detector.c",
    "codePath": "/advayam-codes/esp-idf/Earthquake%20detector.c",
    "description": "Earthquake Detector demo project running on Advayam ESP32 development hardware using the ESP-IDF framework."
  },
  {
    "id": "esp-17",
    "title": "ESP32 Hardware Watchdog Toggle",
    "language": "ESP-IDF",
    "category": "Games & Simulations",
    "level": "Beginner",
    "filename": "ESP32 Hardware Watchdog Toggle.c",
    "codePath": "/advayam-codes/esp-idf/ESP32%20Hardware%20Watchdog%20Toggle.c",
    "description": "ESP32 Hardware Watchdog Toggle demo project running on Advayam ESP32 development hardware using the ESP-IDF framework."
  },
  {
    "id": "esp-18",
    "title": "Flip To Mute",
    "language": "ESP-IDF",
    "category": "Sensors & Actuators",
    "level": "Beginner",
    "filename": "Flip to Mute.c",
    "codePath": "/advayam-codes/esp-idf/Flip%20to%20Mute.c",
    "description": "This program demonstrates flip-to-mute behavior. A buzzer alarm runs until the board is turned face down. Face-down orientation immediately silences the buzzer."
  },
  {
    "id": "esp-19",
    "title": "Free Fall Detector",
    "language": "ESP-IDF",
    "category": "Motion & IMU",
    "level": "Beginner",
    "filename": "Free Fall Detector.c",
    "codePath": "/advayam-codes/esp-idf/Free%20Fall%20Detector.c",
    "description": "This program detects free-fall using the LSM6DS accelerometer. When total acceleration drops close to zero, the buzzer on GPIO 16 sounds immediately. GPIO 38 is used a..."
  },
  {
    "id": "esp-20",
    "title": "G-Force Overload Detector",
    "language": "ESP-IDF",
    "category": "Motion & IMU",
    "level": "Beginner",
    "filename": "G-Force Overload Detector.c",
    "codePath": "/advayam-codes/esp-idf/G-Force%20Overload%20Detector.c",
    "description": "This program monitors total acceleration. If acceleration exceeds 2.5g, the matrix flashes red and the buzzer produces a high-frequency warning chirp."
  },
  {
    "id": "esp-21",
    "title": "Gesture Colour Change",
    "language": "ESP-IDF",
    "category": "Input & Touch",
    "level": "Beginner",
    "filename": "Gesture Colour Change.c",
    "codePath": "/advayam-codes/esp-idf/Gesture%20Colour%20Change.c",
    "description": "This program detects a quick left-to-right flick of the board. Each detected gesture changes the RGB matrix to the next color in a predefined palette."
  },
  {
    "id": "esp-22",
    "title": "Gyro-Based Spin Meter",
    "language": "ESP-IDF",
    "category": "Motion & IMU",
    "level": "Intermediate",
    "filename": "Gyro-based Spin Meter.c",
    "codePath": "/advayam-codes/esp-idf/Gyro-based%20Spin%20Meter.c",
    "description": "This program measures rotation around the Z axis. A bright pixel travels around the matrix edge. Its speed increases as the measured angular velocity increases."
  },
  {
    "id": "esp-23",
    "title": "Hand Proximity Reaction Game",
    "language": "ESP-IDF",
    "category": "Games & Simulations",
    "level": "Advanced",
    "filename": "Hand Proximity Reaction Game.c",
    "codePath": "/advayam-codes/esp-idf/Hand%20Proximity%20Reaction%20Game.c",
    "description": "Hand Proximity Reaction Game demo project running on Advayam ESP32 development hardware using the ESP-IDF framework."
  },
  {
    "id": "esp-24",
    "title": "Inclinometer Audio Feedback",
    "language": "ESP-IDF",
    "category": "Audio & Buzzer",
    "level": "Beginner",
    "filename": "Inclinometer Audio Feedback.c",
    "codePath": "/advayam-codes/esp-idf/Inclinometer%20Audio%20Feedback.c",
    "description": "This program measures the board's pitch relative to a 45-degree target. The buzzer beeps increasingly quickly as the board approaches the target angle."
  },
  {
    "id": "esp-25",
    "title": "Laser Tripwire Alarm",
    "language": "ESP-IDF",
    "category": "Sensors & Actuators",
    "level": "Intermediate",
    "filename": "Laser Tripwire Alarm.c",
    "codePath": "/advayam-codes/esp-idf/Laser%20Tripwire%20Alarm.c",
    "description": "Laser Tripwire Alarm demo project running on Advayam ESP32 development hardware using the ESP-IDF framework."
  },
  {
    "id": "esp-26",
    "title": "LED Heartbeat",
    "language": "ESP-IDF",
    "category": "Light & Optical",
    "level": "Beginner",
    "filename": "LED Heartbeat.c",
    "codePath": "/advayam-codes/esp-idf/LED%20Heartbeat.c",
    "description": "This program simulates a human heartbeat using the User LED on GPIO 45. The LED performs two fading pulses followed by a short pause, creating a repeating heartbeat-li..."
  },
  {
    "id": "esp-27",
    "title": "LED Matrix Analog Clock Mimic",
    "language": "ESP-IDF",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "LED Matrix Analog Clock Mimic.c",
    "codePath": "/advayam-codes/esp-idf/LED%20Matrix%20Analog%20Clock%20Mimic.c",
    "description": "LED Matrix Analog Clock Mimic demo project running on Advayam ESP32 development hardware using the ESP-IDF framework."
  },
  {
    "id": "esp-28",
    "title": "LED Matrix Binary Counter",
    "language": "ESP-IDF",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "LED Matrix Binary Counter.c",
    "codePath": "/advayam-codes/esp-idf/LED%20Matrix%20Binary%20Counter.c",
    "description": "8-bit Binary Counter Each press of the button increases an 8-bit counter by 1. The first 8 LEDs of the matrix represent the binary value: - LED ON = 1 - LED DIM = 0 Th..."
  },
  {
    "id": "esp-29",
    "title": "LED Matrix Catch The Falling Fruit Game",
    "language": "ESP-IDF",
    "category": "LED Matrix",
    "level": "Advanced",
    "filename": "LED Matrix Catch the Falling Fruit Game.c",
    "codePath": "/advayam-codes/esp-idf/LED%20Matrix%20Catch%20the%20Falling%20Fruit%20Game.c",
    "description": "LED Matrix Catch The Falling Fruit Game demo project running on Advayam ESP32 development hardware using the ESP-IDF framework."
  },
  {
    "id": "esp-30",
    "title": "LED Matrix Checkerboard Strobe",
    "language": "ESP-IDF",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "LED Matrix Checkerboard Strobe.c",
    "codePath": "/advayam-codes/esp-idf/LED%20Matrix%20Checkerboard%20Strobe.c",
    "description": "This program alternates two complementary colors across the 5x5 RGB matrix in a checkerboard pattern, creating a strobe effect."
  },
  {
    "id": "esp-31",
    "title": "LED Matrix Coin Flip",
    "language": "ESP-IDF",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "LED Matrix Coin Flip.c",
    "codePath": "/advayam-codes/esp-idf/LED%20Matrix%20Coin%20Flip.c",
    "description": "Coin Flip Simulator Press the button to flip the coin. The matrix shows a spinning coin animation, then stops on HEADS or TAILS and plays a chime."
  },
  {
    "id": "esp-32",
    "title": "LED Matrix Color Mixer",
    "language": "ESP-IDF",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "LED Matrix Color Mixer.c",
    "codePath": "/advayam-codes/esp-idf/LED%20Matrix%20Color%20Mixer.c",
    "description": "This program mixes Red, Green, and Blue components on the RGB matrix using the push button on GPIO 17. Short presses cycle through 8 brightness levels. A long press co..."
  },
  {
    "id": "esp-33",
    "title": "LED Matrix Colour Temperature Simulator",
    "language": "ESP-IDF",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "LED Matrix Colour Temperature Simulator.c",
    "codePath": "/advayam-codes/esp-idf/LED%20Matrix%20Colour%20Temperature%20Simulator.c",
    "description": "LED Matrix Colour Temperature Simulator demo project running on Advayam ESP32 development hardware using the ESP-IDF framework."
  },
  {
    "id": "esp-34",
    "title": "LED Matrix Diagonal Colour Wave",
    "language": "ESP-IDF",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "LED Matrix Diagonal Colour Wave.c",
    "codePath": "/advayam-codes/esp-idf/LED%20Matrix%20Diagonal%20Colour%20Wave.c",
    "description": "This program creates a moving diagonal rainbow wave across the 5x5 RGB LED matrix connected to GPIO 7."
  },
  {
    "id": "esp-35",
    "title": "LED Matrix Digital Bubble Meter",
    "language": "ESP-IDF",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "LED Matrix Digital Bubble Meter.c",
    "codePath": "/advayam-codes/esp-idf/LED%20Matrix%20Digital%20Bubble%20Meter.c",
    "description": "This program uses the accelerometer to create a digital bubble level. The green center LED represents the bubble. The bubble moves according to board tilt and stays ce..."
  },
  {
    "id": "esp-36",
    "title": "LED Matrix Digital Dice",
    "language": "ESP-IDF",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "LED Matrix Digital Dice.c",
    "codePath": "/advayam-codes/esp-idf/LED%20Matrix%20Digital%20Dice.c",
    "description": "Digital Dice Roller Pressing the button starts an animated dice-rolling effect. After the animation, a random number from 1 to 6 is selected and displayed on the 5x5 N..."
  },
  {
    "id": "esp-37",
    "title": "LED Matrix Digital Stopwatch",
    "language": "ESP-IDF",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "LED Matrix Digital Stopwatch.c",
    "codePath": "/advayam-codes/esp-idf/LED%20Matrix%20Digital%20Stopwatch.c",
    "description": "Stopwatch on Matrix This program uses a push button to start and stop a stopwatch. The LED matrix progressively fills with one LED for each elapsed second. After 25 se..."
  },
  {
    "id": "esp-38",
    "title": "LED Matrix Expanding Color Wave",
    "language": "ESP-IDF",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "LED Matrix Expanding Color Wave.c",
    "codePath": "/advayam-codes/esp-idf/LED%20Matrix%20Expanding%20Color%20Wave.c",
    "description": "This program animates expanding square frames on the 5x5 RGB LED matrix, starting from the center and expanding toward the outer boundary."
  },
  {
    "id": "esp-39",
    "title": "LED Matrix Fire Simulation",
    "language": "ESP-IDF",
    "category": "LED Matrix",
    "level": "Advanced",
    "filename": "LED Matrix Fire Simulation.c",
    "codePath": "/advayam-codes/esp-idf/LED%20Matrix%20Fire%20Simulation.c",
    "description": "This program creates a flickering fire effect on the 5x5 RGB LED matrix using a simulated heat map. Heat is converted into red, orange, and yellow colors."
  },
  {
    "id": "esp-40",
    "title": "LED Matrix Heatmap Generator",
    "language": "ESP-IDF",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "LED Matrix Heatmap Generator.c",
    "codePath": "/advayam-codes/esp-idf/LED%20Matrix%20Heatmap%20Generator.c",
    "description": "This program displays the LMT87 temperature as a vertical heatmap on the 5x5 RGB matrix. Lower temperatures produce blue colors while higher temperatures progress thro..."
  },
  {
    "id": "esp-41",
    "title": "LED Matrix Horizontal Scanner",
    "language": "ESP-IDF",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "LED Matrix Horizontal Scanner.c",
    "codePath": "/advayam-codes/esp-idf/LED%20Matrix%20Horizontal%20Scanner.c",
    "description": "Matrix Column Scanner Each button press moves a solid vertical bar one column to the right. After the fifth column, it returns to the first. Includes software button d..."
  },
  {
    "id": "esp-42",
    "title": "LED Matrix Hourglass",
    "language": "ESP-IDF",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "LED Matrix Hourglass.c",
    "codePath": "/advayam-codes/esp-idf/LED%20Matrix%20Hourglass.c",
    "description": "5x5 Falling Sand Timer - Sand starts at the TOP row, CENTER pixel - Each grain falls downward - Grains pile up naturally - Matrix gradually fills with sand - All 25 pi..."
  },
  {
    "id": "esp-43",
    "title": "LED Matrix Maze Game",
    "language": "ESP-IDF",
    "category": "LED Matrix",
    "level": "Advanced",
    "filename": "LED Matrix Maze Game.c",
    "codePath": "/advayam-codes/esp-idf/LED%20Matrix%20Maze%20Game.c",
    "description": "LED Matrix Maze Game demo project running on Advayam ESP32 development hardware using the ESP-IDF framework."
  },
  {
    "id": "esp-44",
    "title": "LED Matrix Memory Game",
    "language": "ESP-IDF",
    "category": "LED Matrix",
    "level": "Advanced",
    "filename": "LED Matrix Memory Game.c",
    "codePath": "/advayam-codes/esp-idf/LED%20Matrix%20Memory%20Game.c",
    "description": "LED Matrix Memory Game demo project running on Advayam ESP32 development hardware using the ESP-IDF framework."
  },
  {
    "id": "esp-45",
    "title": "LED Matrix Minesweeper",
    "language": "ESP-IDF",
    "category": "LED Matrix",
    "level": "Advanced",
    "filename": "LED Matrix Minesweeper.c",
    "codePath": "/advayam-codes/esp-idf/LED%20Matrix%20Minesweeper.c",
    "description": "LED Matrix Minesweeper demo project running on Advayam ESP32 development hardware using the ESP-IDF framework."
  },
  {
    "id": "esp-46",
    "title": "LED Matrix Night Light",
    "language": "ESP-IDF",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "LED Matrix Night Light.c",
    "codePath": "/advayam-codes/esp-idf/LED%20Matrix%20Night%20Light.c",
    "description": "LED Matrix Night Light demo project running on Advayam ESP32 development hardware using the ESP-IDF framework."
  },
  {
    "id": "esp-47",
    "title": "LED Matrix NTP Scrolling Clock",
    "language": "ESP-IDF",
    "category": "LED Matrix",
    "level": "Advanced",
    "filename": "LED Matrix NTP Scrolling Clock.c",
    "codePath": "/advayam-codes/esp-idf/LED%20Matrix%20NTP%20Scrolling%20Clock.c",
    "description": "LED Matrix NTP Scrolling Clock demo project running on Advayam ESP32 development hardware using the ESP-IDF framework."
  },
  {
    "id": "esp-48",
    "title": "LED Matrix Pacman Chase Animation",
    "language": "ESP-IDF",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "LED Matrix Pacman Chase animation.c",
    "codePath": "/advayam-codes/esp-idf/LED%20Matrix%20Pacman%20Chase%20animation.c",
    "description": "LED Matrix Pacman Chase Animation demo project running on Advayam ESP32 development hardware using the ESP-IDF framework."
  },
  {
    "id": "esp-49",
    "title": "LED Matrix Perlin Noise Gradient Generator",
    "language": "ESP-IDF",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "LED Matrix Perlin Noise Gradient Generator.c",
    "codePath": "/advayam-codes/esp-idf/LED%20Matrix%20Perlin%20Noise%20Gradient%20Generator.c",
    "description": "LED Matrix Perlin Noise Gradient Generator demo project running on Advayam ESP32 development hardware using the ESP-IDF framework."
  },
  {
    "id": "esp-50",
    "title": "LED Matrix Pixel Collision Simulator",
    "language": "ESP-IDF",
    "category": "LED Matrix",
    "level": "Advanced",
    "filename": "LED Matrix Pixel Collision Simulator.c",
    "codePath": "/advayam-codes/esp-idf/LED%20Matrix%20Pixel%20Collision%20Simulator.c",
    "description": "LED Matrix Pixel Collision Simulator demo project running on Advayam ESP32 development hardware using the ESP-IDF framework."
  },
  {
    "id": "esp-51",
    "title": "LED Matrix Pixel Particle Fountain",
    "language": "ESP-IDF",
    "category": "LED Matrix",
    "level": "Advanced",
    "filename": "LED Matrix Pixel Particle Fountain.c",
    "codePath": "/advayam-codes/esp-idf/LED%20Matrix%20Pixel%20Particle%20Fountain.c",
    "description": "LED Matrix Pixel Particle Fountain demo project running on Advayam ESP32 development hardware using the ESP-IDF framework."
  },
  {
    "id": "esp-52",
    "title": "LED Matrix Pong Game",
    "language": "ESP-IDF",
    "category": "LED Matrix",
    "level": "Advanced",
    "filename": "LED Matrix Pong Game.c",
    "codePath": "/advayam-codes/esp-idf/LED%20Matrix%20Pong%20Game.c",
    "description": "LED Matrix Pong Game demo project running on Advayam ESP32 development hardware using the ESP-IDF framework."
  },
  {
    "id": "esp-53",
    "title": "LED Matrix Proximity Depth Gauge",
    "language": "ESP-IDF",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "LED Matrix Proximity Depth Gauge.c",
    "codePath": "/advayam-codes/esp-idf/LED%20Matrix%20Proximity%20Depth%20Gauge.c",
    "description": "LED Matrix Proximity Depth Gauge demo project running on Advayam ESP32 development hardware using the ESP-IDF framework."
  },
  {
    "id": "esp-54",
    "title": "LED Matrix Radar Sweep Animation",
    "language": "ESP-IDF",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "LED Matrix Radar Sweep Animation.c",
    "codePath": "/advayam-codes/esp-idf/LED%20Matrix%20Radar%20Sweep%20Animation.c",
    "description": "LED Matrix Radar Sweep Animation demo project running on Advayam ESP32 development hardware using the ESP-IDF framework."
  },
  {
    "id": "esp-55",
    "title": "LED Matrix Rhythm Game",
    "language": "ESP-IDF",
    "category": "LED Matrix",
    "level": "Advanced",
    "filename": "LED Matrix Rhythm Game.c",
    "codePath": "/advayam-codes/esp-idf/LED%20Matrix%20Rhythm%20Game.c",
    "description": "LED Matrix Rhythm Game demo project running on Advayam ESP32 development hardware using the ESP-IDF framework."
  },
  {
    "id": "esp-56",
    "title": "LED Matrix Rolling Marble Simulator",
    "language": "ESP-IDF",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "LED Matrix Rolling Marble Simulator.c",
    "codePath": "/advayam-codes/esp-idf/LED%20Matrix%20Rolling%20Marble%20Simulator.c",
    "description": "LED Matrix Rolling Marble Simulator demo project running on Advayam ESP32 development hardware using the ESP-IDF framework."
  },
  {
    "id": "esp-57",
    "title": "LED Matrix Rotating Colour Wheel",
    "language": "ESP-IDF",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "LED Matrix Rotating Colour Wheel.c",
    "codePath": "/advayam-codes/esp-idf/LED%20Matrix%20Rotating%20Colour%20Wheel.c",
    "description": "Convert HSV to RGB. hue: 0 - 65535 saturation: 0 - 255 value: 0 - 255 Output RGB: 0 - 255"
  },
  {
    "id": "esp-58",
    "title": "LED Matrix Sand Falling Animation",
    "language": "ESP-IDF",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "LED Matrix Sand Falling Animation.c",
    "codePath": "/advayam-codes/esp-idf/LED%20Matrix%20Sand%20Falling%20Animation.c",
    "description": "LED Matrix Sand Falling Animation demo project running on Advayam ESP32 development hardware using the ESP-IDF framework."
  },
  {
    "id": "esp-59",
    "title": "LED Matrix Simulated VU Meter",
    "language": "ESP-IDF",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "LED Matrix Simulated VU Meter.c",
    "codePath": "/advayam-codes/esp-idf/LED%20Matrix%20Simulated%20VU%20Meter.c",
    "description": "ESP-IDF version Simulates a five-bar audio VU meter on a 5x5 RGB matrix."
  },
  {
    "id": "esp-60",
    "title": "LED Matrix Snake Game",
    "language": "ESP-IDF",
    "category": "LED Matrix",
    "level": "Advanced",
    "filename": "LED Matrix Snake Game.c",
    "codePath": "/advayam-codes/esp-idf/LED%20Matrix%20Snake%20Game.c",
    "description": "LED Matrix Snake Game demo project running on Advayam ESP32 development hardware using the ESP-IDF framework."
  },
  {
    "id": "esp-61",
    "title": "LED Matrix Space Invaders Game",
    "language": "ESP-IDF",
    "category": "LED Matrix",
    "level": "Advanced",
    "filename": "LED Matrix Space Invaders Game.c",
    "codePath": "/advayam-codes/esp-idf/LED%20Matrix%20Space%20Invaders%20Game.c",
    "description": "LED Matrix Space Invaders Game demo project running on Advayam ESP32 development hardware using the ESP-IDF framework."
  },
  {
    "id": "esp-62",
    "title": "LED Matrix Spiral Fill",
    "language": "ESP-IDF",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "LED Matrix Spiral Fill.c",
    "codePath": "/advayam-codes/esp-idf/LED%20Matrix%20Spiral%20Fill.c",
    "description": "LED Matrix Spiral Fill demo project running on Advayam ESP32 development hardware using the ESP-IDF framework."
  },
  {
    "id": "esp-63",
    "title": "LED Matrix Spirit Level",
    "language": "ESP-IDF",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "LED Matrix Spirit Level.c",
    "codePath": "/advayam-codes/esp-idf/LED%20Matrix%20Spirit%20Level.c",
    "description": "LED Matrix Spirit Level demo project running on Advayam ESP32 development hardware using the ESP-IDF framework."
  },
  {
    "id": "esp-64",
    "title": "LED Matrix Starfield Animation",
    "language": "ESP-IDF",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "LED Matrix Starfield Animation.c",
    "codePath": "/advayam-codes/esp-idf/LED%20Matrix%20Starfield%20Animation.c",
    "description": "LED Matrix Starfield Animation demo project running on Advayam ESP32 development hardware using the ESP-IDF framework."
  },
  {
    "id": "esp-65",
    "title": "LED Matrix Strobe Light",
    "language": "ESP-IDF",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "LED Matrix Strobe Light.c",
    "codePath": "/advayam-codes/esp-idf/LED%20Matrix%20Strobe%20Light.c",
    "description": "LED Matrix Strobe Light demo project running on Advayam ESP32 development hardware using the ESP-IDF framework."
  },
  {
    "id": "esp-66",
    "title": "LED Matrix Swipe Detector",
    "language": "ESP-IDF",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "LED Matrix Swipe Detector.c",
    "codePath": "/advayam-codes/esp-idf/LED%20Matrix%20Swipe%20Detector.c",
    "description": "LED Matrix Swipe Detector demo project running on Advayam ESP32 development hardware using the ESP-IDF framework."
  },
  {
    "id": "esp-67",
    "title": "LED Matrix Temperature Controlled Dynamic Smiley Faces",
    "language": "ESP-IDF",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "LED Matrix Temperature Controlled Dynamic Smiley Faces.c",
    "codePath": "/advayam-codes/esp-idf/LED%20Matrix%20Temperature%20Controlled%20Dynamic%20Smiley%20Faces.c",
    "description": "LED Matrix Temperature Controlled Dynamic Smiley Faces demo project running on Advayam ESP32 development hardware using the ESP-IDF framework."
  },
  {
    "id": "esp-68",
    "title": "LED Matrix Temperature Controlled Fire Simulation",
    "language": "ESP-IDF",
    "category": "LED Matrix",
    "level": "Advanced",
    "filename": "LED Matrix Temperature Controlled Fire Simulation.c",
    "codePath": "/advayam-codes/esp-idf/LED%20Matrix%20Temperature%20Controlled%20Fire%20Simulation.c",
    "description": "LED Matrix Temperature Controlled Fire Simulation demo project running on Advayam ESP32 development hardware using the ESP-IDF framework."
  },
  {
    "id": "esp-69",
    "title": "LED Matrix Tetris Drop Animation",
    "language": "ESP-IDF",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "LED Matrix Tetris Drop Animation.c",
    "codePath": "/advayam-codes/esp-idf/LED%20Matrix%20Tetris%20Drop%20Animation.c",
    "description": "LED Matrix Tetris Drop Animation demo project running on Advayam ESP32 development hardware using the ESP-IDF framework."
  },
  {
    "id": "esp-70",
    "title": "LED Matrix Text Scroller",
    "language": "ESP-IDF",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "LED Matrix Text Scroller.c",
    "codePath": "/advayam-codes/esp-idf/LED%20Matrix%20Text%20Scroller.c",
    "description": "LED Matrix Text Scroller demo project running on Advayam ESP32 development hardware using the ESP-IDF framework."
  },
  {
    "id": "esp-71",
    "title": "LED Matrix Toggle State",
    "language": "ESP-IDF",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "LED Matrix Toggle State.c",
    "codePath": "/advayam-codes/esp-idf/LED%20Matrix%20Toggle%20State.c",
    "description": "LED Matrix Toggle State demo project running on Advayam ESP32 development hardware using the ESP-IDF framework."
  },
  {
    "id": "esp-72",
    "title": "LED Matrix Touchless Trashcan Mimic",
    "language": "ESP-IDF",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "LED Matrix Touchless Trashcan Mimic.c",
    "codePath": "/advayam-codes/esp-idf/LED%20Matrix%20Touchless%20Trashcan%20Mimic.c",
    "description": "LED Matrix Touchless Trashcan Mimic demo project running on Advayam ESP32 development hardware using the ESP-IDF framework."
  },
  {
    "id": "esp-73",
    "title": "LED Matrix Tug Of War Game",
    "language": "ESP-IDF",
    "category": "LED Matrix",
    "level": "Advanced",
    "filename": "LED Matrix Tug of War Game.c",
    "codePath": "/advayam-codes/esp-idf/LED%20Matrix%20Tug%20of%20War%20Game.c",
    "description": "LED Matrix Tug Of War Game demo project running on Advayam ESP32 development hardware using the ESP-IDF framework."
  },
  {
    "id": "esp-74",
    "title": "LED Matrix Vertical Scanner",
    "language": "ESP-IDF",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "LED Matrix Vertical Scanner.c",
    "codePath": "/advayam-codes/esp-idf/LED%20Matrix%20Vertical%20Scanner.c",
    "description": "LED Matrix Vertical Scanner demo project running on Advayam ESP32 development hardware using the ESP-IDF framework."
  },
  {
    "id": "esp-75",
    "title": "LED Matrix Whack-A-Mole",
    "language": "ESP-IDF",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "LED Matrix Whack-a-Mole.c",
    "codePath": "/advayam-codes/esp-idf/LED%20Matrix%20Whack-a-Mole.c",
    "description": "LED Matrix Whack-A-Mole demo project running on Advayam ESP32 development hardware using the ESP-IDF framework."
  },
  {
    "id": "esp-76",
    "title": "LED Matrix WiFi Signal Strength Indicator",
    "language": "ESP-IDF",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "LED Matrix WiFi Signal Strength Indicator.c",
    "codePath": "/advayam-codes/esp-idf/LED%20Matrix%20WiFi%20Signal%20Strength%20Indicator.c",
    "description": "LED Matrix WiFi Signal Strength Indicator demo project running on Advayam ESP32 development hardware using the ESP-IDF framework."
  },
  {
    "id": "esp-77",
    "title": "Matrix Rain Effect",
    "language": "ESP-IDF",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "Matrix Rain Effect.c",
    "codePath": "/advayam-codes/esp-idf/Matrix%20Rain%20Effect.c",
    "description": "Matrix Rain Effect demo project running on Advayam ESP32 development hardware using the ESP-IDF framework."
  },
  {
    "id": "esp-78",
    "title": "Overheat Alarm",
    "language": "ESP-IDF",
    "category": "Thermal & Climate",
    "level": "Intermediate",
    "filename": "Overheat Alarm.c",
    "codePath": "/advayam-codes/esp-idf/Overheat%20Alarm.c",
    "description": "Overheat Alarm demo project running on Advayam ESP32 development hardware using the ESP-IDF framework."
  },
  {
    "id": "esp-79",
    "title": "Proximity Based Greeting",
    "language": "ESP-IDF",
    "category": "Input & Touch",
    "level": "Beginner",
    "filename": "Proximity Based Greeting.c",
    "codePath": "/advayam-codes/esp-idf/Proximity%20Based%20Greeting.c",
    "description": "Proximity Based Greeting demo project running on Advayam ESP32 development hardware using the ESP-IDF framework."
  },
  {
    "id": "esp-80",
    "title": "Proximity Counter",
    "language": "ESP-IDF",
    "category": "Input & Touch",
    "level": "Beginner",
    "filename": "Proximity Counter.c",
    "codePath": "/advayam-codes/esp-idf/Proximity%20Counter.c",
    "description": "Proximity Counter demo project running on Advayam ESP32 development hardware using the ESP-IDF framework."
  },
  {
    "id": "esp-81",
    "title": "Proximity Theremin",
    "language": "ESP-IDF",
    "category": "Audio & Buzzer",
    "level": "Intermediate",
    "filename": "Proximity Theremin.c",
    "codePath": "/advayam-codes/esp-idf/Proximity%20Theremin.c",
    "description": "Proximity Theremin demo project running on Advayam ESP32 development hardware using the ESP-IDF framework."
  },
  {
    "id": "esp-82",
    "title": "Reaction Time Game",
    "language": "ESP-IDF",
    "category": "Games & Simulations",
    "level": "Advanced",
    "filename": "Reaction Time Game.c",
    "codePath": "/advayam-codes/esp-idf/Reaction%20Time%20Game.c",
    "description": "Reaction Time Game demo project running on Advayam ESP32 development hardware using the ESP-IDF framework."
  },
  {
    "id": "esp-83",
    "title": "Roll And Pitch Reader",
    "language": "ESP-IDF",
    "category": "Audio & Buzzer",
    "level": "Beginner",
    "filename": "Roll and Pitch Reader.c",
    "codePath": "/advayam-codes/esp-idf/Roll%20and%20Pitch%20Reader.c",
    "description": "Roll And Pitch Reader demo project running on Advayam ESP32 development hardware using the ESP-IDF framework."
  },
  {
    "id": "esp-84",
    "title": "Shadow Puppetry Brightness Detector",
    "language": "ESP-IDF",
    "category": "Light & Optical",
    "level": "Beginner",
    "filename": "Shadow Puppetry Brightness Detector.c",
    "codePath": "/advayam-codes/esp-idf/Shadow%20Puppetry%20Brightness%20Detector.c",
    "description": "Shadow Puppetry Brightness Detector demo project running on Advayam ESP32 development hardware using the ESP-IDF framework."
  },
  {
    "id": "esp-85",
    "title": "Shake To Shuffle Matrix Randomizer",
    "language": "ESP-IDF",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "Shake to Shuffle Matrix Randomizer.c",
    "codePath": "/advayam-codes/esp-idf/Shake%20to%20Shuffle%20Matrix%20Randomizer.c",
    "description": "Shake To Shuffle Matrix Randomizer demo project running on Advayam ESP32 development hardware using the ESP-IDF framework."
  },
  {
    "id": "esp-86",
    "title": "Six Orientation Detector",
    "language": "ESP-IDF",
    "category": "Motion & IMU",
    "level": "Beginner",
    "filename": "Six Orientation Detector.c",
    "codePath": "/advayam-codes/esp-idf/Six%20Orientation%20Detector.c",
    "description": "Six Orientation Detector demo project running on Advayam ESP32 development hardware using the ESP-IDF framework."
  },
  {
    "id": "esp-87",
    "title": "Step Counter",
    "language": "ESP-IDF",
    "category": "Motion & IMU",
    "level": "Beginner",
    "filename": "Step Counter.c",
    "codePath": "/advayam-codes/esp-idf/Step%20Counter.c",
    "description": "Step Counter demo project running on Advayam ESP32 development hardware using the ESP-IDF framework."
  },
  {
    "id": "esp-88",
    "title": "Temperature High-Low Latch",
    "language": "ESP-IDF",
    "category": "Thermal & Climate",
    "level": "Intermediate",
    "filename": "Temperature High-Low Latch.c",
    "codePath": "/advayam-codes/esp-idf/Temperature%20High-Low%20Latch.c",
    "description": "Temperature High-Low Latch demo project running on Advayam ESP32 development hardware using the ESP-IDF framework."
  },
  {
    "id": "esp-89",
    "title": "Temperature Reading Comparison",
    "language": "ESP-IDF",
    "category": "Thermal & Climate",
    "level": "Beginner",
    "filename": "Temperature Reading Comparison.c",
    "codePath": "/advayam-codes/esp-idf/Temperature%20Reading%20Comparison.c",
    "description": "Temperature Reading Comparison demo project running on Advayam ESP32 development hardware using the ESP-IDF framework."
  },
  {
    "id": "esp-90",
    "title": "Temperature To Pitch Converter",
    "language": "ESP-IDF",
    "category": "Audio & Buzzer",
    "level": "Beginner",
    "filename": "Temperature to Pitch Converter.c",
    "codePath": "/advayam-codes/esp-idf/Temperature%20to%20Pitch%20Converter.c",
    "description": "Temperature To Pitch Converter demo project running on Advayam ESP32 development hardware using the ESP-IDF framework."
  },
  {
    "id": "esp-91",
    "title": "Tilt Balance Stability Game",
    "language": "ESP-IDF",
    "category": "Games & Simulations",
    "level": "Advanced",
    "filename": "Tilt Balance Stability Game.c",
    "codePath": "/advayam-codes/esp-idf/Tilt%20Balance%20Stability%20Game.c",
    "description": "Tilt Balance Stability Game demo project running on Advayam ESP32 development hardware using the ESP-IDF framework."
  },
  {
    "id": "esp-92",
    "title": "Tilt Controlled Dice Roller",
    "language": "ESP-IDF",
    "category": "Games & Simulations",
    "level": "Beginner",
    "filename": "Tilt Controlled Dice Roller.c",
    "codePath": "/advayam-codes/esp-idf/Tilt%20Controlled%20Dice%20Roller.c",
    "description": "Tilt Controlled Dice Roller demo project running on Advayam ESP32 development hardware using the ESP-IDF framework."
  },
  {
    "id": "esp-93",
    "title": "Tilt Controlled Maze Game",
    "language": "ESP-IDF",
    "category": "Games & Simulations",
    "level": "Advanced",
    "filename": "Tilt Controlled Maze Game.c",
    "codePath": "/advayam-codes/esp-idf/Tilt%20Controlled%20Maze%20Game.c",
    "description": "Tilt Controlled Maze Game demo project running on Advayam ESP32 development hardware using the ESP-IDF framework."
  },
  {
    "id": "esp-94",
    "title": "Tilt To Sound Synthesizer",
    "language": "ESP-IDF",
    "category": "Audio & Buzzer",
    "level": "Beginner",
    "filename": "Tilt to Sound Synthesizer.c",
    "codePath": "/advayam-codes/esp-idf/Tilt%20to%20Sound%20Synthesizer.c",
    "description": "Tilt To Sound Synthesizer demo project running on Advayam ESP32 development hardware using the ESP-IDF framework."
  },
  {
    "id": "esp-95",
    "title": "Touchless Page Turner",
    "language": "ESP-IDF",
    "category": "Input & Touch",
    "level": "Beginner",
    "filename": "Touchless Page Turner.c",
    "codePath": "/advayam-codes/esp-idf/Touchless%20Page%20Turner.c",
    "description": "Touchless Page Turner demo project running on Advayam ESP32 development hardware using the ESP-IDF framework."
  },
  {
    "id": "esp-96",
    "title": "Touchless Proximity Switch",
    "language": "ESP-IDF",
    "category": "Input & Touch",
    "level": "Beginner",
    "filename": "Touchless Proximity Switch.c",
    "codePath": "/advayam-codes/esp-idf/Touchless%20Proximity%20Switch.c",
    "description": "Touchless Proximity Switch demo project running on Advayam ESP32 development hardware using the ESP-IDF framework."
  },
  {
    "id": "esp-97",
    "title": "Touchless Volume Control Knob",
    "language": "ESP-IDF",
    "category": "Input & Touch",
    "level": "Beginner",
    "filename": "Touchless Volume Control Knob.c",
    "codePath": "/advayam-codes/esp-idf/Touchless%20Volume%20Control%20Knob.c",
    "description": "Touchless Volume Control Knob demo project running on Advayam ESP32 development hardware using the ESP-IDF framework."
  },
  {
    "id": "rtos-1",
    "title": "Ambient Lux Data Logger",
    "language": "FreeRTOS",
    "category": "Light & Optical",
    "level": "Advanced",
    "filename": "Ambient Lux Data Logger.ino",
    "codePath": "/advayam-codes/freertos/Ambient%20Lux%20Data%20Logger.ino",
    "description": "Ambient Lux Data Logger demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "rtos-2",
    "title": "Anti-Theft Alarm",
    "language": "FreeRTOS",
    "category": "Sensors & Actuators",
    "level": "Advanced",
    "filename": "Anti-Theft Alarm.ino",
    "codePath": "/advayam-codes/freertos/Anti-Theft%20Alarm.ino",
    "description": "Anti-Theft Alarm demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "rtos-3",
    "title": "Auto Dimming Matrix Display",
    "language": "FreeRTOS",
    "category": "LED Matrix",
    "level": "Advanced",
    "filename": "Auto Dimming Matrix Display.ino",
    "codePath": "/advayam-codes/freertos/Auto%20Dimming%20Matrix%20Display.ino",
    "description": "Auto Dimming Matrix Display demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "rtos-4",
    "title": "Automatic Text Orientation And Scrolling",
    "language": "FreeRTOS",
    "category": "Motion & IMU",
    "level": "Advanced",
    "filename": "Automatic Text Orientation and Scrolling.ino",
    "codePath": "/advayam-codes/freertos/Automatic%20Text%20Orientation%20and%20Scrolling.ino",
    "description": "Automatic Text Orientation And Scrolling demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "rtos-5",
    "title": "Blind Spot Proximity Warning",
    "language": "FreeRTOS",
    "category": "Games & Simulations",
    "level": "Advanced",
    "filename": "Blind Spot Proximity Warning.ino",
    "codePath": "/advayam-codes/freertos/Blind%20Spot%20Proximity%20Warning.ino",
    "description": "Blind Spot Proximity Warning demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "rtos-6",
    "title": "Button Click Counter",
    "language": "FreeRTOS",
    "category": "Input & Touch",
    "level": "Advanced",
    "filename": "Button Click Counter.ino",
    "codePath": "/advayam-codes/freertos/Button%20Click%20Counter.ino",
    "description": "Button Click Counter demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "rtos-7",
    "title": "Button Hold Velocity Accelerometer",
    "language": "FreeRTOS",
    "category": "Input & Touch",
    "level": "Advanced",
    "filename": "Button Hold Velocity Accelerometer.ino",
    "codePath": "/advayam-codes/freertos/Button%20Hold%20Velocity%20Accelerometer.ino",
    "description": "Button Hold Velocity Accelerometer demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "rtos-8",
    "title": "Button Long Press Factory Reset Demo",
    "language": "FreeRTOS",
    "category": "Input & Touch",
    "level": "Advanced",
    "filename": "Button Long Press Factory Reset Demo.ino",
    "codePath": "/advayam-codes/freertos/Button%20Long%20Press%20Factory%20Reset%20Demo.ino",
    "description": "Button Long Press Factory Reset Demo demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "rtos-9",
    "title": "Button Press SOS Message",
    "language": "FreeRTOS",
    "category": "Input & Touch",
    "level": "Advanced",
    "filename": "Button Press SOS Message.ino",
    "codePath": "/advayam-codes/freertos/Button%20Press%20SOS%20Message.ino",
    "description": "Button Press SOS Message demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "rtos-10",
    "title": "Buzzer Tone Pitch Ladder",
    "language": "FreeRTOS",
    "category": "Audio & Buzzer",
    "level": "Advanced",
    "filename": "Buzzer Tone Pitch Ladder.ino",
    "codePath": "/advayam-codes/freertos/Buzzer%20Tone%20Pitch%20Ladder.ino",
    "description": "Buzzer Tone Pitch Ladder demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "rtos-11",
    "title": "Catch The Falling Fruit",
    "language": "FreeRTOS",
    "category": "Motion & IMU",
    "level": "Advanced",
    "filename": "Catch the Falling Fruit.ino",
    "codePath": "/advayam-codes/freertos/Catch%20the%20Falling%20Fruit.ino",
    "description": "Catch The Falling Fruit demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "rtos-12",
    "title": "Coin Flip Simulator",
    "language": "FreeRTOS",
    "category": "Games & Simulations",
    "level": "Advanced",
    "filename": "Coin Flip Simulator.ino",
    "codePath": "/advayam-codes/freertos/Coin%20Flip%20Simulator.ino",
    "description": "Coin Flip Simulator demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "rtos-13",
    "title": "Colour Temperature Simulator With Automatic Brightness",
    "language": "FreeRTOS",
    "category": "Light & Optical",
    "level": "Advanced",
    "filename": "Colour Temperature Simulator with Automatic Brightness.ino",
    "codePath": "/advayam-codes/freertos/Colour%20Temperature%20Simulator%20with%20Automatic%20Brightness.ino",
    "description": "Colour Temperature Simulator With Automatic Brightness demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "rtos-14",
    "title": "Daylight Exposure Meter",
    "language": "FreeRTOS",
    "category": "Light & Optical",
    "level": "Advanced",
    "filename": "Daylight Exposure Meter.ino",
    "codePath": "/advayam-codes/freertos/Daylight%20Exposure%20Meter.ino",
    "description": "Daylight Exposure Meter demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "rtos-15",
    "title": "Digital Bubble Meter",
    "language": "FreeRTOS",
    "category": "Motion & IMU",
    "level": "Advanced",
    "filename": "Digital Bubble Meter.ino",
    "codePath": "/advayam-codes/freertos/Digital%20Bubble%20Meter.ino",
    "description": "Digital Bubble Meter demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "rtos-16",
    "title": "Double Tap Unlock",
    "language": "FreeRTOS",
    "category": "Input & Touch",
    "level": "Advanced",
    "filename": "Double Tap Unlock.ino",
    "codePath": "/advayam-codes/freertos/Double%20Tap%20Unlock.ino",
    "description": "Double Tap Unlock demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "rtos-17",
    "title": "Earthquake Detector",
    "language": "FreeRTOS",
    "category": "Motion & IMU",
    "level": "Advanced",
    "filename": "Earthquake Detector.ino",
    "codePath": "/advayam-codes/freertos/Earthquake%20Detector.ino",
    "description": "This program monitors acceleration from the LSM6DSOX and detects strong mechanical vibration. Strong vibrations trigger the buzzer on GPIO 16. The buzzer uses ESP32 LE..."
  },
  {
    "id": "rtos-18",
    "title": "ESP32 Hardware Watchdog Timer Toggle",
    "language": "FreeRTOS",
    "category": "Games & Simulations",
    "level": "Advanced",
    "filename": "ESP32 Hardware Watchdog Timer Toggle.ino",
    "codePath": "/advayam-codes/freertos/ESP32%20Hardware%20Watchdog%20Timer%20Toggle.ino",
    "description": "This program uses the ESP32 Task Watchdog Timer to monitor the main loop. The User LED on GPIO 45 toggles whenever the loop executes successfully and feeds the watchdog."
  },
  {
    "id": "rtos-19",
    "title": "Flick Colour Changer",
    "language": "FreeRTOS",
    "category": "Sensors & Actuators",
    "level": "Advanced",
    "filename": "Flick Colour Changer.ino",
    "codePath": "/advayam-codes/freertos/Flick%20Colour%20Changer.ino",
    "description": "This program detects a quick flick of the board. Each detected gesture changes the RGB matrix to the next color in a predefined palette. The original gesture condition..."
  },
  {
    "id": "rtos-20",
    "title": "Flip To Mute",
    "language": "FreeRTOS",
    "category": "Sensors & Actuators",
    "level": "Advanced",
    "filename": "Flip to Mute.ino",
    "codePath": "/advayam-codes/freertos/Flip%20to%20Mute.ino",
    "description": "This program demonstrates flip-to-mute behavior. A buzzer alarm runs until the board is turned face down. Face-down orientation immediately silences the buzzer. The bu..."
  },
  {
    "id": "rtos-21",
    "title": "Free Fall Alarm",
    "language": "FreeRTOS",
    "category": "Motion & IMU",
    "level": "Advanced",
    "filename": "Free Fall Alarm.ino",
    "codePath": "/advayam-codes/freertos/Free%20Fall%20Alarm.ino",
    "description": "This program detects free-fall using the LSM6DS accelerometer. When total acceleration drops close to zero, the buzzer on GPIO 16 sounds. GPIO 38 is retained as the IM..."
  },
  {
    "id": "rtos-22",
    "title": "G-Force Overload Detector",
    "language": "FreeRTOS",
    "category": "Motion & IMU",
    "level": "Advanced",
    "filename": "G-Force Overload Detector.ino",
    "codePath": "/advayam-codes/freertos/G-Force%20Overload%20Detector.ino",
    "description": "This program monitors total acceleration. If acceleration exceeds 2.5g, the matrix flashes red and the buzzer produces a high-frequency warning chirp. The buzzer uses ..."
  },
  {
    "id": "rtos-23",
    "title": "Gyro Based Spin Meter",
    "language": "FreeRTOS",
    "category": "Motion & IMU",
    "level": "Advanced",
    "filename": "Gyro Based Spin Meter.ino",
    "codePath": "/advayam-codes/freertos/Gyro%20Based%20Spin%20Meter.ino",
    "description": "This program measures rotation around the Z axis. A bright pixel travels around the matrix edge. Its speed increases as the measured angular velocity increases."
  },
  {
    "id": "rtos-24",
    "title": "Hand Proximity Reaction Game",
    "language": "FreeRTOS",
    "category": "Games & Simulations",
    "level": "Advanced",
    "filename": "Hand Proximity Reaction Game.ino",
    "codePath": "/advayam-codes/freertos/Hand%20Proximity%20Reaction%20Game.ino",
    "description": "Hand Proximity Reaction Game demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "rtos-25",
    "title": "Inclinometer With Pitch Feedback",
    "language": "FreeRTOS",
    "category": "Audio & Buzzer",
    "level": "Advanced",
    "filename": "Inclinometer with Pitch Feedback.ino",
    "codePath": "/advayam-codes/freertos/Inclinometer%20with%20Pitch%20Feedback.ino",
    "description": "Inclinometer With Pitch Feedback demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "rtos-26",
    "title": "LED Heartbeat Simulation",
    "language": "FreeRTOS",
    "category": "Light & Optical",
    "level": "Advanced",
    "filename": "LED Heartbeat Simulation.ino",
    "codePath": "/advayam-codes/freertos/LED%20Heartbeat%20Simulation.ino",
    "description": "LED Heartbeat Simulation demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "rtos-27",
    "title": "LED Matrix 8-Bit Binary Counter",
    "language": "FreeRTOS",
    "category": "LED Matrix",
    "level": "Advanced",
    "filename": "LED Matrix 8-bit Binary Counter.ino",
    "codePath": "/advayam-codes/freertos/LED%20Matrix%208-bit%20Binary%20Counter.ino",
    "description": "LED Matrix 8-Bit Binary Counter demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "rtos-28",
    "title": "LED Matrix Analog Clock Simulation",
    "language": "FreeRTOS",
    "category": "LED Matrix",
    "level": "Advanced",
    "filename": "LED Matrix Analog Clock Simulation.ino",
    "codePath": "/advayam-codes/freertos/LED%20Matrix%20Analog%20Clock%20Simulation.ino",
    "description": "LED Matrix Analog Clock Simulation demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "rtos-29",
    "title": "LED Matrix Checkerboard Strobe",
    "language": "FreeRTOS",
    "category": "LED Matrix",
    "level": "Advanced",
    "filename": "LED Matrix Checkerboard Strobe.ino",
    "codePath": "/advayam-codes/freertos/LED%20Matrix%20Checkerboard%20Strobe.ino",
    "description": "LED Matrix Checkerboard Strobe demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "rtos-30",
    "title": "LED Matrix Colour Temp Simulator",
    "language": "FreeRTOS",
    "category": "LED Matrix",
    "level": "Advanced",
    "filename": "LED Matrix Colour Temp Simulator.ino",
    "codePath": "/advayam-codes/freertos/LED%20Matrix%20Colour%20Temp%20Simulator.ino",
    "description": "LED Matrix Colour Temp Simulator demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "rtos-31",
    "title": "LED Matrix Colour Wave",
    "language": "FreeRTOS",
    "category": "LED Matrix",
    "level": "Advanced",
    "filename": "LED Matrix Colour Wave.ino",
    "codePath": "/advayam-codes/freertos/LED%20Matrix%20Colour%20Wave.ino",
    "description": "LED Matrix Colour Wave demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "rtos-32",
    "title": "LED Matrix Colour Wheel",
    "language": "FreeRTOS",
    "category": "LED Matrix",
    "level": "Advanced",
    "filename": "LED Matrix Colour Wheel.ino",
    "codePath": "/advayam-codes/freertos/LED%20Matrix%20Colour%20Wheel.ino",
    "description": "LED Matrix Colour Wheel demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "rtos-33",
    "title": "LED Matrix Column Scanner",
    "language": "FreeRTOS",
    "category": "LED Matrix",
    "level": "Advanced",
    "filename": "LED Matrix Column Scanner.ino",
    "codePath": "/advayam-codes/freertos/LED%20Matrix%20Column%20Scanner.ino",
    "description": "Matrix Column Scanner - FreeRTOS Version Each button press moves a solid vertical bar one column to the right. After the fifth column, it returns to the first. Include..."
  },
  {
    "id": "rtos-34",
    "title": "LED Matrix Conway'S Game Of Life",
    "language": "FreeRTOS",
    "category": "LED Matrix",
    "level": "Advanced",
    "filename": "LED Matrix Conway's Game of Life.ino",
    "codePath": "/advayam-codes/freertos/LED%20Matrix%20Conway's%20Game%20of%20Life.ino",
    "description": "LED Matrix Conway'S Game Of Life demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "rtos-35",
    "title": "LED Matrix Digital Dice Roller",
    "language": "FreeRTOS",
    "category": "LED Matrix",
    "level": "Advanced",
    "filename": "LED Matrix Digital Dice Roller.ino",
    "codePath": "/advayam-codes/freertos/LED%20Matrix%20Digital%20Dice%20Roller.ino",
    "description": "LED Matrix Digital Dice Roller demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "rtos-36",
    "title": "LED Matrix Digital Spirit Level",
    "language": "FreeRTOS",
    "category": "LED Matrix",
    "level": "Advanced",
    "filename": "LED Matrix Digital Spirit Level.ino",
    "codePath": "/advayam-codes/freertos/LED%20Matrix%20Digital%20Spirit%20Level.ino",
    "description": "This program provides a precision digital spirit level using an IMU task running on FreeRTOS. Green means the board is level. Yellow indicates a small error and red in..."
  },
  {
    "id": "rtos-37",
    "title": "LED Matrix Expanding Colour Wave",
    "language": "FreeRTOS",
    "category": "LED Matrix",
    "level": "Advanced",
    "filename": "LED Matrix Expanding Colour Wave.ino",
    "codePath": "/advayam-codes/freertos/LED%20Matrix%20Expanding%20Colour%20Wave.ino",
    "description": "Expanding Square Frames - FreeRTOS Version This program animates expanding square frames on the 5x5 RGB LED matrix, starting from the center and expanding toward the o..."
  },
  {
    "id": "rtos-38",
    "title": "LED Matrix Fire Simulation",
    "language": "FreeRTOS",
    "category": "LED Matrix",
    "level": "Advanced",
    "filename": "LED Matrix Fire Simulation.ino",
    "codePath": "/advayam-codes/freertos/LED%20Matrix%20Fire%20Simulation.ino",
    "description": "Fire Effect - FreeRTOS Version This program creates a flickering fire effect on the 5x5 RGB LED matrix using a simulated heat map. Heat is converted into red, orange, ..."
  },
  {
    "id": "rtos-39",
    "title": "LED Matrix Hourglass",
    "language": "FreeRTOS",
    "category": "LED Matrix",
    "level": "Advanced",
    "filename": "LED Matrix Hourglass.ino",
    "codePath": "/advayam-codes/freertos/LED%20Matrix%20Hourglass.ino",
    "description": "5x5 Falling Sand Timer - FreeRTOS - Sand starts at the TOP row, CENTER pixel - Each grain falls downward - Grains pile up naturally - Matrix gradually fills with sand ..."
  },
  {
    "id": "rtos-40",
    "title": "LED Matrix Laser Tripwire",
    "language": "FreeRTOS",
    "category": "LED Matrix",
    "level": "Advanced",
    "filename": "LED Matrix Laser Tripwire.ino",
    "codePath": "/advayam-codes/freertos/LED%20Matrix%20Laser%20Tripwire.ino",
    "description": "LED Matrix Laser Tripwire demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "rtos-41",
    "title": "LED Matrix Maze Game",
    "language": "FreeRTOS",
    "category": "LED Matrix",
    "level": "Advanced",
    "filename": "LED Matrix Maze Game.ino",
    "codePath": "/advayam-codes/freertos/LED%20Matrix%20Maze%20Game.ino",
    "description": "LED Matrix Maze Game demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "rtos-42",
    "title": "LED Matrix Minesweeper",
    "language": "FreeRTOS",
    "category": "LED Matrix",
    "level": "Advanced",
    "filename": "LED Matrix Minesweeper.ino",
    "codePath": "/advayam-codes/freertos/LED%20Matrix%20Minesweeper.ino",
    "description": "LED Matrix Minesweeper demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "rtos-43",
    "title": "LED Matrix Night Light",
    "language": "FreeRTOS",
    "category": "LED Matrix",
    "level": "Advanced",
    "filename": "LED Matrix Night Light.ino",
    "codePath": "/advayam-codes/freertos/LED%20Matrix%20Night%20Light.ino",
    "description": "LED Matrix Night Light demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "rtos-44",
    "title": "LED Matrix NTP Clock",
    "language": "FreeRTOS",
    "category": "LED Matrix",
    "level": "Advanced",
    "filename": "LED Matrix NTP Clock.ino",
    "codePath": "/advayam-codes/freertos/LED%20Matrix%20NTP%20Clock.ino",
    "description": "LED Matrix NTP Clock demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "rtos-45",
    "title": "LED Matrix Pacman Chase Animation",
    "language": "FreeRTOS",
    "category": "LED Matrix",
    "level": "Advanced",
    "filename": "LED Matrix Pacman Chase Animation.ino",
    "codePath": "/advayam-codes/freertos/LED%20Matrix%20Pacman%20Chase%20Animation.ino",
    "description": "LED Matrix Pacman Chase Animation demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "rtos-46",
    "title": "LED Matrix Patricle Fountain",
    "language": "FreeRTOS",
    "category": "LED Matrix",
    "level": "Advanced",
    "filename": "LED Matrix Patricle Fountain.ino",
    "codePath": "/advayam-codes/freertos/LED%20Matrix%20Patricle%20Fountain.ino",
    "description": "LED Matrix Patricle Fountain demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "rtos-47",
    "title": "LED Matrix Perlin Noise Gradient Generator",
    "language": "FreeRTOS",
    "category": "LED Matrix",
    "level": "Advanced",
    "filename": "LED Matrix Perlin Noise Gradient Generator.ino",
    "codePath": "/advayam-codes/freertos/LED%20Matrix%20Perlin%20Noise%20Gradient%20Generator.ino",
    "description": "LED Matrix Perlin Noise Gradient Generator demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "rtos-48",
    "title": "LED Matrix Pixel Collision Simulator",
    "language": "FreeRTOS",
    "category": "LED Matrix",
    "level": "Advanced",
    "filename": "LED Matrix Pixel Collision Simulator.ino",
    "codePath": "/advayam-codes/freertos/LED%20Matrix%20Pixel%20Collision%20Simulator.ino",
    "description": "LED Matrix Pixel Collision Simulator demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "rtos-49",
    "title": "LED Matrix Pong Game",
    "language": "FreeRTOS",
    "category": "LED Matrix",
    "level": "Advanced",
    "filename": "LED Matrix Pong Game.ino",
    "codePath": "/advayam-codes/freertos/LED%20Matrix%20Pong%20Game.ino",
    "description": "LED Matrix Pong Game demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "rtos-50",
    "title": "LED Matrix Radar Sweep Animation",
    "language": "FreeRTOS",
    "category": "LED Matrix",
    "level": "Advanced",
    "filename": "LED Matrix Radar Sweep Animation.ino",
    "codePath": "/advayam-codes/freertos/LED%20Matrix%20Radar%20Sweep%20Animation.ino",
    "description": "LED Matrix Radar Sweep Animation demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "rtos-51",
    "title": "LED Matrix Rhythm Game",
    "language": "FreeRTOS",
    "category": "LED Matrix",
    "level": "Advanced",
    "filename": "LED Matrix Rhythm Game.ino",
    "codePath": "/advayam-codes/freertos/LED%20Matrix%20Rhythm%20Game.ino",
    "description": "LED Matrix Rhythm Game demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "rtos-52",
    "title": "LED Matrix Row Scanner",
    "language": "FreeRTOS",
    "category": "LED Matrix",
    "level": "Advanced",
    "filename": "LED Matrix Row Scanner.ino",
    "codePath": "/advayam-codes/freertos/LED%20Matrix%20Row%20Scanner.ino",
    "description": "LED Matrix Row Scanner demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "rtos-53",
    "title": "LED Matrix Simon Says Memory Game",
    "language": "FreeRTOS",
    "category": "LED Matrix",
    "level": "Advanced",
    "filename": "LED Matrix Simon Says Memory Game.ino",
    "codePath": "/advayam-codes/freertos/LED%20Matrix%20Simon%20Says%20Memory%20Game.ino",
    "description": "LED Matrix Simon Says Memory Game demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "rtos-54",
    "title": "LED Matrix Simulated VU Meter",
    "language": "FreeRTOS",
    "category": "LED Matrix",
    "level": "Advanced",
    "filename": "LED Matrix Simulated VU Meter.ino",
    "codePath": "/advayam-codes/freertos/LED%20Matrix%20Simulated%20VU%20Meter.ino",
    "description": "LED Matrix Simulated VU Meter demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "rtos-55",
    "title": "LED Matrix Snake Game",
    "language": "FreeRTOS",
    "category": "LED Matrix",
    "level": "Advanced",
    "filename": "LED Matrix Snake Game.ino",
    "codePath": "/advayam-codes/freertos/LED%20Matrix%20Snake%20Game.ino",
    "description": "LED Matrix Snake Game demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "rtos-56",
    "title": "LED Matrix Space Invaders Game",
    "language": "FreeRTOS",
    "category": "LED Matrix",
    "level": "Advanced",
    "filename": "LED Matrix Space Invaders Game.ino",
    "codePath": "/advayam-codes/freertos/LED%20Matrix%20Space%20Invaders%20Game.ino",
    "description": "LED Matrix Space Invaders Game demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "rtos-57",
    "title": "LED Matrix Spiral Fill Animation",
    "language": "FreeRTOS",
    "category": "LED Matrix",
    "level": "Advanced",
    "filename": "LED Matrix Spiral Fill Animation.ino",
    "codePath": "/advayam-codes/freertos/LED%20Matrix%20Spiral%20Fill%20Animation.ino",
    "description": "This program illuminates every pixel of the 5x5 RGB matrix one-by-one in a clockwise spiral from the outside toward the center using a FreeRTOS task."
  },
  {
    "id": "rtos-58",
    "title": "LED Matrix Starfield Animation",
    "language": "FreeRTOS",
    "category": "LED Matrix",
    "level": "Advanced",
    "filename": "LED Matrix Starfield Animation.ino",
    "codePath": "/advayam-codes/freertos/LED%20Matrix%20Starfield%20Animation.ino",
    "description": "LED Matrix Starfield Animation demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "rtos-59",
    "title": "LED Matrix Stopwatch",
    "language": "FreeRTOS",
    "category": "LED Matrix",
    "level": "Advanced",
    "filename": "LED Matrix Stopwatch.ino",
    "codePath": "/advayam-codes/freertos/LED%20Matrix%20Stopwatch.ino",
    "description": "Stopwatch on Matrix - FreeRTOS Version This program uses a push button to start and stop a stopwatch. The LED matrix progressively fills with one LED for each elapsed ..."
  },
  {
    "id": "rtos-60",
    "title": "LED Matrix Strobe Light",
    "language": "FreeRTOS",
    "category": "LED Matrix",
    "level": "Advanced",
    "filename": "LED Matrix Strobe Light.ino",
    "codePath": "/advayam-codes/freertos/LED%20Matrix%20Strobe%20Light.ino",
    "description": "LED Matrix Strobe Light (FreeRTOS Version) This program uses a touch sensor connected to GPIO 8 to control a strobe effect on a 25-pixel NeoPixel LED matrix. The first..."
  },
  {
    "id": "rtos-61",
    "title": "LED Matrix Swipe Detector",
    "language": "FreeRTOS",
    "category": "LED Matrix",
    "level": "Advanced",
    "filename": "LED Matrix Swipe Detector.ino",
    "codePath": "/advayam-codes/freertos/LED%20Matrix%20Swipe%20Detector.ino",
    "description": "LED Matrix Swipe Detector demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "rtos-62",
    "title": "LED Matrix Temperature Controlled Fire",
    "language": "FreeRTOS",
    "category": "LED Matrix",
    "level": "Advanced",
    "filename": "LED Matrix Temperature Controlled Fire.ino",
    "codePath": "/advayam-codes/freertos/LED%20Matrix%20Temperature%20Controlled%20Fire.ino",
    "description": "This program creates a temperature-controlled fire effect on the 5x5 RGB LED matrix using the LMT87 sensor inside a FreeRTOS task."
  },
  {
    "id": "rtos-63",
    "title": "LED Matrix Temperature Controlled Smiley",
    "language": "FreeRTOS",
    "category": "LED Matrix",
    "level": "Advanced",
    "filename": "LED Matrix Temperature Controlled Smiley.ino",
    "codePath": "/advayam-codes/freertos/LED%20Matrix%20Temperature%20Controlled%20Smiley.ino",
    "description": "This program displays happy, neutral, or sad faces on the RGB matrix depending on the LMT87 temperature monitored inside a dedicated FreeRTOS task."
  },
  {
    "id": "rtos-64",
    "title": "LED Matrix Temperature Heatmap",
    "language": "FreeRTOS",
    "category": "LED Matrix",
    "level": "Advanced",
    "filename": "LED Matrix Temperature Heatmap.ino",
    "codePath": "/advayam-codes/freertos/LED%20Matrix%20Temperature%20Heatmap.ino",
    "description": "LMT87 Temperature Heatmap - FreeRTOS Version This program displays the LMT87 temperature as a vertical heatmap on the 5x5 RGB matrix. Lower temperatures produce blue c..."
  },
  {
    "id": "rtos-65",
    "title": "LED Matrix Tetris Animation",
    "language": "FreeRTOS",
    "category": "LED Matrix",
    "level": "Advanced",
    "filename": "LED Matrix Tetris Animation.ino",
    "codePath": "/advayam-codes/freertos/LED%20Matrix%20Tetris%20Animation.ino",
    "description": "This program animates simple falling tetromino blocks on the 5x5 RGB matrix using a dedicated FreeRTOS task."
  },
  {
    "id": "rtos-66",
    "title": "LED Matrix Text Scroller",
    "language": "FreeRTOS",
    "category": "LED Matrix",
    "level": "Advanced",
    "filename": "LED Matrix Text Scroller.ino",
    "codePath": "/advayam-codes/freertos/LED%20Matrix%20Text%20Scroller.ino",
    "description": "LED Matrix Text Scroller demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "rtos-67",
    "title": "LED Matrix Tilt To HSV",
    "language": "FreeRTOS",
    "category": "LED Matrix",
    "level": "Advanced",
    "filename": "LED Matrix Tilt to HSV.ino",
    "codePath": "/advayam-codes/freertos/LED%20Matrix%20Tilt%20to%20HSV.ino",
    "description": "LED Matrix Tilt To HSV demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "rtos-68",
    "title": "LED Matrix Toggle State",
    "language": "FreeRTOS",
    "category": "LED Matrix",
    "level": "Advanced",
    "filename": "LED Matrix Toggle State.ino",
    "codePath": "/advayam-codes/freertos/LED%20Matrix%20Toggle%20State.ino",
    "description": "LED Matrix Toggle State demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "rtos-69",
    "title": "LED Matrix Trashcan Animation",
    "language": "FreeRTOS",
    "category": "LED Matrix",
    "level": "Advanced",
    "filename": "LED Matrix Trashcan Animation.ino",
    "codePath": "/advayam-codes/freertos/LED%20Matrix%20Trashcan%20Animation.ino",
    "description": "LED Matrix Trashcan Animation demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "rtos-70",
    "title": "LED Matrix Whack-A-Mole",
    "language": "FreeRTOS",
    "category": "LED Matrix",
    "level": "Advanced",
    "filename": "LED Matrix Whack-a-Mole.ino",
    "codePath": "/advayam-codes/freertos/LED%20Matrix%20Whack-a-Mole.ino",
    "description": "LED Matrix Whack-A-Mole demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "rtos-71",
    "title": "Matrix Rain Effect",
    "language": "FreeRTOS",
    "category": "LED Matrix",
    "level": "Advanced",
    "filename": "Matrix Rain Effect.ino",
    "codePath": "/advayam-codes/freertos/Matrix%20Rain%20Effect.ino",
    "description": "Matrix Rain Effect demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "rtos-72",
    "title": "Overheat Alarm",
    "language": "FreeRTOS",
    "category": "Thermal & Climate",
    "level": "Advanced",
    "filename": "Overheat Alarm.ino",
    "codePath": "/advayam-codes/freertos/Overheat%20Alarm.ino",
    "description": "Overheat Alarm demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "rtos-73",
    "title": "Proximity Counter",
    "language": "FreeRTOS",
    "category": "Input & Touch",
    "level": "Advanced",
    "filename": "Proximity Counter.ino",
    "codePath": "/advayam-codes/freertos/Proximity%20Counter.ino",
    "description": "Proximity Counter demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "rtos-74",
    "title": "Proximity Depth Gauge",
    "language": "FreeRTOS",
    "category": "Input & Touch",
    "level": "Advanced",
    "filename": "Proximity Depth Gauge.ino",
    "codePath": "/advayam-codes/freertos/Proximity%20Depth%20Gauge.ino",
    "description": "Proximity Depth Gauge demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "rtos-75",
    "title": "Proximity Distance Theremin",
    "language": "FreeRTOS",
    "category": "Audio & Buzzer",
    "level": "Advanced",
    "filename": "Proximity Distance Theremin.ino",
    "codePath": "/advayam-codes/freertos/Proximity%20Distance%20Theremin.ino",
    "description": "Proximity Distance Theremin demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "rtos-76",
    "title": "Proximity Greeting",
    "language": "FreeRTOS",
    "category": "Input & Touch",
    "level": "Advanced",
    "filename": "Proximity Greeting.ino",
    "codePath": "/advayam-codes/freertos/Proximity%20Greeting.ino",
    "description": "Proximity Greeting demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "rtos-77",
    "title": "Reaction Time Game",
    "language": "FreeRTOS",
    "category": "Games & Simulations",
    "level": "Advanced",
    "filename": "Reaction Time Game.ino",
    "codePath": "/advayam-codes/freertos/Reaction%20Time%20Game.ino",
    "description": "Reaction Time Game demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "rtos-78",
    "title": "RGB Colour Mixer",
    "language": "FreeRTOS",
    "category": "Sensors & Actuators",
    "level": "Advanced",
    "filename": "RGB Colour Mixer.ino",
    "codePath": "/advayam-codes/freertos/RGB%20Colour%20Mixer.ino",
    "description": "RGB Colour Mixer demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "rtos-79",
    "title": "Roll And Pitch Reader",
    "language": "FreeRTOS",
    "category": "Audio & Buzzer",
    "level": "Advanced",
    "filename": "Roll and Pitch Reader.ino",
    "codePath": "/advayam-codes/freertos/Roll%20and%20Pitch%20Reader.ino",
    "description": "Roll And Pitch Reader demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "rtos-80",
    "title": "Rolling Marble Simulator",
    "language": "FreeRTOS",
    "category": "Motion & IMU",
    "level": "Advanced",
    "filename": "Rolling Marble Simulator.ino",
    "codePath": "/advayam-codes/freertos/Rolling%20Marble%20Simulator.ino",
    "description": "Rolling Marble Simulator demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "rtos-81",
    "title": "Sand Falling Simulator",
    "language": "FreeRTOS",
    "category": "Motion & IMU",
    "level": "Advanced",
    "filename": "Sand Falling Simulator.ino",
    "codePath": "/advayam-codes/freertos/Sand%20Falling%20Simulator.ino",
    "description": "Sand Falling Simulator demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "rtos-82",
    "title": "Shadow Puppetry Brightness Detector",
    "language": "FreeRTOS",
    "category": "Light & Optical",
    "level": "Advanced",
    "filename": "Shadow Puppetry Brightness Detector.ino",
    "codePath": "/advayam-codes/freertos/Shadow%20Puppetry%20Brightness%20Detector.ino",
    "description": "Shadow Puppetry Brightness Detector demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "rtos-83",
    "title": "Shake To Shuffle LED Matrix Randomizer",
    "language": "FreeRTOS",
    "category": "LED Matrix",
    "level": "Advanced",
    "filename": "Shake to Shuffle LED Matrix Randomizer.ino",
    "codePath": "/advayam-codes/freertos/Shake%20to%20Shuffle%20LED%20Matrix%20Randomizer.ino",
    "description": "Shake To Shuffle LED Matrix Randomizer demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "rtos-84",
    "title": "Six Orientation Detector",
    "language": "FreeRTOS",
    "category": "Motion & IMU",
    "level": "Advanced",
    "filename": "Six Orientation Detector.ino",
    "codePath": "/advayam-codes/freertos/Six%20Orientation%20Detector.ino",
    "description": "Six Orientation Detector demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "rtos-85",
    "title": "Step Counter",
    "language": "FreeRTOS",
    "category": "Motion & IMU",
    "level": "Advanced",
    "filename": "Step Counter.ino",
    "codePath": "/advayam-codes/freertos/Step%20Counter.ino",
    "description": "Step Counter demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "rtos-86",
    "title": "Temperature High-Low Latch",
    "language": "FreeRTOS",
    "category": "Thermal & Climate",
    "level": "Advanced",
    "filename": "Temperature High-Low Latch.ino",
    "codePath": "/advayam-codes/freertos/Temperature%20High-Low%20Latch.ino",
    "description": "Temperature High-Low Latch demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "rtos-87",
    "title": "Temperature Reading Comparison",
    "language": "FreeRTOS",
    "category": "Thermal & Climate",
    "level": "Advanced",
    "filename": "Temperature Reading Comparison.ino",
    "codePath": "/advayam-codes/freertos/Temperature%20Reading%20Comparison.ino",
    "description": "Temperature Reading Comparison demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "rtos-88",
    "title": "Temperature To Pitch Converter",
    "language": "FreeRTOS",
    "category": "Audio & Buzzer",
    "level": "Advanced",
    "filename": "Temperature to Pitch Converter.ino",
    "codePath": "/advayam-codes/freertos/Temperature%20to%20Pitch%20Converter.ino",
    "description": "Temperature To Pitch Converter demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "rtos-89",
    "title": "Tilt Balance Stability Game",
    "language": "FreeRTOS",
    "category": "Games & Simulations",
    "level": "Advanced",
    "filename": "Tilt Balance Stability Game.ino",
    "codePath": "/advayam-codes/freertos/Tilt%20Balance%20Stability%20Game.ino",
    "description": "Tilt Balance Stability Game demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "rtos-90",
    "title": "Tilt Controlled Dice Roller",
    "language": "FreeRTOS",
    "category": "Games & Simulations",
    "level": "Advanced",
    "filename": "Tilt Controlled Dice Roller.ino",
    "codePath": "/advayam-codes/freertos/Tilt%20Controlled%20Dice%20Roller.ino",
    "description": "Tilt Controlled Dice Roller demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "rtos-91",
    "title": "Tilt Controlled Maze Game",
    "language": "FreeRTOS",
    "category": "Games & Simulations",
    "level": "Advanced",
    "filename": "Tilt Controlled Maze Game.ino",
    "codePath": "/advayam-codes/freertos/Tilt%20Controlled%20Maze%20Game.ino",
    "description": "Tilt Controlled Maze Game demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "rtos-92",
    "title": "Tilt To Sound Pitch Controller",
    "language": "FreeRTOS",
    "category": "Audio & Buzzer",
    "level": "Advanced",
    "filename": "Tilt to Sound Pitch Controller.ino",
    "codePath": "/advayam-codes/freertos/Tilt%20to%20Sound%20Pitch%20Controller.ino",
    "description": "Tilt To Sound Pitch Controller demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "rtos-93",
    "title": "Touchless Page Turner",
    "language": "FreeRTOS",
    "category": "Input & Touch",
    "level": "Advanced",
    "filename": "Touchless Page Turner.ino",
    "codePath": "/advayam-codes/freertos/Touchless%20Page%20Turner.ino",
    "description": "Touchless Page Turner demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "rtos-94",
    "title": "Touchless Proximity Switch",
    "language": "FreeRTOS",
    "category": "Input & Touch",
    "level": "Advanced",
    "filename": "Touchless Proximity Switch.ino",
    "codePath": "/advayam-codes/freertos/Touchless%20Proximity%20Switch.ino",
    "description": "Touchless Proximity Switch demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "rtos-95",
    "title": "Touchless Volume Knob",
    "language": "FreeRTOS",
    "category": "Input & Touch",
    "level": "Advanced",
    "filename": "Touchless Volume Knob.ino",
    "codePath": "/advayam-codes/freertos/Touchless%20Volume%20Knob.ino",
    "description": "Touchless Volume Knob demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "rtos-96",
    "title": "Tug Of War Game",
    "language": "FreeRTOS",
    "category": "Games & Simulations",
    "level": "Advanced",
    "filename": "Tug of War Game.ino",
    "codePath": "/advayam-codes/freertos/Tug%20of%20War%20Game.ino",
    "description": "Tug Of War Game demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "rtos-97",
    "title": "Wifi Signal Strength Indicator",
    "language": "FreeRTOS",
    "category": "Connectivity & System",
    "level": "Advanced",
    "filename": "Wifi Signal Strength Indicator.ino",
    "codePath": "/advayam-codes/freertos/Wifi%20Signal%20Strength%20Indicator.ino",
    "description": "Wifi Signal Strength Indicator demo project running on Advayam ESP32 development hardware using the FreeRTOS framework."
  },
  {
    "id": "cpp-0",
    "title": "Blink LED",
    "language": "Arduino",
    "category": "Sensors & Actuators",
    "level": "Beginner",
    "filename": "Blink-LED.cpp",
    "codePath": "/community/demo-programs/blink-led",
    "description": "Control an LED using GPIO output and timing functions.",
    "existingHref": "/community/demo-programs/blink-led"
  },
  {
    "id": "cpp-1",
    "title": "Button Control LED",
    "language": "Arduino",
    "category": "Input & Touch",
    "level": "Beginner",
    "filename": "Program1.cpp",
    "codePath": "/Programs/Program1.cpp",
    "description": "Control the user LED using a push button (LED turns ON only while the button is pressed).",
    "existingHref": "/community/demo-programs/1"
  },
  {
    "id": "cpp-2",
    "title": "Button Toggle LED",
    "language": "Arduino",
    "category": "Input & Touch",
    "level": "Beginner",
    "filename": "Program2.cpp",
    "codePath": "/Programs/Program2.cpp",
    "description": "Toggle the user LED state (ON/OFF) on button presses with debounce protection.",
    "existingHref": "/community/demo-programs/2"
  },
  {
    "id": "cpp-3",
    "title": "UART Control LED",
    "language": "Arduino",
    "category": "Connectivity & System",
    "level": "Beginner",
    "filename": "Program3.cpp",
    "codePath": "/Programs/Program3.cpp",
    "description": "Control the user LED status using serial port (UART) terminal commands (ON/OFF).",
    "existingHref": "/community/demo-programs/3"
  },
  {
    "id": "cpp-4",
    "title": "Buzzer Touch Trigger",
    "language": "Arduino",
    "category": "Audio & Buzzer",
    "level": "Beginner",
    "filename": "Program4.cpp",
    "codePath": "/Programs/Program4.cpp",
    "description": "Trigger a buzzer beep when the capacitive touch sensor detects a touch.",
    "existingHref": "/community/demo-programs/4"
  },
  {
    "id": "cpp-5",
    "title": "Matrix Flash Color",
    "language": "Arduino",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "Program5.cpp",
    "codePath": "/Programs/Program5.cpp",
    "description": "Flash the whole RGB matrix sequentially with different colors.",
    "existingHref": "/community/demo-programs/5"
  },
  {
    "id": "cpp-6",
    "title": "Matrix Touch Chase",
    "language": "Arduino",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "Program6.cpp",
    "codePath": "/Programs/Program6.cpp",
    "description": "Advance a single lit LED across the RGB matrix layout on touch detection.",
    "existingHref": "/community/demo-programs/6"
  },
  {
    "id": "cpp-7",
    "title": "Matrix UART Control",
    "language": "Arduino",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "Program7.cpp",
    "codePath": "/Programs/Program7.cpp",
    "description": "Set the RGB matrix color by sending text color names over UART.",
    "existingHref": "/community/demo-programs/7"
  },
  {
    "id": "cpp-8",
    "title": "Temperature Readout Basic",
    "language": "Arduino",
    "category": "Thermal & Climate",
    "level": "Intermediate",
    "filename": "Program8.cpp",
    "codePath": "/Programs/Program8.cpp",
    "description": "Read temperature data from the onboard temperature sensor and print basic values.",
    "existingHref": "/community/demo-programs/8"
  },
  {
    "id": "cpp-9",
    "title": "Temperature Readout Formatted",
    "language": "Arduino",
    "category": "Thermal & Climate",
    "level": "Intermediate",
    "filename": "Program9.cpp",
    "codePath": "/Programs/Program9.cpp",
    "description": "Read temperature in Celsius and Fahrenheit, and print formatted outputs to the serial monitor.",
    "existingHref": "/community/demo-programs/9"
  },
  {
    "id": "cpp-10",
    "title": "LED Chase Matrix",
    "language": "Arduino",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "Program10.cpp",
    "codePath": "/Programs/Program10.cpp",
    "description": "Drive a single blue LED to chase across the matrix rows and columns.",
    "existingHref": "/community/demo-programs/10"
  },
  {
    "id": "cpp-11",
    "title": "Dual LED Matrix Chase",
    "language": "Arduino",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "Program11.cpp",
    "codePath": "/Programs/Program11.cpp",
    "description": "Chase red and blue LEDs simultaneously across the matrix layout.",
    "existingHref": "/community/demo-programs/11"
  },
  {
    "id": "cpp-12",
    "title": "Matrix Breathing Effect",
    "language": "Arduino",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "Program12.cpp",
    "codePath": "/Programs/Program12.cpp",
    "description": "Perform a breathing fade-in and fade-out brightness effect on the blue matrix.",
    "existingHref": "/community/demo-programs/12"
  },
  {
    "id": "cpp-13",
    "title": "Touch Toggle Matrix",
    "language": "Arduino",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "Program13.cpp",
    "codePath": "/Programs/Program13.cpp",
    "description": "Toggle the whole white RGB matrix ON or OFF on capacitive touch sensor activation.",
    "existingHref": "/community/demo-programs/13"
  },
  {
    "id": "cpp-14",
    "title": "Touch Cycle Colors",
    "language": "Arduino",
    "category": "Input & Touch",
    "level": "Intermediate",
    "filename": "Program14.cpp",
    "codePath": "/Programs/Program14.cpp",
    "description": "Cycle through different solid colors on the matrix upon touch detection.",
    "existingHref": "/community/demo-programs/14"
  },
  {
    "id": "cpp-15",
    "title": "Touch Cycle Brightness",
    "language": "Arduino",
    "category": "Light & Optical",
    "level": "Intermediate",
    "filename": "Program15.cpp",
    "codePath": "/Programs/Program15.cpp",
    "description": "Cycle through matrix brightness levels for a blue light display on touch activation.",
    "existingHref": "/community/demo-programs/15"
  },
  {
    "id": "cpp-16",
    "title": "UART Matrix Brightness",
    "language": "Arduino",
    "category": "LED Matrix",
    "level": "Intermediate",
    "filename": "Program16.cpp",
    "codePath": "/Programs/Program16.cpp",
    "description": "Adjust RGB matrix brightness dynamically via Serial commands (BRT=0 to BRT=255).",
    "existingHref": "/community/demo-programs/16"
  },
  {
    "id": "cpp-17",
    "title": "Matrix Heart Pattern",
    "language": "Arduino",
    "category": "LED Matrix",
    "level": "Advanced",
    "filename": "Program17.cpp",
    "codePath": "/Programs/Program17.cpp",
    "description": "Display a heart icon/pattern layout on the RGB matrix.",
    "existingHref": "/community/demo-programs/17"
  },
  {
    "id": "cpp-18",
    "title": "Auto Cycle Digits",
    "language": "Arduino",
    "category": "Sensors & Actuators",
    "level": "Advanced",
    "filename": "Program18.cpp",
    "codePath": "/Programs/Program18.cpp",
    "description": "Automatically cycle through digit symbols (0-9) on the LED matrix.",
    "existingHref": "/community/demo-programs/18"
  },
  {
    "id": "cpp-19",
    "title": "Touch Cycle Digits",
    "language": "Arduino",
    "category": "Input & Touch",
    "level": "Advanced",
    "filename": "Program19.cpp",
    "codePath": "/Programs/Program19.cpp",
    "description": "Cycle and display digits (0-9) on the matrix with touch sensor control.",
    "existingHref": "/community/demo-programs/19"
  },
  {
    "id": "cpp-20",
    "title": "Auto Cycle Alphabet",
    "language": "Arduino",
    "category": "Sensors & Actuators",
    "level": "Advanced",
    "filename": "Program20.cpp",
    "codePath": "/Programs/Program20.cpp",
    "description": "Automatically cycle and render alphabet characters A to J on the matrix.",
    "existingHref": "/community/demo-programs/20"
  },
  {
    "id": "cpp-21",
    "title": "Scroll 'ADVAYAM'",
    "language": "Arduino",
    "category": "Motion & IMU",
    "level": "Advanced",
    "filename": "Program21.cpp",
    "codePath": "/Programs/Program21.cpp",
    "description": "Automatically scroll the name 'ADVAYAM' letter by letter on the matrix.",
    "existingHref": "/community/demo-programs/21"
  },
  {
    "id": "cpp-22",
    "title": "Touch Scroll 'ADVAYAM'",
    "language": "Arduino",
    "category": "Input & Touch",
    "level": "Advanced",
    "filename": "Program22.cpp",
    "codePath": "/Programs/Program22.cpp",
    "description": "Display and advance letters of 'ADVAYAM' one-by-one on touch, then clear.",
    "existingHref": "/community/demo-programs/22"
  },
  {
    "id": "cpp-23",
    "title": "LSM6DSOX SPI Readout",
    "language": "Arduino",
    "category": "Connectivity & System",
    "level": "Advanced",
    "filename": "Program23.cpp",
    "codePath": "/Programs/Program23.cpp",
    "description": "Interface with the LSM6DSOX accelerometer/gyroscope sensor using SPI protocol.",
    "existingHref": "/community/demo-programs/23"
  }
];
