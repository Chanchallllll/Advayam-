"use client";

import { useEffect, useState } from "react";
import {
  Bot,
  CarFront,
  Cloud,
  Cpu,
  Droplets,
  Sprout,
  Thermometer,
  Wifi,
  Zap,
} from "lucide-react";
import { AnimatePresence, motion } from "framer-motion";
import plantSenseImage from "@/projects/plant_Sense_control-removebg-preview.webp";
import roboticShieldImage from "@/projects/robotic_shield_project-removebg-preview.webp";
import communicationShieldImage from "@/projects/c_shield (1).png";

const projects = [
  {
    id: "plant-sense-control",
    eyebrow: "Automated Agriculture",
    title: "Plant Sense Control Project",
    description:
      "An automated plant-monitoring and irrigation system that uses the PlantSense Control Shield board to monitor soil conditions and control watering for healthier plants.",
    image: plantSenseImage.src,
    alt: "Plant Sense Control project with its monitoring and irrigation setup",
    label: "Plant Sense Control Project",
    note: "Smart Plant Care",
    tags: [
      { icon: Sprout, label: "Plant Monitoring" },
      { icon: Droplets, label: "Automated Irrigation" },
      { icon: Cpu, label: "PlantSense Shield" },
    ],
  },
  {
    id: "robotic-car-arm",
    eyebrow: "Robotics & Motion Control",
    title: "Robotic Car & Robotic Arm",
    description:
      "A robotics project featuring a Robotic Shield board to control the movement of a four-wheel robotic car and the servo-driven robotic arm.",
    image: roboticShieldImage.src,
    alt: "Robotic car and robotic arm project using a Robotic Shield",
    label: "Robotic Car & Robotic Arm",
    note: "Mobile Robotics",
    tags: [
      { icon: CarFront, label: "Four-Wheel Car" },
      { icon: Bot, label: "Robotic Arm" },
      { icon: Zap, label: "Robotic Shield" },
      { icon: CarFront, label: "Line Following Robot" },
    ],
  },
  {
    id: "iot-communication",
    eyebrow: "IoT & Cloud Monitoring",
    title: "IoT Communication Project",
    description:
      "The Communication Shield collects data from connected sensors, processes it through the onboard microcontroller, and prepares it for transmission in IoT applications. It sends the collected data to a cloud server or remote dashboard via Wi-Fi or Ethernet, enabling remote monitoring and data access.",
    image: communicationShieldImage.src,
    alt: "IoT communication project with a communication shield and plant sensor dashboard",
    label: "IoT Communication Project",
    note: "Remote IoT Monitoring",
    tags: [
      { icon: Droplets, label: "Soil Moisture" },
      { icon: Thermometer, label: "Temperature & Humidity" },
      { icon: Cloud, label: "Wi-Fi Cloud Dashboard" },
      { icon: Wifi, label: "Communication Shield" },
    ],
  },
];

export default function ProjectShowcase() {
  const [activeSlide, setActiveSlide] = useState(0);
  const [isAutoPlaying, setIsAutoPlaying] = useState(true);
  const currentProject = projects[activeSlide];

  useEffect(() => {
    if (!isAutoPlaying) return;

    const timer = setInterval(() => {
      setActiveSlide((current) => (current + 1) % projects.length);
    }, 5000);

    return () => clearInterval(timer);
  }, [isAutoPlaying]);

  return (
    <section id="projects" className="mt-16 scroll-mt-8 sm:mt-20">
      <h2 className="font-jakarta text-xl font-black text-white sm:text-2xl">
        <span className="text-xl text-[#EA580C] sm:text-2xl">12.</span>{" "}
        Featured Projects
      </h2>
      <p className="font-jakarta mt-4 max-w-4xl text-sm font-medium leading-7 text-white/65 sm:text-base">
        Explore practical builds that bring Advayam boards and shields to life.
      </p>

      <div
        className="relative mt-8 overflow-hidden rounded-3xl border border-white/15 bg-black/70 p-5 text-white shadow-2xl backdrop-blur-2xl sm:p-7 md:p-10"
        onMouseEnter={() => setIsAutoPlaying(false)}
      >
        <AnimatePresence mode="wait" initial={false}>
          <motion.div
            key={currentProject.id}
            initial={{ opacity: 0, x: 56 }}
            animate={{ opacity: 1, x: 0 }}
            exit={{ opacity: 0, x: -56 }}
            transition={{ duration: 0.55, ease: "easeInOut" }}
            className="flex flex-col items-center justify-between gap-8 lg:flex-row"
          >
            <div className="w-full text-left lg:max-w-lg">
              <div className="inline-flex items-center gap-2 rounded-full border border-[#BE5209]/40 bg-[#BE5209]/20 px-4 py-1.5 text-xs font-bold uppercase tracking-wider text-orange-400">
                <Cpu className="h-4 w-4" /> {currentProject.eyebrow}
              </div>
              <h3 className="font-jakarta mt-3 text-2xl font-extrabold leading-snug text-white md:text-3xl">
                {currentProject.title}
              </h3>
              <p className="font-jakarta mt-3 text-sm leading-relaxed text-slate-300 md:text-base">
                {currentProject.description}
              </p>

              <div className="mt-6 flex flex-wrap gap-2.5">
                {currentProject.tags.map((tag) => {
                  const Icon = tag.icon;

                  return (
                    <span
                      key={`${currentProject.id}-${tag.label}`}
                      className="flex items-center gap-1.5 rounded-lg border border-white/15 bg-white/10 px-3.5 py-1.5 text-xs font-bold text-slate-200 shadow-sm"
                    >
                      <Icon className="h-4 w-4 text-[#BE5209]" /> {tag.label}
                    </span>
                  );
                })}
              </div>
            </div>

            <div className="group relative w-full shrink-0 lg:w-[52%]">
              <div className="absolute inset-0 rounded-3xl bg-black/70 opacity-80 blur-2xl" />
              <div className="relative rounded-2xl border border-white/15 bg-black/60 p-4 shadow-[0_0_30px_rgba(0,0,0,0.9)] transition duration-300 hover:border-white/30">
                <img
                  src={currentProject.image}
                  alt={currentProject.alt}
                  className="h-[260px] w-full object-contain transition-transform duration-500 group-hover:scale-[1.03] sm:h-[340px] lg:h-[400px]"
                />
                <div className="mt-3 flex items-center justify-between gap-3 border-t border-white/10 pt-3">
                  <span className="font-jakarta text-xs font-bold text-slate-400">
                    {currentProject.label}
                  </span>
                  <span className="font-jakarta text-right text-xs font-bold text-orange-400">
                    {currentProject.note}
                  </span>
                </div>
              </div>
            </div>
          </motion.div>
        </AnimatePresence>

        <div className="mt-6 flex w-full items-center justify-center gap-3" aria-label="Project slides">
          {projects.map((project, index) => (
            <button
              key={`${project.id}-indicator`}
              type="button"
              onClick={() => {
                setIsAutoPlaying(false);
                setActiveSlide(index);
              }}
              className={`h-2.5 rounded-full transition-all ${
                index === activeSlide
                  ? "w-8 bg-[#BE5209]"
                  : "w-2.5 bg-white/30 hover:bg-white/50"
              }`}
              aria-label={`View project ${index + 1}: ${project.title}`}
              aria-current={index === activeSlide ? "true" : undefined}
              title={`View ${project.title}`}
            />
          ))}
        </div>
      </div>

      <div
        id="boards-compatible"
        className="mt-8 rounded-3xl border border-white/15 bg-black/50 p-5 text-white shadow-xl backdrop-blur-2xl sm:p-7"
      >
        <h3 className="font-jakarta text-xl font-extrabold text-white sm:text-2xl">
          Boards Compatible
        </h3>
        <div className="mt-4 flex flex-wrap gap-3">
          {["Advayam ESP32 S3", "Advayam Thejas32 SK"].map((board) => (
            <span
              key={board}
              className="rounded-lg border border-white/15 bg-white/10 px-4 py-2 text-sm font-bold text-slate-200"
            >
              {board}
            </span>
          ))}
        </div>
      </div>
    </section>
  );
}