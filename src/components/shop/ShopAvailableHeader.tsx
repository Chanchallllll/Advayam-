"use client";

import {
  Heart,
  Menu,
  Search,
  ShoppingCart,
  SlidersHorizontal,
  X,
} from "lucide-react";
import { useState } from "react";
import type { PriceFilter } from "@/data/products";
import { priceFilters } from "@/data/products";
import { useShop } from "@/context/ShopContext";

export default function ShopAvailableHeader({
  sidebarOpen,
  filterOpen,
  setFilterOpen,
  priceFilter,
  setPriceFilter,
  favouriteMode,
  setFavouriteMode,
  searchQuery,
  setSearchQuery,
}: {
  sidebarOpen: boolean;
  filterOpen: boolean;
  setFilterOpen: (value: boolean) => void;
  priceFilter: PriceFilter;
  setPriceFilter: (value: PriceFilter) => void;
  favouriteMode: boolean;
  setFavouriteMode: (value: boolean) => void;
  searchQuery: string;
  setSearchQuery: (value: string) => void;
}) {
  const { totalCartItems } = useShop();
  const [mobileMenuOpen, setMobileMenuOpen] = useState(false);

  return (
    <header className="absolute left-0 right-10 top-6 z-50">
      <div
        className={`grid w-full items-start transition-all duration-300 ${
          sidebarOpen
            ? "grid-cols-[150px_1fr_150px] gap-x-[72px]"
            : "grid-cols-[150px_1fr_150px] gap-x-[36px]"
        } max-[1180px]:grid-cols-[110px_1fr_52px] max-[1180px]:gap-x-4`}
      >
        <a href="/" className="flex justify-start">
          <img
            src="/logos/advayam-logo.svg"
            alt="Advayam"
            className="h-20 md:h-24 w-auto object-contain transition-transform hover:scale-105"
          />
        </a>

        <div className="max-[1180px]:hidden">
          <HeaderSearch
            sidebarOpen={sidebarOpen}
            filterOpen={filterOpen}
            setFilterOpen={setFilterOpen}
            priceFilter={priceFilter}
            setPriceFilter={setPriceFilter}
            searchQuery={searchQuery}
            setSearchQuery={setSearchQuery}
          />
        </div>

        <div
          className={`flex items-center justify-end transition-all duration-300 max-[1180px]:hidden ${
            sidebarOpen ? "gap-1.5" : "gap-3"
          }`}
        >
          <button
            type="button"
            onClick={() => setFavouriteMode(!favouriteMode)}
            className={`flex h-10 w-10 shrink-0 items-center justify-center rounded-full border-2 border-[#202020] transition ${
              favouriteMode ? "bg-[#BE5209] text-[#FFFFFF]" : ""
            }`}
          >
            <img
              src="/icons/heart-icon.svg"
              className={`h-4 w-4 ${
                favouriteMode ? "fill-[#FFFFFF]" : "fill-none"
              }`}
            />
          </button>

          <a
            href="/shop_available/cart"
            className="relative flex h-10 w-10 shrink-0 items-center justify-center rounded-full border-2 border-[#202020]"
          >
            <img src="/icons/shopcart-icon.svg" className="h-4 w-4" />

            {totalCartItems > 0 && (
              <span className="absolute -right-1 -top-1 flex h-5 w-5 items-center justify-center rounded-full bg-[#BE5209] font-jakarta text-[10px] font-bold text-[#FFFFFF]">
                {totalCartItems}
              </span>
            )}
          </a>

          <a
            href="/auth/login"
            className="shrink-0 whitespace-nowrap font-jakarta text-[13px] font-bold"
          >
            Sign in
          </a>
        </div>

        <button
          type="button"
          onClick={() => setMobileMenuOpen(!mobileMenuOpen)}
          className="hidden h-11 w-11 items-center justify-center rounded-full border-2 border-[#202020] max-[1180px]:flex"
          aria-label="Toggle shop menu"
        >
          {mobileMenuOpen ? (
            <X className="h-5 w-5" />
          ) : (
            <Menu className="h-5 w-5" />
          )}
        </button>
      </div>

      {mobileMenuOpen && (
        <div className="absolute left-0 right-0 top-[70px] z-50 rounded-[2rem] border-2 border-[#202020] bg-[#E2E2E2] p-4 shadow-xl min-[1181px]:hidden">
          <HeaderSearch
            sidebarOpen={sidebarOpen}
            filterOpen={filterOpen}
            setFilterOpen={setFilterOpen}
            priceFilter={priceFilter}
            setPriceFilter={setPriceFilter}
            searchQuery={searchQuery}
            setSearchQuery={setSearchQuery}
          />

          <div className="mt-4 grid grid-cols-3 gap-3 max-sm:grid-cols-1">
            <button
              type="button"
              onClick={() => setFavouriteMode(!favouriteMode)}
              className={`flex h-10 items-center justify-center gap-2 rounded-full border-2 border-[#202020] font-jakarta text-sm font-semibold ${
                favouriteMode ? "bg-[#BE5209] text-[#FFFFFF]" : ""
              }`}
            >
              <Heart
                className={`h-4 w-4 ${
                  favouriteMode ? "fill-[#FFFFFF]" : "fill-none"
                }`}
              />
              Favourites
            </button>

            <a
              href="/shop_available/cart"
              className="flex h-10 items-center justify-center gap-2 rounded-full border-2 border-[#202020] font-jakarta text-sm font-semibold"
            >
              <ShoppingCart className="h-4 w-4" />
              Cart {totalCartItems > 0 && `(${totalCartItems})`}
            </a>

            <a
              href="/auth/login"
              className="flex h-10 items-center justify-center rounded-full bg-[#BE5209] font-jakarta text-sm font-semibold text-[#FFFFFF]"
            >
              Sign in
            </a>
          </div>
        </div>
      )}
    </header>
  );
}

function HeaderSearch({
  sidebarOpen,
  filterOpen,
  setFilterOpen,
  priceFilter,
  setPriceFilter,
  searchQuery,
  setSearchQuery,
}: {
  sidebarOpen: boolean;
  filterOpen: boolean;
  setFilterOpen: (value: boolean) => void;
  priceFilter: PriceFilter;
  setPriceFilter: (value: PriceFilter) => void;
  searchQuery: string;
  setSearchQuery: (value: string) => void;
}) {
  return (
    <div>
      <div
        className={`flex items-center transition-all duration-300 ${
          sidebarOpen ? "gap-2" : "gap-4"
        }`}
      >
        <button
          type="button"
          onClick={() => setFilterOpen(!filterOpen)}
          className={`flex h-11 w-11 shrink-0 items-center justify-center rounded-full border-2 border-[#202020] transition ${
            filterOpen ? "bg-[#BE5209] text-[#FFFFFF]" : ""
          }`}
        >
          {filterOpen ? (
            <img src="/icons/filter-selected-icon.svg" className="h-5 w-5" />
          ) : (
            <img src="/icons/filter-icon.svg" className="h-5 w-5" />
          )}
        </button>

        <form className="flex h-11 min-w-0 flex-1 items-center rounded-full border-2 border-[#202020] px-5">
          <input
            type="search"
            placeholder="Search"
            value={searchQuery}
            onChange={(e) => setSearchQuery(e.target.value)}
            className="w-full bg-transparent font-jakarta text-sm outline-none placeholder:text-[#202020]/60"
          />
        </form>

        <button className="flex h-11 w-11 shrink-0 items-center justify-center rounded-full border-2 border-[#202020]">
          <img src="/icons/search-shop-icon.svg" className="h-5 w-5" />
        </button>
      </div>

      {filterOpen && (
        <div className="ml-12 mt-3 flex flex-wrap gap-2">
          {priceFilters.map((filter) => (
            <button
              key={filter.value}
              type="button"
              onClick={() => setPriceFilter(filter.value)}
              className={`rounded-full border border-[#202020] px-4 py-1 font-jakarta text-xs transition ${
                priceFilter === filter.value
                  ? "bg-[#BE5209] text-[#FFFFFF]"
                  : "bg-transparent text-[#202020]"
              }`}
            >
              {filter.label}
            </button>
          ))}
        </div>
      )}
    </div>
  );
}
