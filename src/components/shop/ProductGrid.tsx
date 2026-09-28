"use client";

import type { Product } from "@/data/products";
import ProductCard from "./ProductCard";

export default function ProductGrid({ products }: { products: Product[] }) {
  return (
    <div className="relative h-full min-h-0 overflow-hidden">
      <div className="pointer-events-none absolute left-0 right-0 top-0 z-10 h-16 bg-gradient-to-b from-[#FFFFFF] to-transparent" />
      <div className="pointer-events-none absolute bottom-0 left-0 right-0 z-10 h-20 bg-gradient-to-t from-[#FFFFFF] to-transparent" />

      <div className="doc-scrollbar h-full overflow-y-auto pb-24 pt-5">
        {products.length === 0 ? (
          <div className="flex h-full items-center justify-center text-center">
            <p className="font-jakarta text-xl font-semibold text-[#202020]/60">
              No products match this filter.
            </p>
          </div>
        ) : (
          <div className="grid w-[978px] grid-cols-[302px_302px_302px] gap-x-[36px] gap-y-8">
            {products.map((product) => (
              <ProductCard key={product.id} product={product} />
            ))}
          </div>
        )}
      </div>
    </div>
  );
}
