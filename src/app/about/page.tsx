import type { Metadata } from "next";
import Header from "@/components/Header";
import Footer from "@/components/Footer";

export const metadata: Metadata = {
  title: "About",
  description: "Learn more about the Advayam ecosystem.",
};

export default function AboutPage() {
  return (
    <main className="flex min-h-screen flex-col">
      <Header />
      <section className="px-6 pb-20 pt-40">
        <div className="mx-auto max-w-8xl">
          <p className="font-bold text-[#c85a0a]">About Advayam</p>
          <p className="font-jakarta text-2xl font-bold">
            India’s embedded ecosystem.
          </p>
          <div className="mt-12 grid gap-6 md:grid-cols-2">
            {[
              "Vision",
              "Mission",
              "Engineering Philosophy",
              "Advayam vs Arduino",
              "Roadmap",
              "Leadership",
            ].map((i) => (
              <div key={i} className="rounded-[2rem] border-2 border-black p-8">
                <h2 className="text-3xl font-black">{i}</h2>
                <p className="mt-4 leading-7">
                  Advayam empowers education, innovation, research, and product
                  development through indigenous hardware and software.
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
