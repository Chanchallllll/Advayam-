import Header from "@/components/Header";
import Footer from "@/components/Footer";
import Hero from "@/components/Hero";
import HeroScrollBackground from "@/components/HeroScrollBackground";
import WhyAdvayam from "@/components/WhyAdvayam";
import LearningPathShowcase from "@/components/LearningPathShowcase";
import EcosystemOverview from "@/components/EcosystemOverview";
import CommunityShowcase from "@/components/CommunityShowcase";

export default function HomePage() {
  return (
    <main className="relative flex min-h-screen flex-col overflow-x-hidden bg-[#050505] text-white">
      <HeroScrollBackground />
      <div className="relative z-10 flex flex-col flex-1">
        <Header />
        <Hero />
        <WhyAdvayam />
        <LearningPathShowcase />
        <EcosystemOverview />
        <CommunityShowcase />
        <Footer />
      </div>
    </main>
  );
}