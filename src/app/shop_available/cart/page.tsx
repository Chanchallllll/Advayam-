"use client";

import Link from "next/link";
import { ArrowLeft, ShoppingBag } from "lucide-react";
import { products } from "@/data/products";
import { useShop } from "@/context/ShopContext";
import ProductCard from "@/components/shop/ProductCard";

export default function CartPage() {
  const { cart, totalCartItems } = useShop();

  const cartProducts = Object.entries(cart)
    .map(([id]) => products.find((product) => product.id === Number(id)))
    .filter(Boolean) as typeof products;

  const subtotal = cartProducts.reduce((sum, product) => {
    return sum + product.price * (cart[product.id] ?? 0);
  }, 0);

  return (
    <main className="h-screen overflow-hidden bg-[#FFFFFF] px-8 py-8 text-[#202020]">
      <div className="mx-auto flex h-full max-w-[1180px] flex-col overflow-hidden">
        <header className="shrink-0 flex items-center justify-between">
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

        <section className="mt-12 grid min-h-0 flex-1 gap-10 overflow-hidden lg:grid-cols-[1fr_360px]">
          <div className="min-h-0 overflow-hidden">
            <h1 className="shrink-0 font-jakarta text-6xl font-black">Cart</h1>

            <p className="mt-3 font-jakarta text-lg font-medium text-[#202020]/70">
              {totalCartItems} item{totalCartItems === 1 ? "" : "s"} in your
              cart
            </p>

            <div className="relative mt-8 h-[calc(100%-120px)] overflow-hidden">
              <div className="pointer-events-none absolute left-0 right-0 top-0 z-10 h-20 bg-gradient-to-b from-[#FFFFFF] to-transparent" />
              <div className="pointer-events-none absolute bottom-0 left-0 right-0 z-10 h-24 bg-gradient-to-t from-[#FFFFFF] to-transparent" />

              <div className="doc-scrollbar h-full overflow-y-auto px-2 pb-28 pt-5">
                {cartProducts.length === 0 ? (
                  <div className="flex h-full flex-col items-center justify-center rounded-[32px] border-2 border-[#202020] bg-[#E2E2E2] text-center">
                    <ShoppingBag className="h-14 w-14" />
                    <h2 className="mt-6 font-jakarta text-3xl font-black">
                      Your cart is empty
                    </h2>
                    <p className="mt-3 font-jakarta text-base font-medium text-[#202020]/70">
                      Add products from the shop to see them here.
                    </p>
                  </div>
                ) : (
                  <div className="grid grid-cols-1 gap-x-12 gap-y-8 md:grid-cols-2">
                    {cartProducts.map((product) => (
                      <ProductCard key={product.id} product={product} />
                    ))}
                  </div>
                )}
              </div>
            </div>
          </div>

          <aside className="h-fit rounded-[32px] border-2 border-[#202020] bg-[#E2E2E2] p-7">
            <h2 className="font-jakarta text-3xl font-black">Summary</h2>

            <div className="mt-8 space-y-4 font-jakarta text-base font-medium">
              <div className="flex justify-between">
                <span>Subtotal</span>
                <span>₹{subtotal.toLocaleString("en-IN")}</span>
              </div>

              <div className="flex justify-between">
                <span>Shipping</span>
                <span>Calculated later</span>
              </div>

              <div className="border-t-2 border-[#202020] pt-5">
                <div className="flex justify-between text-xl font-black">
                  <span>Total</span>
                  <span>₹{subtotal.toLocaleString("en-IN")}</span>
                </div>
              </div>
            </div>

            <button className="mt-8 h-12 w-full rounded-full bg-[#BE5209] font-jakarta font-semibold text-[#FFFFFF]">
              Checkout
            </button>
          </aside>
        </section>
      </div>
    </main>
  );
}
