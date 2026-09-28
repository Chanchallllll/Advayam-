"use client";

import { useState } from "react";
import DocumentationHeader from "./DocumentationHeader";
import DocumentationSidebar from "./DocumentationSidebar";
import DocumentationContent from "./DocumentationContent";
import DocumentationScrollFade from "./DocumentationScrollFade";

export default function DocumentationLayout() {
  const [sidebarOpen, setSidebarOpen] = useState(true);

  return (
    <main className="min-h-screen bg-black p-2 text-white sm:p-3 md:p-4">
      {/* SIDEBAR */}
      <DocumentationSidebar
        sidebarOpen={sidebarOpen}
        setSidebarOpen={setSidebarOpen}
      />

      {/* MAIN DOCUMENTATION PANEL */}
      <section
        className={`fixed bottom-2 right-2 top-2 overflow-hidden rounded-2xl border border-white/10 bg-[#0c0c0c] shadow-2xl shadow-black/40 transition-all duration-300 sm:bottom-3 sm:right-3 sm:top-3 sm:rounded-3xl md:bottom-4 md:right-4 md:top-4 ${
          sidebarOpen
            ? "left-2 sm:left-3 md:left-[350px]"
            : "left-2 sm:left-3 md:left-[100px]"
        }`}
      >
        <DocumentationHeader />

        <div className="relative h-[calc(100%-76px)] overflow-hidden sm:h-[calc(100%-84px)]">
          <DocumentationScrollFade />

          <div className="doc-scrollbar h-full overflow-y-auto scroll-smooth px-4 sm:px-6 md:px-8 lg:px-10">
            <DocumentationContent />
          </div>
        </div>
      </section>
    </main>
  );
}