import type { Metadata } from "next";
import Header from "@/components/Header";
import Footer from "@/components/Footer";

export const metadata: Metadata = {
  title: "Makers",
  description: "Create, experiment, and share hardware projects.",
};

const projects = [
  "DIY Automation",
  "Robotics",
  "Sensor Dashboards",
  "Home IoT",
  "Wireless Projects",
  "Custom Hardware Builds",
];

export default function MakersPage() {
  return (
    <main className="flex min-h-screen flex-col">
      <Header />
      <section className="px-6 pb-20 pt-40">
        <div className="mx-auto max-w-8xl">
          <p className="font-bold text-[#c85a0a]">Makers</p>
          <h1 className="mt-4 max-w-5xl text-6xl font-black">
            Create, experiment, and share hardware projects.
          </h1>
          <div className="mt-12 grid gap-6 md:grid-cols-3">
            {projects.map((p) => (
              <div
                key={p}
                className="rounded-[2rem] border-2 border-black p-8 hover:bg-[#DBDBDB]"
              >
                <h2 className="text-2xl font-black">{p}</h2>
                <p className="mt-4 leading-7">
                  Use Advayam boards, sample code, documentation, and community
                  support to build creative prototypes.
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
