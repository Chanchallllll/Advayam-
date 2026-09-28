"use client";

import {
  ArrowLeft,
  Settings,
  SlidersHorizontal,
} from "lucide-react";
import { categoryOptions, type ProductCategory } from "@/data/products";

export default function ShopSidebar({
  sidebarOpen,
  setSidebarOpen,
  activeCategory,
  setActiveCategory,
}: {
  sidebarOpen: boolean;
  setSidebarOpen: (value: boolean) => void;
  activeCategory: ProductCategory;
  setActiveCategory: (value: ProductCategory) => void;
}) {
  return (
    <aside
      className={`fixed left-4 top-3 z-40 flex h-[calc(100vh-24px)] max-h-[1056px] min-h-[560px] flex-col overflow-hidden rounded-[36px] bg-[#dbdbdb] transition-all duration-300 ${
        sidebarOpen
          ? "w-[clamp(300px,18.4vw,353px)] px-7 py-7"
          : "w-[clamp(82px,5.8vw,112px)] px-4 py-7"
      }`}
    >
      {/* Toggle */}
      <button
        type="button"
        onClick={() => setSidebarOpen(!sidebarOpen)}
        className={`flex h-11 items-center font-jakarta text-base font-black transition ${
          sidebarOpen
            ? "w-full justify-start gap-4"
            : "w-full justify-center"
        }`}
        aria-label="Toggle categories sidebar"
      >
        <img src="/icons/chip-icon.svg" className="h-5 w-5 shrink-0 *:" />
        {sidebarOpen && <span>Categories</span>}
      </button>

      {sidebarOpen && (
        <div className="relative ml-2 mt-5 h-[205px] shrink-0">
          <SidebarBranch />

          <div className="absolute left-10 top-[5px] flex flex-col gap-[7px]">
            {categoryOptions.map((category) => (
              <button
                key={category.value}
                type="button"
                onClick={() => setActiveCategory(category.value)}
                className={`w-fit rounded-full px-5 py-1 font-jakarta text-sm font-medium transition ${
                  activeCategory === category.value
                    ? "bg-[#FFFFFF] text-[#202020]"
                    : "text-[#202020]/70 hover:bg-[#FFFFFF]/60"
                }`}
              >
                {category.label}
              </button>
            ))}
          </div>
        </div>
      )}

      <div className="mt-auto space-y-8">
        {sidebarOpen && (
          <button className="flex items-center gap-4 font-jakarta text-base font-black">
           <img src="/icons/settings-icon.svg" className="h-5 w-5" />
            Settings
          </button>
        )}

        <button
          type="button"
          onClick={() => window.history.back()}
          className="flex h-11 w-full items-center justify-center gap-3 rounded-full border-2 border-[#202020] font-jakarta text-base font-semibold hover:bg-[#FFFFFF]"
        >
          <img src="/icons/back-icon.svg" className="h-4 w-4" />
          {sidebarOpen && "Back"}
        </button>
      </div>
    </aside>
  );
}

function SidebarBranch() {
  return (
    <svg
      width="28"
      height="205"
      viewBox="0 0 28 205"
      fill="none"
      xmlns="http://www.w3.org/2000/svg"
      className="absolute left-0 top-0"
    >
      <path d="M0.5 0.5V180.5" stroke="#202020" strokeLinecap="round" />
      <path d="M27 24C19.5 24 15.4 24 12.1 22.8C7.5 21.1 3.9 17.4 2 12.9C0.5 9.4 0.5 5.3 0.5 0.5" stroke="#202020" strokeLinecap="round" />
      <path d="M27 63C19.5 63 15.4 63 12.1 61.8C7.5 60.1 3.9 56.4 2 51.9C0.5 48.4 0.5 44.3 0.5 39.5" stroke="#202020" strokeLinecap="round" />
      <path d="M27 102C19.5 102 15.4 102 12.1 100.8C7.5 99.1 3.9 95.4 2 90.9C0.5 87.4 0.5 83.3 0.5 78.5" stroke="#202020" strokeLinecap="round" />
      <path d="M27 141C19.5 141 15.4 141 12.1 139.8C7.5 138.1 3.9 134.4 2 129.9C0.5 126.4 0.5 122.3 0.5 117.5" stroke="#202020" strokeLinecap="round" />
      <path d="M27 180C19.5 180 15.4 180 12.1 178.8C7.5 177.1 3.9 173.4 2 168.9C0.5 165.4 0.5 161.3 0.5 156.5" stroke="#202020" strokeLinecap="round" />
    </svg>
  );
}