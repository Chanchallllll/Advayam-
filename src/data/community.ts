export type CommunityPerson = {
  name: string;
  role: string;
  image: string;
};

export const mentors: CommunityPerson[] = [
  {
    name: "Sojan O Nelson",
    role: "Lead Architect",
    image: "/people/sojan.jpg",
  },
  {
    name: "Nidhin V Raj",
    role: "Hardware Consultant",
    image: "/logos/advayam-icon.svg",
  },
  {
    name: "Denil Sunny",
    role: "Hardware Designer",
    image: "/logos/advayam-icon.svg",
  },
];

export const contributors: CommunityPerson[] = [
  {
    name: "Chanchal Haridas",
    role: "Developer",
    image: "/logos/advayam-icon.svg",
  },
  {
    name: "Akshay Prasad",
    role: "IDE Developer",
    image: "/people/akshay.jpg",
  },
  {
    name: "Pridhu Raj R S",
    role: "Web Designer",
    image: "/logos/advayam-icon.svg",
  },
  {
    name: "Angel Roy",
    role: "Website Backend Developer",
    image: "/logos/advayam-icon.svg",
  },
  {
    name: "Gowri Lal",
    role: "Website Frontend Developer",
    image: "/people/gowri.jpg",
  },
  {
    name: "Shifa Fathima K",
    role: "IDE Developer",
    image: "/people/shifa.jpg",
  },
  {
    name: "Priyananda T",
    role: "VS Code Extension Developer",
    image: "/people/priya.jpg",
  },
  {
    name: "Neha Maria Rose",
    role: "VS Code Extension Developer",
    image: "/logos/advayam-icon.svg",
  },
  {
    name: "Sidharth Sunil",
    role: "VS Code Extension Developer",
    image: "/logos/advayam-icon.svg",
  },
  {
    name: "Krishnaja S",
    role: "IDE Developer",
    image: "/people/krishna.jpg",
  },
  {
    name: "Gautham Krishna B",
    role: "VS Code Extension Developer",
    image: "/logos/advayam-icon.svg",
  },
];

export const demoPrograms = [
  {
    id: 0,
    title: "Blink LED",
    level: "Beginner",
    description:
      "Control an LED using GPIO output and timing functions.",
    href: "/community/demo-programs/blink-led",
  },
  {
    id: 1,
    title: "Button Control LED",
    level: "Beginner",
    description: "Control the user LED using a push button (LED turns ON only while the button is pressed).",
    href: "/community/demo-programs/1",
  },
  {
    id: 2,
    title: "Button Toggle LED",
    level: "Beginner",
    description: "Toggle the user LED state (ON/OFF) on button presses with debounce protection.",
    href: "/community/demo-programs/2",
  },
  {
    id: 3,
    title: "UART Control LED",
    level: "Beginner",
    description: "Control the user LED status using serial port (UART) terminal commands (ON/OFF).",
    href: "/community/demo-programs/3",
  },
  {
    id: 4,
    title: "Buzzer Touch Trigger",
    level: "Beginner",
    description: "Trigger a buzzer beep when the capacitive touch sensor detects a touch.",
    href: "/community/demo-programs/4",
  },
  {
    id: 5,
    title: "Matrix Flash Color",
    level: "Intermediate",
    description: "Flash the whole RGB matrix sequentially with different colors.",
    href: "/community/demo-programs/5",
  },
  {
    id: 6,
    title: "Matrix Touch Chase",
    level: "Intermediate",
    description: "Advance a single lit LED across the RGB matrix layout on touch detection.",
    href: "/community/demo-programs/6",
  },
  {
    id: 7,
    title: "Matrix UART Control",
    level: "Intermediate",
    description: "Set the RGB matrix color by sending text color names over UART.",
    href: "/community/demo-programs/7",
  },
  {
    id: 8,
    title: "Temperature Readout Basic",
    level: "Intermediate",
    description: "Read temperature data from the onboard temperature sensor and print basic values.",
    href: "/community/demo-programs/8",
  },
  {
    id: 9,
    title: "Temperature Readout Formatted",
    level: "Intermediate",
    description: "Read temperature in Celsius and Fahrenheit, and print formatted outputs to the serial monitor.",
    href: "/community/demo-programs/9",
  },
  {
    id: 10,
    title: "LED Chase Matrix",
    level: "Intermediate",
    description: "Drive a single blue LED to chase across the matrix rows and columns.",
    href: "/community/demo-programs/10",
  },
  {
    id: 11,
    title: "Dual LED Matrix Chase",
    level: "Intermediate",
    description: "Chase red and blue LEDs simultaneously across the matrix layout.",
    href: "/community/demo-programs/11",
  },
  {
    id: 12,
    title: "Matrix Breathing Effect",
    level: "Intermediate",
    description: "Perform a breathing fade-in and fade-out brightness effect on the blue matrix.",
    href: "/community/demo-programs/12",
  },
  {
    id: 13,
    title: "Touch Toggle Matrix",
    level: "Intermediate",
    description: "Toggle the whole white RGB matrix ON or OFF on capacitive touch sensor activation.",
    href: "/community/demo-programs/13",
  },
  {
    id: 14,
    title: "Touch Cycle Colors",
    level: "Intermediate",
    description: "Cycle through different solid colors on the matrix upon touch detection.",
    href: "/community/demo-programs/14",
  },
  {
    id: 15,
    title: "Touch Cycle Brightness",
    level: "Intermediate",
    description: "Cycle through matrix brightness levels for a blue light display on touch activation.",
    href: "/community/demo-programs/15",
  },
  {
    id: 16,
    title: "UART Matrix Brightness",
    level: "Intermediate",
    description: "Adjust RGB matrix brightness dynamically via Serial commands (BRT=0 to BRT=255).",
    href: "/community/demo-programs/16",
  },
  {
    id: 17,
    title: "Matrix Heart Pattern",
    level: "Advanced",
    description: "Display a heart icon/pattern layout on the RGB matrix.",
    href: "/community/demo-programs/17",
  },
  {
    id: 18,
    title: "Auto Cycle Digits",
    level: "Advanced",
    description: "Automatically cycle through digit symbols (0-9) on the LED matrix.",
    href: "/community/demo-programs/18",
  },
  {
    id: 19,
    title: "Touch Cycle Digits",
    level: "Advanced",
    description: "Cycle and display digits (0-9) on the matrix with touch sensor control.",
    href: "/community/demo-programs/19",
  },
  {
    id: 20,
    title: "Auto Cycle Alphabet",
    level: "Advanced",
    description: "Automatically cycle and render alphabet characters A to J on the matrix.",
    href: "/community/demo-programs/20",
  },
  {
    id: 21,
    title: "Scroll 'ADVAYAM'",
    level: "Advanced",
    description: "Automatically scroll the name 'ADVAYAM' letter by letter on the matrix.",
    href: "/community/demo-programs/21",
  },
  {
    id: 22,
    title: "Touch Scroll 'ADVAYAM'",
    level: "Advanced",
    description: "Display and advance letters of 'ADVAYAM' one-by-one on touch, then clear.",
    href: "/community/demo-programs/22",
  },
  {
    id: 23,
    title: "LSM6DSOX SPI Readout",
    level: "Advanced",
    description: "Interface with the LSM6DSOX accelerometer/gyroscope sensor using SPI protocol.",
    href: "/community/demo-programs/23",
  },
];