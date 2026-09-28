"use client";

import { motion, useScroll, useTransform } from "framer-motion";

export default function HeroScrollBackground() {
  const { scrollY } = useScroll();

  // From scrollY 0 to 750px (as user scrolls down the hero towards Why Advayam):
  // Opacity: 1 at top, fades to 0 as Why Advayam approaches
  const opacity = useTransform(scrollY, [0, 400, 750], [1, 0.6, 0]);
  
  // Blur: 0px at top, progressively blurs up to 24px
  const blurValue = useTransform(scrollY, [0, 650], [0, 24]);
  const filter = useTransform(blurValue, (v) => `blur(${v}px)`);

  // Scale: subtle cinematic zoom from 1.0 to 1.06
  const scale = useTransform(scrollY, [0, 800], [1, 1.06]);

  return (
    <div className="fixed inset-0 pointer-events-none z-0 overflow-hidden" aria-hidden="true">
      {/* Solid Black Canvas Base that remains after image disappears */}
      <div className="absolute inset-0 bg-[#050505]" />

      {/* Animated Hardware Lab Background Layer */}
      <motion.div
        style={{
          opacity,
          filter,
          scale,
          backgroundImage: "url('/hero-bg.jpg')",
        }}
        className="absolute inset-0 bg-cover bg-center bg-no-repeat will-change-transform"
      >
        {/* Cinematic Dark Gradient Overlay for Maximum Readability */}
        <div className="absolute inset-0 bg-gradient-to-b from-black/75 via-black/60 to-[#050505]" />
        {/* Subtle Radial Vignette focusing attention on the center */}
        <div className="absolute inset-0 bg-[radial-gradient(circle_at_center,transparent_0%,rgba(0,0,0,0.65)_100%)]" />
      </motion.div>
    </div>
  );
}
