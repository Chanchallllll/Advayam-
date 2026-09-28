import { ArrowRight, Cpu, Code2, GraduationCap, FolderGit2, Users } from "lucide-react";

export default function BuildEcosystem({ dark = false }: { dark?: boolean }) {
  const buildCards = [
    {
      icon: Cpu,
      badgeBg: dark
        ? "bg-orange-500/10 border-orange-500/20 text-[#F97316]"
        : "bg-orange-500/10 border-orange-500/20 text-[#EA580C]",
      title: "Hardware",
      description: "High-performance PCBs designed for makers, engineers & innovators.",
      linkText: "Explore Hardware",
      linkColor: dark ? "text-[#F97316]" : "text-[#EA580C]",
      href: "/hardware",
    },
    {
      icon: Code2,
      badgeBg: dark
        ? "bg-cyan-500/10 border-cyan-500/20 text-cyan-400"
        : "bg-sky-500/10 border-sky-500/20 text-sky-600",
      title: "Software",
      description: "Production firmware, libraries, SDKs and example code.",
      linkText: "Explore Software",
      linkColor: dark ? "text-cyan-400" : "text-sky-600",
      href: "/software",
    },
    {
      icon: GraduationCap,
      badgeBg: dark
        ? "bg-emerald-500/10 border-emerald-500/20 text-emerald-400"
        : "bg-emerald-500/10 border-emerald-500/20 text-emerald-600",
      title: "Learn",
      description: "Tutorials, guides, documentation and hands-on resources.",
      linkText: "Start Learning",
      linkColor: dark ? "text-emerald-400" : "text-emerald-600",
      href: "/learningpath",
    },
    {
      icon: FolderGit2,
      badgeBg: dark
        ? "bg-purple-500/10 border-purple-500/20 text-purple-400"
        : "bg-purple-500/10 border-purple-500/20 text-purple-600",
      title: "Projects",
      description: "Real-world reference designs, blueprints and projects.",
      linkText: "View Projects",
      linkColor: dark ? "text-purple-400" : "text-purple-600",
      href: "/community",
    },
    {
      icon: Users,
      badgeBg: dark
        ? "bg-amber-500/10 border-amber-500/20 text-amber-400"
        : "bg-amber-500/10 border-amber-500/20 text-amber-600",
      title: "Community",
      description: "Connect, collaborate and grow with the Advayam community.",
      linkText: "Join Community",
      linkColor: dark ? "text-amber-400" : "text-amber-600",
      href: "/community",
    },
  ];

  return (
    <section className={`relative w-full py-24 px-4 sm:px-6 font-jakarta transition-colors duration-300 ${
      dark ? "bg-[#070A0F] text-white" : "bg-slate-50 text-slate-900"
    }`}>
      <div className="mx-auto max-w-7xl">
        
        {/* Section Heading */}
        <div className="text-center mb-16">
          <div className="inline-flex items-center justify-center rounded-full border border-orange-500/30 bg-orange-500/10 px-4 py-1.5 text-xs font-bold uppercase tracking-wider text-[#EA580C] mb-4">
            Unified Ecosystem
          </div>
          <h2 className={`text-3xl sm:text-4xl md:text-5xl font-extrabold tracking-tight ${dark ? "text-white" : "text-slate-900"}`}>
            Everything You Need to{" "}
            <span className="bg-gradient-to-r from-[#EA580C] to-[#F97316] bg-clip-text text-transparent">
              Build
            </span>
          </h2>
          <p className={`mt-3 text-base sm:text-lg font-medium max-w-2xl mx-auto ${dark ? "text-slate-400" : "text-slate-600"}`}>
            Modular hardware, open libraries, interactive tutorials, and a vibrant developer network.
          </p>
        </div>

        {/* 5 Card Grid */}
        <div className="grid grid-cols-1 sm:grid-cols-2 md:grid-cols-3 lg:grid-cols-5 gap-6">
          {buildCards.map((card, idx) => {
            const Icon = card.icon;
            return (
              <a
                key={idx}
                href={card.href}
                className={`group relative flex flex-col justify-between rounded-3xl border p-7 transition-all duration-300 hover:-translate-y-2 overflow-hidden ${
                  dark
                    ? "border-slate-800/80 bg-[#0C111C]/90 text-white hover:border-orange-500/30 hover:bg-[#101726] shadow-xl hover:shadow-[0_20px_40px_-15px_rgba(249,115,22,0.15)]"
                    : "border-slate-200/80 bg-white/90 text-slate-900 hover:border-orange-500/30 shadow-[0_10px_30px_-10px_rgba(15,23,42,0.05)] hover:shadow-[0_20px_40px_-15px_rgba(234,88,12,0.15)] backdrop-blur-md"
                }`}
              >
                {/* Top Subtle Hover Accent */}
                <div className="absolute top-0 left-0 right-0 h-1 bg-gradient-to-r from-[#EA580C] to-[#F97316] opacity-0 transition-opacity duration-300 group-hover:opacity-100" />

                <div>
                  {/* Card Icon */}
                  <div
                    className={`inline-flex h-13 w-13 items-center justify-center rounded-2xl border ${card.badgeBg}`}
                  >
                    <Icon className="h-6 w-6" />
                  </div>

                  {/* Title */}
                  <h3 className={`mt-6 text-xl font-extrabold tracking-tight ${dark ? "text-white" : "text-slate-900"}`}>
                    {card.title}
                  </h3>

                  {/* Description */}
                  <p className={`mt-2.5 text-xs leading-relaxed font-medium ${dark ? "text-slate-400" : "text-slate-600"}`}>
                    {card.description}
                  </p>
                </div>

                {/* Link */}
                <div className="mt-8 flex items-center gap-1.5 text-xs font-bold">
                  <span className={`transition-transform duration-300 flex items-center gap-2 group-hover:translate-x-1 ${card.linkColor}`}>
                    {card.linkText}
                    <ArrowRight className="h-4 w-4" />
                  </span>
                </div>
              </a>
            );
          })}
        </div>

      </div>
    </section>
  );
}
