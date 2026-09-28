import Header from "@/components/Header";
import Footer from "@/components/Footer";
import DownloadsHero from "@/components/downloads/DownloadsHero";
import AdditionalDownloads from "@/components/downloads/AdditionalDownloads";
import VersionHistory from "@/components/downloads/VersionHistory";
import SystemRequirements from "@/components/downloads/SystemRequirements";
import InstallationGuide from "@/components/downloads/InstallationGuide";
import { Metadata } from "next";

export const metadata: Metadata = {
  title: "Downloads - Advayam IDE",
  description: "Download the latest Advayam IDE for Windows, VS Code extension, Board Support Packages, and Open Source GitHub repository.",
};

export default function DownloadsPage() {
  return (
    <main className="relative min-h-screen overflow-hidden bg-black text-white">
      <Header />

      <img
        src="/patterns/wave-left-yellow.svg"
        alt=""
        className="pointer-events-none absolute bottom-0 left-0 w-[420px] opacity-20"
      />

      <img
        src="/patterns/wave-right-yellow.svg"
        alt=""
        className="pointer-events-none absolute right-0 top-[120px] w-[520px] opacity-20"
      />

      <section className="relative z-10 mx-auto max-w-[1300px] px-6 md:px-8 pb-20 pt-[140px]">
        <DownloadsHero />
        <AdditionalDownloads />
        <VersionHistory />
        <SystemRequirements />
        <InstallationGuide />
      </section>

      <Footer />
    </main>
  );
}