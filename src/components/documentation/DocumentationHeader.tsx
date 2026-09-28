"use client";

import { Menu, Search, ShoppingCart, X } from "lucide-react";
import { useState } from "react";

export default function DocumentationHeader() {
  const [open, setOpen] = useState(false);

  return (
    <header className="relative z-50 h-[76px] border-b border-white/10 bg-[#0c0c0c] px-3 py-3 text-white sm:h-[84px] sm:px-5 md:px-6">
      <div className="flex h-full items-center">

        {/* LOGO */}
        <a
          href="/"
          className="flex shrink-0 items-center"
        >
          <img
            src="/logos/advayam-logo.png"
            alt="Advayam"
            className="h-10 w-auto object-contain transition-transform hover:scale-105 sm:h-12 md:h-14"
          />
        </a>

        {/* DESKTOP SEARCH */}
        <div className="mx-auto hidden w-full max-w-[700px] items-center gap-3 lg:flex">
          <form className="flex h-11 min-w-0 flex-1 items-center rounded-full border border-white/20 bg-white/[0.06] px-5 transition-colors focus-within:border-[#BE5209]">
            <input
              type="search"
              placeholder="Explore Advayam"
              className="w-full bg-transparent font-jakarta text-sm text-white outline-none placeholder:text-white/50"
            />

            <button
              type="submit"
              aria-label="Search"
              className="flex h-8 w-8 shrink-0 items-center justify-center rounded-full text-white/60 transition hover:bg-white/10 hover:text-[#EA580C]"
            >
              <Search className="h-4 w-4" />
            </button>
          </form>
        </div>

        {/* DESKTOP ACTIONS */}
        <div className="ml-auto hidden items-center gap-2 lg:flex">
          <a
            href="/shop"
            className="rounded-full bg-gradient-to-r from-[#EA580C] to-[#F97316] px-6 py-3 font-jakarta text-sm font-bold text-white shadow-md shadow-orange-500/20 transition hover:scale-[1.03] active:scale-[0.98]"
          >
            Shop
          </a>

          <a
            href="/shop"
            aria-label="Shopping cart"
            className="flex h-11 w-11 items-center justify-center rounded-full border border-white/20 bg-white/[0.06] text-white transition hover:bg-white/10 hover:text-[#EA580C]"
          >
            <ShoppingCart className="h-5 w-5" />
          </a>
        </div>

        {/* MOBILE BUTTON */}
        <button
          type="button"
          onClick={() => setOpen((value) => !value)}
          aria-label={open ? "Close menu" : "Open menu"}
          aria-expanded={open}
          className="relative z-[9999] ml-auto flex h-11 w-11 items-center justify-center rounded-full border border-white/20 bg-white/[0.06] text-white transition hover:bg-white/10 active:scale-95 lg:hidden"
        >
          {open ? (
            <X className="h-5 w-5" />
          ) : (
            <Menu className="h-5 w-5" />
          )}
        </button>
      </div>

      {/* MOBILE MENU */}
      {open && (
        <div className="absolute left-2 right-2 top-[calc(100%+8px)] z-[9999] rounded-2xl border border-white/15 bg-[#111111] p-4 shadow-2xl shadow-black/50 lg:hidden sm:left-3 sm:right-3 sm:rounded-3xl">
          
          <form className="flex h-11 items-center rounded-full border border-white/20 bg-white/[0.06] px-4 focus-within:border-[#BE5209]">
            <input
              type="search"
              placeholder="Explore Advayam"
              className="w-full bg-transparent font-jakarta text-sm text-white outline-none placeholder:text-white/50"
            />

            <button
              type="submit"
              aria-label="Search"
              className="text-white/60 hover:text-[#EA580C]"
            >
              <Search className="h-4 w-4" />
            </button>
          </form>

          <div className="mt-4 flex items-center justify-between border-t border-white/10 pt-4">
            <a
              href="/shop"
              onClick={() => setOpen(false)}
              className="rounded-full bg-gradient-to-r from-[#EA580C] to-[#F97316] px-6 py-3 font-jakarta text-sm font-bold text-white"
            >
              Shop
            </a>

            <a
              href="/shop"
              onClick={() => setOpen(false)}
              aria-label="Shopping cart"
              className="flex h-11 w-11 items-center justify-center rounded-full border border-white/20 bg-white/[0.06]"
            >
              <ShoppingCart className="h-5 w-5" />
            </a>
          </div>
        </div>
      )}
    </header>
  );
}