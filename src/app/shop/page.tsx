"use client";

import { useState } from "react";
import {
  ChevronDown,
  Heart,
  Menu,
  Search,
  ShoppingCart,
  SlidersHorizontal,
  ArrowLeft,
  AlertCircle,
} from "lucide-react";
import ShopHeader from "@/components/ShopHeader";

const categories = [
  "Core Boards",
  "Base Boards",
  "Starter Kits",
  "Sensors",
  "New Arrivals",
];

export default function ShopPage() {
  const [sidebarOpen, setSidebarOpen] = useState(true);
  const [activeCategory, setActiveCategory] = useState("Core Boards");

  return (
    <main className="min-h-[120vh]overflow-hidden bg-[#FFFFFF] text-[#202020]">
      <ShopHeader sidebarOpen={sidebarOpen} />

      <aside
        className={`fixed left-4 top-4 z-40 flex max-h-[calc(100vh-2rem)] min-h-[420px] flex-col overflow-hidden rounded-[2rem] bg-[#dbdbdb] px-6 py-6 transition-all duration-300 ${
          sidebarOpen
            ? "h-[calc(100vh-2rem)] w-[300px]"
            : "h-[calc(100vh-2rem)] w-[72px]"
        }`}
      >
        <button
          onClick={() => setSidebarOpen(!sidebarOpen)}
          className="mb-6 flex items-center gap-4 font-jakarta text-base font-bold"
        >
          <img src="/icons/cpu-icon.svg" alt="CPU" className="h-5 w-5" />
          {sidebarOpen && (
            <>
              Categories
              <ChevronDown className="h-4 w-4" />
            </>
          )}
        </button>

        {sidebarOpen && (
          <div className="relative ml-2 mt-2 flex min-h-0 flex-1 overflow-hidden">
            {/* Branch SVG */}
            <svg
              viewBox="0 0 28 185"
              preserveAspectRatio="xMinYMid meet"
              className="absolute left-0 top-0 h-full max-h-[200px] w-[28px]"
              fill="none"
              xmlns="http://www.w3.org/2000/svg"
            >
              <path d="M0.5 0.5V160.5" stroke="#202020" strokeLinecap="round" />

              <path
                d="M27 25C19.2 25 15.2 25 12 23.8C7.3 22 3.6 18.3 1.8 13.6C0.5 10.5 0.5 6.5 0.5 0.5"
                stroke="#202020"
                strokeLinecap="round"
              />

              <path
                d="M27 60C19.2 60 15.2 60 12 58.8C7.3 57 3.6 53.3 1.8 48.6C0.5 45.5 0.5 41.5 0.5 35.5"
                stroke="#202020"
                strokeLinecap="round"
              />

              <path
                d="M27 95C19.2 95 15.2 95 12 93.8C7.3 92 3.6 88.3 1.8 83.6C0.5 80.5 0.5 76.5 0.5 70.5"
                stroke="#202020"
                strokeLinecap="round"
              />

              <path
                d="M27 130C19.2 130 15.2 130 12 128.8C7.3 127 3.6 123.3 1.8 118.6C0.5 115.5 0.5 111.5 0.5 105.5"
                stroke="#202020"
                strokeLinecap="round"
              />

              <path
                d="M27 165C19.2 165 15.2 165 12 163.8C7.3 162 3.6 158.3 1.8 153.6C0.5 150.5 0.5 146.5 0.5 140.5"
                stroke="#202020"
                strokeLinecap="round"
              />
            </svg>
            {/* Category buttons aligned with SVG branches */}
            <div className="absolute left-10 top-[6px] flex h-full max-h-[185px] flex-col justify-between py-[2px]">
              {categories.map((category) => (
                <button
                  key={category}
                  onClick={() => setActiveCategory(category)}
                  className="flex h-[20px] items-center text-left font-jakarta text-[15px] font-medium text-[#202020]/75"
                >
                  <span
                    className={`rounded-full px-5 py-1.5 transition ${
                      activeCategory === category
                        ? "bg-[#FFFFFF] text-[#202020]"
                        : "hover:bg-[#FFFFFF]/60"
                    }`}
                  >
                    {category}
                  </span>
                </button>
              ))}
            </div>
          </div>
        )}

        <button
          onClick={() => window.history.back()}
          className="mt-5 flex h-10 shrink-0 items-center justify-center gap-3 rounded-full border-2 border-[#202020] font-jakarta text-base font-semibold transition hover:bg-[#FFFFFF]"
        >
          <img src="/icons/back-icon.svg" alt="Back" className="h-5 w-5" />
          {sidebarOpen && "Back"}
        </button>
      </aside>

      <section
        className={`min-h-screen transition-all duration-300 ${
          sidebarOpen ? "pl-[330px]" : "pl-[100px]"
        }`}
      >
        <div className="flex min-h-screen items-center justify-center">
          <div className="flex flex-col items-center text-center">
            <div className="flex h-14 w-14 items-center justify-center rounded-full border-2 border-[#202020]">
              <img
                src="/icons/alert-icon.svg"
                alt="Alert"
                className="h-5 w-5"
              />
            </div>

            <p className="font-jakarta mt-5 text-xl font-medium">
              Products will be available soon!
            </p>
          </div>
        </div>
      </section>
    </main>
  );
}
