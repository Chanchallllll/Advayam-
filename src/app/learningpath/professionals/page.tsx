import type { Metadata } from "next";
import Header from "@/components/Header";
import Footer from "@/components/Footer";

export const metadata: Metadata = {
  title: "Professionals",
  description: "Build scalable embedded products for industry.",
};

const modules = [
  "Industrial Automation",
  "IoT Product Development",
  "RTOS Firmware",
  "Edge AI Readiness",
  "Testing & Debugging",
  "Deployment Workflows",
];

export default function ProfessionalsPage() {
  return (
    <main className="flex min-h-screen flex-col">
      <Header />
      <section className="px-6 pb-20 pt-40">
        <div className="mx-auto max-w-8xl">
          <p className="font-bold text-[#c85a0a]">Professionals</p>
          <h1 className="mt-4 max-w-5xl text-6xl font-black">
            Build scalable embedded products for industry.
          </h1>
          <div className="mt-12 grid gap-6 md:grid-cols-3">
            {modules.map((m) => (
              <div
                key={m}
                className="rounded-[2rem] border-2 border-black p-8 hover:bg-[#DBDBDB]"
              >
                <h2 className="text-2xl font-black">{m}</h2>
                <p className="mt-4 leading-7">
                  Learn production-focused workflows using Advayam hardware,
                  documentation, and deployment-ready practices.
                </p>
              </div>
            ))}
          </div>
        </div>
      </section>
      <Footer />
    </main>
  );
}
