import { Cpu, Download } from "lucide-react";

const features = [
  "Project templates",
  "Simulator Integration",
  "Board management",
  "Integrated Development Workflow",
  "Documentation Access",
  "Future OTA Deployment Support",
];

export default function CommunityShowcase() {
  return (
    <section className="relative overflow-hidden bg-transparent px-6 py-24 text-white">
      <div className="mx-auto flex max-w-4xl flex-col items-center text-center">
        <img
          src="/logos/advayam-logo.png"
          //alt="Advayam Logo"
          className="h-20 w-auto"
        />

        <h2 className="font-jakarta mt-6 max-w-3xl text-3xl font-black leading-tight text-white md:text-4xl">
          One Ecosystem for Learning, Building, and Deploying Embedded Systems
        </h2>

        <p className="font-jakarta mt-8 max-w-3xl text-lg font-medium leading-relaxed text-slate-300">
          A professional embedded development workflow designed for setup,
          project creation, simulation, documentation, and deployment.
        </p>

        <a
          href="/support"
          className="font-jakarta mt-10 inline-flex items-center gap-2.5 rounded-full border-2 border-[#BE5209] bg-[#BE5209] px-6 py-3.5 text-sm font-bold text-white shadow-xl shadow-orange-500/25 transition hover:bg-orange-600 hover:scale-[1.02]"
        >
          <span className="flex h-10 w-10 items-center justify-center rounded-full border border-white/30">
            <Download className="h-4 w-4 text-white" />
          </span>
          Get Advayam Now
        </a>

        <p className="font-jakarta mt-4 text-base font-bold text-white">
          Open source for curious minds.
        </p>

        <div className="mt-12 flex max-w-4xl flex-wrap justify-center gap-3.5">
          {features.map((feature) => (
            <span
              key={feature}
              className="font-jakarta inline-flex items-center justify-center gap-3 rounded-full border border-white/15 bg-white/[0.04] backdrop-blur-md px-5 py-2.5 text-sm font-medium text-slate-200 hover:border-white/30 hover:bg-white/[0.08] transition"
            >
              <span className="flex h-7 w-7 items-center justify-center rounded-full border border-white/20 text-orange-400">
                <Cpu className="h-3.5 w-3.5" />
              </span>
              {feature}
            </span>
          ))}
        </div>
      </div>
    </section>
  );
}
