"use client";

import { useState } from "react";
import { Cpu, Code2, GraduationCap, Users, ArrowRight, X, Sparkles, CheckCircle2 } from "lucide-react";

type EcosystemItem = {
  id: string;
  title: string;
  icon: React.ElementType;
  badgeBg: string;
  badgeText: string;
  gradient: string;
  shortText: string;
  contentLead: string;
  content: string;
  bullets: string[];
  link: string;
  linkText: string;
  social?: boolean;
};

const ecosystem: EcosystemItem[] = [
  {
    id: "hardware",
    title: "Hardware",
    icon: Cpu,
    badgeBg: "bg-orange-500/10 border-orange-500/20",
    badgeText: "text-[#EA580C]",
    gradient: "from-orange-500/15 via-amber-500/5 to-transparent",
    shortText:
      "High-performance indigenous hardware boards for education, IoT, robotics, industrial automation, and Edge AI.",
    contentLead: "Hardware for learning, prototyping, and industrial systems.",
    content:
      "Advayam hardware includes core boards and base boards for embedded learning, IoT, robotics, automation, industrial communication, and future Edge AI applications.",
    bullets: ["Core boards & RISC-V processors", "Explorer & Ethernet expansion boards", "Industrial Relay & Sensor modules", "Dual USB-C & CAN Bus interfaces"],
    link: "/hardware",
    linkText: "Explore Hardware",
  },
  {
    id: "community",
    title: "Community",
    icon: Users,
    badgeBg: "bg-blue-500/10 border-blue-500/20",
    badgeText: "text-blue-600",
    gradient: "from-blue-500/15 via-cyan-500/5 to-transparent",
    shortText:
      "Collaborate, contribute, and innovate with the growing Advayam developer community.",
    contentLead: "A vibrant community for learners, makers, educators, and developers.",
    content:
      "Join forums, hackathons, showcases, and developer collaborations that help grow the Advayam ecosystem.",
    bullets: ["Technical Forums & Q&A", "Hackathons & Innovation Events", "Maker & Student Showcases", "Real-world Project Blueprints"],
    link: "/community",
    linkText: "Join Community",
    social: true,
  },
  {
    id: "software",
    title: "Software",
    icon: Code2,
    badgeBg: "bg-purple-500/10 border-purple-500/20",
    badgeText: "text-purple-600",
    gradient: "from-purple-500/15 via-indigo-500/5 to-transparent",
    shortText:
      "Advayam VS Code Extension, Web IDE, Simulators, SDKs, and developer APIs.",
    contentLead: "Software tools for complete embedded development.",
    content:
      "Use the Advayam VS Code Extension, Web IDE, simulators, SDKs, APIs, documentation, and future OTA deployment support.",
    bullets: ["VS Code Extension", "Web IDE & Online Compiler", "Interactive Board Simulators", "Production C/C++ & MicroPython SDKs"],
    link: "/software",
    linkText: "Explore Software",
  },
  {
    id: "learning",
    title: "Learning",
    icon: GraduationCap,
    badgeBg: "bg-emerald-500/10 border-emerald-500/20",
    badgeText: "text-emerald-600",
    gradient: "from-emerald-500/15 via-teal-500/5 to-transparent",
    shortText:
      "Guided learning pathways engineered for students, professionals, and makers.",
    contentLead: "Structured learning paths for every stage of your career.",
    content:
      "Advayam supports school students, undergraduates, postgraduates, professionals, and makers through hands-on practical learning modules.",
    bullets: ["K-12 School & College Tracks", "RTOS & Edge AI Professional Workflows", "Maker & DIY Project Tutorials", "Classroom to Industry Curricula"],
    link: "/learningpath",
    linkText: "Explore Learning",
  },
];

export default function EcosystemOverview() {
  const [activeItem, setActiveItem] = useState<EcosystemItem | null>(null);

  return (
    <section className="relative w-full overflow-hidden bg-transparent py-20 sm:py-28 text-white font-jakarta">
      {/* Background Ambient Blur */}
      <div className="pointer-events-none absolute top-1/3 left-1/2 -translate-x-1/2 h-[600px] w-[900px] rounded-full bg-gradient-to-r from-orange-500/8 via-amber-500/5 to-transparent blur-[160px]" />

      <div className="relative z-10 mx-auto max-w-7xl px-4 sm:px-6 lg:px-8">
        
        {/* Section Title Header */}
        <div className="text-center max-w-3xl mx-auto">
          <div className="inline-flex items-center gap-2 rounded-full border border-orange-500/30 bg-orange-500/15 px-4 py-1.5 text-xs font-semibold text-orange-400">
            <Sparkles className="h-3.5 w-3.5" />
            <span>Advayam Core Pillars</span>
          </div>

          <h2 className="mt-4 font-extrabold text-3xl sm:text-5xl tracking-tight text-white">
            Our Ecosystem
          </h2>

          <p className="mt-3.5 text-base sm:text-lg text-slate-300 font-medium leading-relaxed">
            A unified hardware, software, and learning architecture designed for high-performance embedded systems.
          </p>
        </div>

        {/* 4-Card Ecosystem Grid */}
        <div className="mt-14 grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-4 gap-6">
          {ecosystem.map((item) => {
            const Icon = item.icon;
            const isSelected = activeItem?.id === item.id;

            return (
              <div
                key={item.id}
                className={`group relative flex flex-col justify-between rounded-3xl border transition-all duration-300 bg-white/[0.04] backdrop-blur-xl overflow-hidden ${
                  isSelected
                    ? "border-orange-500 shadow-2xl ring-2 ring-orange-500/30"
                    : "border-white/15 shadow-sm hover:shadow-2xl hover:border-white/30 hover:-translate-y-1.5"
                }`}
              >
                {/* Top Banner Card Background */}
                <div className={`relative h-auto w-full bg-gradient-to-b ${item.gradient} p-6 pb-4 flex flex-col overflow-hidden`}>                  {/* Decorative Circuit Circle */}
                  <div className="absolute -right-6 -top-6 h-32 w-32 rounded-full border border-white/10 opacity-40 pointer-events-none" />


                  {/* Large Icon Container */}
                  <div className="flex h-14 w-14 items-center justify-center rounded-2xl bg-black/40 shadow-lg border border-white/15 text-white transition-transform duration-300 group-hover:scale-110">
                    <Icon className="h-7 w-7 text-orange-400" />
                  </div>
                </div>

                {/* Card Body Details */}
                  <div className="flex flex-1 flex-col justify-between px-6 pb-6 pt-1">                  <div>
                    <h3 className="text-xl font-extrabold text-white group-hover:text-orange-400 transition-colors">
                      {item.title}
                    </h3>

                    <p className="mt-2.5 text-sm text-slate-300 font-medium leading-relaxed">
                      {item.shortText}
                    </p>
                  </div>

                  {/* Card Bottom CTA Actions */}
                  <div className="mt-6 pt-5 border-t border-white/10 flex items-center justify-between">
                    <button
                      onClick={() => setActiveItem(isSelected ? null : item)}
                      className="text-xs font-bold text-slate-400 hover:text-white transition-colors"
                    >
                      {isSelected ? "Close ↑" : "Quick View ↑"}
                    </button>

                    <a
                      href={item.link}
                      className="inline-flex items-center gap-1 text-xs font-bold text-orange-400 group-hover:text-orange-300 transition-all"
                    >
                      <span>Read more</span>
                      <ArrowRight className="h-3.5 w-3.5 transition-transform duration-200 group-hover:translate-x-1" />
                    </a>
                  </div>
                </div>
              </div>
            );
          })}
        </div>

        {/* Expandable Detail Panel Drawer */}
        {activeItem && (
          <div className="mt-8 rounded-3xl border border-orange-500/30 bg-[#111111]/95 backdrop-blur-2xl p-7 sm:p-10 shadow-2xl animate-in fade-in slide-in-from-bottom-4 duration-300 text-white">
            <div className="flex items-center justify-between border-b border-white/10 pb-5">
              <div className="flex items-center gap-3">
                <div className="flex h-10 w-10 items-center justify-center rounded-xl bg-orange-500/20 text-orange-400">
                  <activeItem.icon className="h-5 w-5" />
                </div>
                <div>
                  <h3 className="text-2xl font-extrabold text-white">
                    Advayam {activeItem.title} Ecosystem
                  </h3>
                  <p className="text-xs font-semibold text-slate-400">
                    {activeItem.contentLead}
                  </p>
                </div>
              </div>

              <button
                onClick={() => setActiveItem(null)}
                className="flex h-9 w-9 items-center justify-center rounded-full bg-white/10 text-slate-300 hover:bg-white/20 transition"
              >
                <X className="h-4 w-4" />
              </button>
            </div>

            <p className="mt-5 text-base text-slate-300 leading-relaxed font-medium">
              {activeItem.content}
            </p>

            <div className="mt-6 grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-4 gap-3">
              {activeItem.bullets.map((bullet, i) => (
                <div
                  key={i}
                  className="flex items-center gap-2.5 rounded-xl border border-white/15 bg-white/[0.04] p-3 text-xs font-semibold text-slate-200"
                >
                  <CheckCircle2 className="h-4 w-4 text-[#EA580C] shrink-0" />
                  <span>{bullet}</span>
                </div>
              ))}
            </div>

            <div className="mt-7 pt-6 border-t border-white/10 flex items-center justify-between">
              <span className="text-xs font-semibold text-slate-400">
                Advayam Core Platform
              </span>
              <a
                href={activeItem.link}
                className="inline-flex items-center gap-2 rounded-xl bg-gradient-to-r from-[#EA580C] to-[#F97316] px-6 py-3 text-xs font-bold text-white shadow-md hover:scale-[1.02] transition"
              >
                <span>{activeItem.linkText}</span>
                <ArrowRight className="h-4 w-4" />
              </a>
            </div>
          </div>
        )}

      </div>
    </section>
  );
}
