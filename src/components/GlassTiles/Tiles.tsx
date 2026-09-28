"use client";

import Blob from "./Blob";
import GlassTile from "./GlassTile";

export default function Tiles() {
  return (
    <div className="glass-container">
      <Blob />

      <div className="tiles-grid">
        {Array.from({ length: 36 }).map((_, i) => (
          <GlassTile key={i} />
        ))}
      </div>
    </div>
  );
}