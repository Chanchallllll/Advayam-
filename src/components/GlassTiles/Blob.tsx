"use client";

import { useEffect, useRef } from "react";

export default function Blob() {
  const blobRef = useRef<HTMLDivElement>(null);

  useEffect(() => {
    const container = document.querySelector(
      ".glass-container"
    ) as HTMLDivElement;

    if (!container || !blobRef.current) return;

    const blob = blobRef.current;

    let idle = true;
    let mouseX = 0;
    let mouseY = 0;

    let x = 0;
    let y = 0;

    let targetX = 0;
    let targetY = 0;

    const getRandomTarget = () => {
      targetX = Math.random() * 500 - 250;
      targetY = Math.random() * 500 - 250;
    };

    getRandomTarget();

    const randomTargetInterval = setInterval(() => {
      if (idle) {
        getRandomTarget();
      }
    }, 1000);

    const handleMove = (e: MouseEvent) => {
      idle = false;

      const rect = container.getBoundingClientRect();

      mouseX = e.clientX - rect.left;
      mouseY = e.clientY - rect.top;
    };

    const handleLeave = () => {
      idle = true;
      getRandomTarget();
    };

    container.addEventListener("mousemove", handleMove);
    container.addEventListener("mouseleave", handleLeave);

    let frameId: number;

    const animate = () => {
      if (idle) {
        x += (targetX - x) * 0.03;
        y += (targetY - y) * 0.03;
      } else {
        const followX = mouseX - container.clientWidth / 2;
        const followY = mouseY - container.clientHeight / 2;

        x += (followX - x) * 0.08;
        y += (followY - y) * 0.08;
      }

      blob.style.transform = `translate(${x}px, ${y}px)`;

      frameId = requestAnimationFrame(animate);
    };

    animate();

    return () => {
      clearInterval(randomTargetInterval);
      cancelAnimationFrame(frameId);

      container.removeEventListener("mousemove", handleMove);
      container.removeEventListener("mouseleave", handleLeave);
    };
  }, []);

  return <div ref={blobRef} className="blob" />;
}