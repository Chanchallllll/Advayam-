"use client";

import { Heart, ShoppingCart } from "lucide-react";
import type { Product } from "@/data/products";
import { useShop } from "@/context/ShopContext";

export default function ProductCard({ product }: { product: Product }) {
  const {
    favourites,
    toggleFavourite,
    addToCart,
    increaseCart,
    decreaseCart,
    getCartCount,
  } = useShop();

  const isFavourite = favourites.includes(product.id);
  const quantity = getCartCount(product.id);

  return (
    <article className="h-[312px] w-[302px] shrink-0 rounded-[28px] border-2 border-[#202020] bg-[#FFFFFF] p-[7px]">
      <div className="relative flex h-[166px] w-[286px] items-center justify-center rounded-[28px] border border-[#202020]">
        {product.isNew && (
          <span className="absolute left-4 top-3 rounded-full border border-[#202020] px-3 py-1 font-jakarta text-[10px] font-medium">
            New
          </span>
        )}

        <button
          type="button"
          onClick={() => toggleFavourite(product.id)}
          className={`absolute right-4 top-3 flex h-7 w-7 items-center justify-center rounded-full border border-[#202020] transition ${
            isFavourite ? "bg-[#BE5209] text-[#FFFFFF]" : "text-[#202020]"
          }`}
        >
          <Heart
            className={`h-4 w-4 ${isFavourite ? "fill-[#FFFFFF]" : "fill-none"}`}
          />
        </button>

        <img
          src={product.image}
          alt={product.name}
          className="h-[105px] w-auto object-contain"
        />
      </div>

      <div className="px-3 pb-3 pt-4">
        <p className="font-jakarta text-lg font-black leading-none">
          ₹{product.price.toLocaleString("en-IN")}
        </p>

        <h3 className="mt-1 line-clamp-2 font-jakarta text-2xl font-black leading-none">
          {product.name}
        </h3>
      </div>

      <div className="grid grid-cols-2 gap-1 px-1">
        {quantity === 0 ? (
          <button
            type="button"
            onClick={() => addToCart(product.id)}
            className="flex h-10 items-center justify-center rounded-full border border-[#202020] transition hover:bg-[#E2E2E2]"
          >
            <ShoppingCart className="h-4 w-4" />
          </button>
        ) : (
          <div className="flex h-10 items-center justify-between rounded-full bg-[#BE5209] px-4 font-jakarta text-sm font-bold text-[#FFFFFF]">
            <button type="button" onClick={() => decreaseCart(product.id)}>
              -
            </button>
            <span>{quantity}</span>
            <button type="button" onClick={() => increaseCart(product.id)}>
              +
            </button>
          </div>
        )}

        <button className="h-10 rounded-full bg-[#BE5209] font-jakarta text-sm font-semibold text-white">
          Buy
        </button>
      </div>
    </article>
  );
}
