"use client";

import { useMemo, useState } from "react";
import {
  products,
  type PriceFilter,
  type ProductCategory,
} from "@/data/products";
import ShopAvailableHeader from "@/components/shop/ShopAvailableHeader";
import ShopSidebar from "@/components/shop/ShopSidebar";
import ProductGrid from "@/components/shop/ProductGrid";
import { useShop } from "@/context/ShopContext";
import { filterProducts } from "@/components/shop/ProductFilters";

export default function ShopAvailablePage() {
  const [sidebarOpen, setSidebarOpen] = useState(true);
  const [activeCategory, setActiveCategory] =
    useState<ProductCategory>("core-boards");
  const [filterOpen, setFilterOpen] = useState(false);
  const [priceFilter, setPriceFilter] = useState<PriceFilter>("all");
  const [favouriteMode, setFavouriteMode] = useState(false);
  const [searchQuery, setSearchQuery] = useState("");

  const { favourites } = useShop();

  const visibleProducts = useMemo(() => {
    return filterProducts({
      products,
      activeCategory,
      priceFilter,
      searchQuery,
      favouriteIds: favourites,
      favouriteOnly: favouriteMode,
    });
  }, [activeCategory, priceFilter, searchQuery, favourites, favouriteMode]);

  return (
    <main className="relative h-screen w-screen overflow-hidden bg-[#FFFFFF] text-[#202020]">
      <ShopSidebar
        sidebarOpen={sidebarOpen}
        setSidebarOpen={setSidebarOpen}
        activeCategory={activeCategory}
        setActiveCategory={setActiveCategory}
      />

      <section
        className={`absolute bottom-0 right-0 top-0 overflow-hidden transition-all duration-300 ${
          sidebarOpen ? "left-[460px]" : "left-[180px]"
        }`}
      >
        <ShopAvailableHeader
          sidebarOpen={sidebarOpen}
          filterOpen={filterOpen}
          setFilterOpen={setFilterOpen}
          priceFilter={priceFilter}
          setPriceFilter={setPriceFilter}
          favouriteMode={favouriteMode}
          setFavouriteMode={setFavouriteMode}
          searchQuery={searchQuery}
          setSearchQuery={setSearchQuery}
        />

        <div className="flex h-full flex-col overflow-hidden pt-[150px]">
          <h1 className="shrink-0 font-jakarta text-[64px] font-black leading-none">
            {favouriteMode ? "My Favourites" : "Our Products"}
          </h1>

          <div className="mt-12 min-h-0 flex-1 overflow-hidden">
            <ProductGrid products={visibleProducts} />
          </div>
        </div>
      </section>
    </main>
  );
}
