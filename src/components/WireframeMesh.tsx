"use client";

import { motion } from "framer-motion";
import { useEffect, useState } from "react";

interface PathData {
  d: string;
  opacity: string;
}

function MeshLayer({
  flipX,
  rotate180,
  slideFrom,
  baseDelay,
  strokeWidth,
  style,
}: {
  flipX?: boolean;
  rotate180?: boolean;
  slideFrom: number;
  baseDelay: number;
  strokeWidth: number;
  style?: React.CSSProperties;
}) {
  const [paths, setPaths] = useState<PathData[]>([]);

  useEffect(() => {
    fetch("/mesh.svg")
      .then((r) => r.text())
      .then((text) => {
        const parser = new DOMParser();
        const doc = parser.parseFromString(text, "image/svg+xml");
        const svgPaths = doc.querySelectorAll("path");
        const extracted: PathData[] = [];

        svgPaths.forEach((p) => {
          extracted.push({
            d: p.getAttribute("d") || "",
            opacity: p.getAttribute("opacity") || "1",
          });
        });

        setPaths(extracted);
      });
  }, []);

  if (paths.length === 0) return null;

  return (
    <svg
      viewBox="0 0 1562 1245"
      fill="none"
      xmlns="http://www.w3.org/2000/svg"
      style={{
        position: "absolute",
        overflow: "visible",
        ...style,
        transform: `
          ${flipX ? "scaleX(-1)" : ""}
          ${rotate180 ? "rotate(180deg)" : ""}
        `,
        transformOrigin: "center center",
      }}
    >
      <g opacity="0.24">
        {paths.map((p, i) => (
          <motion.path
            key={i}
            d={p.d}
            stroke="rgba(234, 88, 12, 0.45)"
            strokeWidth={strokeWidth}
            fill="none"
            initial={{
              x: slideFrom,
              opacity: 0,
            }}
            animate={{
              x: 0,
              opacity: Number(p.opacity),
            }}
            transition={{
              duration: 1,
              delay: baseDelay + i * 0.03,
              ease: [0.22, 1, 0.36, 1],
            }}
          />
        ))}
      </g>
    </svg>
  );
}

export default function WireframeMesh() {
  return (
    <div
      style={{
        position: "absolute",
        inset: 0,
        overflow: "hidden",
        pointerEvents: "none",
      }}
    >
      {/* ── LEFT SIDE ── */}
      <MeshLayer
        slideFrom={-250}
        baseDelay={0}
        strokeWidth={1.5}
        style={{
          left: "-24%",
          bottom: "10%",
          height: "750px",
          width: "auto",
        }}
      />
      {/* ── RIGHT SIDE ── */}
      <MeshLayer
        rotate180
        slideFrom={-250}
        baseDelay={0}
        strokeWidth={1.5}
        style={{
          right: "-24%",
          bottom: "-10%",
          height: "750px",
          width: "auto",
        }}
      />
    </div>
  );
}
