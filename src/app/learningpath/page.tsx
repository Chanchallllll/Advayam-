import type { Metadata } from "next";
import Header from "@/components/Header";
import Footer from "@/components/Footer";
import { GraduationCap, Briefcase, Wrench, CheckCircle2, ArrowRight, Sparkles } from "lucide-react";

export const metadata: Metadata = {
  title: "Learning Path",
  description: "Choose your embedded systems journey.",
};

const paths = [
  {
    id: "students",
    title: "Students & Education",
    subtitle: "For School (K-12), Undergraduates & Postgraduate Researchers.",
    icon: GraduationCap,
    badge: "Academic Track",
    badgeBg: "bg-orange-500/10 border-orange-500/20 text-[#EA580C]",
    href: "/learningpath/students",
    items: [
      "K-12 STEM & Electronics Foundations",
      "Undergraduate Microcontroller Labs",
      "Postgraduate & Research Projects",
      "Hands-on Hardware Kits & Curriculum",
    ],
  },
  {
    id: "professionals",
    title: "Professionals",
    subtitle: "For Industry Engineers, Firmware Developers & IoT Specialists.",
    icon: Briefcase,
    badge: "Industry Ready",
    badgeBg: "bg-purple-500/10 border-purple-500/20 text-purple-600",
    href: "/learningpath/professionals",
    items: [
      "Industrial Automation & Control",
      "IoT Product Development & APIs",
      "RTOS Firmware & Low-Latency Tasks",
      "Edge AI & On-Device Inference",
      "Hardware Testing & Debugging Workflows",
    ],
  },
  {
    id: "makers",
    title: "Makers & Innovators",
    subtitle: "For Hobbyists, Robotics Builders & DIY Hardware Creators.",
    icon: Wrench,
    badge: "Project First",
    badgeBg: "bg-emerald-500/10 border-emerald-500/20 text-emerald-600",
    href: "/learningpath/makers",
    items: [
      "DIY Smart Home & Automation",
      "Robotics & Motor Control Builds",
      "Sensor Dashboards & Telemetry",
      "Wireless Bluetooth & Wi-Fi Nodes",
      "Custom PCB & Prototyping Projects",
    ],
  },
];

export default function LearningPathPage() {
  return (
    <main className="flex min-h-screen flex-col bg-black text-white font-jakarta selection:bg-orange-500/20 selection:text-[#EA580C]">
      <Header />
      
      <section className="relative px-6 pt-36 pb-24 sm:pt-44 sm:pb-32 overflow-hidden">
        {/* Background Ambient Glows */}
        <div className="pointer-events-none absolute -top-40 left-1/3 h-[600px] w-[600px] rounded-full bg-gradient-to-br from-orange-500/15 via-amber-500/10 to-transparent blur-[140px]" />
        <div className="pointer-events-none absolute top-1/2 -right-40 h-[500px] w-[500px] rounded-full bg-gradient-to-bl from-amber-500/10 via-orange-400/10 to-transparent blur-[160px]" />

        <div className="relative z-10 mx-auto max-w-7xl">
          
          {/* Header Title Block */}
          <div className="max-w-3xl">
            <div className="inline-flex items-center gap-2 rounded-full border border-orange-500/20 bg-orange-500/10 px-4 py-1.5 text-xs font-semibold text-[#EA580C]">
              <Sparkles className="h-3.5 w-3.5" />
              <span>Tailored Learning Journeys</span>
            </div>

            <h1 className="mt-4 font-extrabold text-4xl sm:text-6xl tracking-tight text-slate-900">
              Choose Your Embedded Systems Journey
            </h1>

            <p className="mt-4 text-base sm:text-xl text-slate-600 font-medium leading-relaxed">
              Structured learning pathways engineered for students, industry engineers, and hardware makers.
            </p>
          </div>

          {/* 3-Column Learning Path Cards */}
          <div className="mt-14 grid grid-cols-1 md:grid-cols-3 gap-8">
            {paths.map((path) => {
              const Icon = path.icon;

              return (
                <div
                  key={path.id}
                  className="group relative flex flex-col justify-between rounded-3xl border border-slate-200/80 bg-white p-7 sm:p-8 shadow-sm transition-all duration-300 hover:shadow-xl hover:border-slate-300 hover:-translate-y-2"
                >
                  <div>
                    {/* Header Row */}
                    <div className="flex items-center justify-between">
                      <div className="flex h-12 w-12 items-center justify-center rounded-2xl bg-orange-500/10 text-[#EA580C] group-hover:scale-110 transition-transform duration-300">
                        <Icon className="h-6 w-6" />
                      </div>

                      <span className={`inline-flex items-center rounded-full border px-3 py-1 text-xs font-bold ${path.badgeBg}`}>
                        {path.badge}
                      </span>
                    </div>

                    {/* Title & Subtitle */}
                    <h2 className="mt-6 text-2xl font-extrabold text-slate-900 group-hover:text-[#EA580C] transition-colors">
                      {path.title}
                    </h2>

                    <p className="mt-2.5 text-xs sm:text-sm text-slate-500 font-semibold leading-relaxed">
                      {path.subtitle}
                    </p>

                    {/* Checklist Items */}
                    <div className="mt-7 pt-6 border-t border-slate-100 space-y-3">
                      {path.items.map((item, i) => (
                        <div key={i} className="flex items-start gap-2.5">
                          <CheckCircle2 className="h-4 w-4 shrink-0 text-[#EA580C] mt-0.5" />
                          <span className="text-xs font-semibold text-slate-700">
                            {item}
                          </span>
                        </div>
                      ))}
                    </div>
                  </div>

                  {/* CTA Action Button */}
                  <div className="mt-8 pt-6 border-t border-slate-100">
                    <a
                      href={path.href}
                      className="inline-flex w-full items-center justify-center gap-2 rounded-xl bg-gradient-to-r from-[#EA580C] to-[#F97316] px-6 py-3.5 text-xs sm:text-sm font-bold text-white shadow-md shadow-orange-500/20 transition-all duration-200 hover:shadow-lg hover:shadow-orange-500/30 hover:scale-[1.01] active:scale-[0.99]"
                    >
                      <span>Explore Learning Track</span>
                      <ArrowRight className="h-4 w-4 transition-transform duration-200 group-hover:translate-x-1" />
                    </a>
                  </div>

                </div>
              );
            })}
          </div>

        </div>
      </section>

      <Footer />
    </main>
  );
}
