"use client";

import Link from "next/link";
import { ArrowLeft, Heart } from "lucide-react";
import { useMemo, useState } from "react";
import { products, type PriceFilter } from "@/data/products";
import { useShop } from "@/context/ShopContext";
import ProductGrid from "@/components/shop/ProductGrid";
import { filterProducts } from "@/components/shop/ProductFilters";

export default function FavouritesPage() {
  const { favourites } = useShop();
  const [priceFilter, setPriceFilter] = useState<PriceFilter>("all");
  const [searchQuery, setSearchQuery] = useState("");

  const visibleProducts = useMemo(() => {
    return filterProducts({
      products,
      priceFilter,
      searchQuery,
      favouriteIds: favourites,
      favouriteOnly: true,
    });
  }, [favourites, priceFilter, searchQuery]);

  return (
    <main className="h-[1080px] overflow-hidden bg-[#FFFFFF] px-8 py-8 text-[#202020]">
      <header className="flex items-center justify-between">
        <Link
          href="/shop_available"
          className="flex h-12 items-center gap-3 rounded-full border-2 border-[#202020] px-6 font-jakarta font-semibold"
        >
          <ArrowLeft className="h-5 w-5" />
          Back to Shop
        </Link>

        <img
          src="/logos/advayam-logo.svg"
          alt="Advayam"
          className="h-14 w-auto"
        />
      </header>

      <section className="mx-auto mt-16 max-w-[1100px]">
        <div className="flex items-center justify-between gap-6">
          <div>
            <h1 className="font-jakarta text-6xl font-black">Favourites</h1>
            <p className="mt-3 font-jakarta text-base font-medium text-[#202020]/70">
              Products you saved for later.
            </p>
          </div>

          <div className="flex items-center gap-3">
            <input
              type="search"
              placeholder="Search favourites"
              value={searchQuery}
              onChange={(e) => setSearchQuery(e.target.value)}
              className="h-12 rounded-full border-2 border-[#202020] bg-transparent px-6 font-jakarta text-sm outline-none"
            />

            <select
              value={priceFilter}
              onChange={(e) => setPriceFilter(e.target.value as PriceFilter)}
              className="h-12 rounded-full border-2 border-[#202020] bg-transparent px-5 font-jakarta text-sm outline-none"
            >
              <option value="all">All</option>
              <option value="under-500">Under ₹500</option>
              <option value="above-500">Above ₹500</option>
            </select>
          </div>
        </div>

        <div className="mt-12">
          {favourites.length === 0 ? (
            <div className="flex h-[520px] flex-col items-center justify-center rounded-[32px] border-2 border-[#202020] bg-[#E2E2E2] text-center">
              <Heart className="h-14 w-14" />
              <h2 className="mt-6 font-jakarta text-3xl font-black">
                No favourites yet
              </h2>
              <p className="mt-3 font-jakarta text-base font-medium text-[#202020]/70">
                Tap the heart button on a product to save it here.
              </p>
            </div>
          ) : (
            <ProductGrid products={visibleProducts} />
          )}
        </div>
      </section>
    </main>
  );
}
