import { readFile } from "node:fs/promises";
import path from "node:path";
import Header from "@/components/Header";
import Footer from "@/components/Footer";
import { Metadata } from "next";

export const metadata: Metadata = {
  title: "Free to Use - Advayam IDE",
  description: "Free-to-use and legal information for Advayam IDE.",
};

export default async function FreeToUsePage() {
  const freeToUseText = await readFile(path.join(process.cwd(), "src/free-to-use.txt"), "utf8");

  return (
    <main className="min-h-screen bg-black text-white">
      <Header />
      <section className="mx-auto max-w-5xl px-6 pb-20 pt-[140px] md:px-8">
        <div className="rounded-3xl border border-white/10 bg-white/[0.06] p-6 shadow-2xl backdrop-blur-md md:p-10">
          <p className="font-jakarta text-xs font-bold uppercase tracking-[0.2em] text-[#BE5209]">
            Free to Use
          </p>
          <h1 className="mt-3 font-jakarta text-4xl font-black text-white md:text-5xl">
            Advayam IDE Usage Information
          </h1>
          <pre className="mt-8 whitespace-pre-wrap font-mono text-sm leading-7 text-slate-300">
            {freeToUseText}
          </pre>
        </div>
      </section>
      <Footer />
    </main>
  );
}