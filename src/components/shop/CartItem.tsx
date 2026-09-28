"use client";

import { Minus, Plus, Trash2 } from "lucide-react";
import type { Product } from "@/data/products";
import { useShop } from "@/context/ShopContext";

export default function CartItem({
  product,
  quantity,
}: {
  product: Product;
  quantity: number;
}) {
  const { increaseCart, decreaseCart, removeFromCart } = useShop();

  return (
    <article className="grid grid-cols-[140px_1fr_160px_80px] items-center gap-6 rounded-[28px] border-2 border-[#202020] bg-[#FFFFFF] p-4">
      <div className="flex h-[105px] items-center justify-center rounded-[24px] border border-[#202020]">
        <img
          src={product.image}
          alt={product.name}
          className="h-[75px] w-auto object-contain"
        />
      </div>

      <div>
        <h3 className="font-jakarta text-2xl font-black">{product.name}</h3>
        <p className="font-jakarta mt-2 text-lg font-bold">
          ₹{product.price.toLocaleString("en-IN")}
        </p>
      </div>

      <div className="flex h-11 items-center justify-between rounded-full bg-[#BE5209] px-5 font-jakarta font-bold text-[#FFFFFF]">
        <button
          type="button"
          onClick={() => decreaseCart(product.id)}
          aria-label="Decrease quantity"
        >
          <Minus className="h-4 w-4" />
        </button>

        <span>{quantity}</span>

        <button
          type="button"
          onClick={() => increaseCart(product.id)}
          aria-label="Increase quantity"
        >
          <Plus className="h-4 w-4" />
        </button>
      </div>

      <button
        type="button"
        onClick={() => removeFromCart(product.id)}
        className="flex h-11 w-11 items-center justify-center rounded-full border-2 border-[#202020]"
        aria-label="Remove item"
      >
        <Trash2 className="h-5 w-5" />
      </button>
    </article>
  );
}
