import { ArrowRight, Sparkles } from "lucide-react";

export default function MovementBanner({ dark = false }: { dark?: boolean }) {
  return (
    <section className={`relative w-full pb-16 sm:pb-24 px-4 sm:px-6 font-jakarta transition-colors duration-300 ${
      dark ? "bg-[#070A0F] text-white" : "bg-slate-50 text-slate-900"
    }`}>
      <div className="mx-auto max-w-7xl">
        <div className={`relative overflow-hidden rounded-[2.5rem] border p-8 sm:p-12 transition-all duration-300 ${
          dark
            ? "border-slate-800/90 bg-[#0B101A]/90 text-white shadow-2xl backdrop-blur-xl"
            : "border-slate-200/80 bg-white/90 text-slate-900 shadow-[0_20px_50px_-15px_rgba(15,23,42,0.06)] backdrop-blur-xl"
        }`}>
          
          {/* Orange Gradient Accent Glow behind card */}
          <div className="pointer-events-none absolute -right-20 -bottom-20 h-72 w-72 rounded-full bg-gradient-to-tr from-amber-500/20 via-orange-500/20 to-transparent blur-3xl opacity-80" />

          <div className="relative z-10 flex flex-col lg:flex-row items-start lg:items-center justify-between gap-8 sm:gap-10">
            
            {/* Left Content */}
            <div className="max-w-md">
              <div className="inline-flex items-center gap-2 rounded-full border border-orange-500/30 bg-orange-500/10 px-3.5 py-1 text-xs font-bold uppercase tracking-wider text-[#EA580C] mb-3">
                <Sparkles className="h-3.5 w-3.5" />
                <span>Join the Ecosystem</span>
              </div>
              <h3 className={`text-2xl sm:text-4xl font-extrabold tracking-tight ${dark ? "text-white" : "text-slate-900"}`}>
                Indigenous Platform. <span className="bg-gradient-to-r from-[#EA580C] to-[#F97316] bg-clip-text text-transparent">Future Ready.</span>
              </h3>
              <p className={`mt-2.5 text-sm sm:text-base leading-relaxed font-medium ${dark ? "text-slate-400" : "text-slate-600"}`}>
                Advayam is more than hardware — it's an innovation movement. Join thousands of builders shaping India's tech future.
              </p>
            </div>

            {/* Stats Breakdown */}
            <div className={`grid grid-cols-3 gap-4 sm:gap-12 py-6 lg:py-0 border-y lg:border-y-0 lg:border-x lg:px-12 w-full lg:w-auto text-center sm:text-left ${
              dark ? "border-slate-800/80" : "border-slate-200"
            }`}>
              
              {/* Stat 1 */}
              <div className="flex flex-col items-center sm:items-start">
                <span className="text-3xl sm:text-4xl font-extrabold tracking-tight text-slate-900 bg-gradient-to-r from-[#EA580C] to-[#F97316] bg-clip-text text-transparent">
                  1K+
                </span>
                <span className={`text-xs font-bold mt-1 uppercase tracking-wider ${dark ? "text-slate-400" : "text-slate-500"}`}>
                  Developers
                </span>
              </div>

              {/* Stat 2 */}
              <div className="flex flex-col items-center sm:items-start">
                <span className="text-3xl sm:text-4xl font-extrabold tracking-tight text-slate-900 bg-gradient-to-r from-[#EA580C] to-[#F97316] bg-clip-text text-transparent">
                  100+
                </span>
                <span className={`text-xs font-bold mt-1 uppercase tracking-wider ${dark ? "text-slate-400" : "text-slate-500"}`}>
                  SDK Libraries
                </span>
              </div>

              {/* Stat 3 */}
              <div className="flex flex-col items-center sm:items-start">
                <span className="text-3xl sm:text-4xl font-extrabold tracking-tight text-slate-900 bg-gradient-to-r from-[#EA580C] to-[#F97316] bg-clip-text text-transparent">
                  50+
                </span>
                <span className={`text-xs font-bold mt-1 uppercase tracking-wider ${dark ? "text-slate-400" : "text-slate-500"}`}>
                  Projects
                </span>
              </div>

            </div>

            {/* Right Action Button */}
            <div className="w-full lg:w-auto shrink-0">
              <a
                href="/community"
                className="inline-flex w-full lg:w-auto justify-center items-center gap-2.5 rounded-xl bg-gradient-to-r from-[#EA580C] to-[#F97316] px-8 py-4 text-sm font-bold text-white shadow-[0_8px_25px_-4px_rgba(234,88,12,0.35)] transition-all duration-200 hover:shadow-[0_12px_30px_-4px_rgba(234,88,12,0.45)] hover:scale-[1.02] active:scale-[0.99]"
              >
                <span>Join the Movement</span>
                <ArrowRight className="h-4 w-4" />
              </a>
            </div>

          </div>

        </div>
      </div>
    </section>
  );
}
