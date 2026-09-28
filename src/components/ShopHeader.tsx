import { Heart, Menu, Search, ShoppingCart } from "lucide-react";

export default function ShopHeader({ sidebarOpen }: { sidebarOpen: boolean }) {
  return (
    <header
      className={`fixed right-0 top-0 z-30 py-6 pr-6 transition-all duration-300 ${
        sidebarOpen ? "left-[380px]" : "left-[120px]"
      }`}
    >
      <div
        className={`grid w-full items-center transition-all duration-300 ${
          sidebarOpen
            ? "grid-cols-[140px_minmax(320px,1fr)_210px] gap-x-8"
            : "grid-cols-[140px_minmax(420px,1fr)_210px] gap-x-12"
        }`}
      >
        <a href="/" className="flex justify-center">
          <img
            src="/logos/advayam-logo-blue.svg"
            alt="Advayam"
            className="h-18 md:h-22 w-auto object-contain transition-transform hover:scale-105"
          />
        </a>

        <div
          className={`mx-auto flex w-full items-center transition-all duration-300 ${
            sidebarOpen ? "max-w-[620px] gap-3" : "max-w-[760px] gap-4"
          }`}
        >
          <button
            type="button"
            className="flex h-11 w-11 shrink-0 items-center justify-center rounded-full border-2 border-[#202020] hover:bg-[#DBDBDB]"
          >
            <img src="/icons/menu-icon.svg" alt="Menu" className="h-5 w-5" />
          </button>

          <form className="flex h-11 min-w-0 flex-1 items-center rounded-full border-2 border-[#202020] px-4">
            <input
              type="search"
              placeholder="Search"
              className="w-full bg-transparent font-jakarta outline-none placeholder:text-[#202020]/60"
            />
          </form>

          <button
            type="button"
            className="flex h-11 w-11 shrink-0 items-center justify-center rounded-full border-2 border-[#202020] hover:bg-[#DBDBDB]"
          >
            <img
              src="/icons/search-shop-icon.svg"
              alt="Search"
              className="h-5 w-5"
            />
          </button>
        </div>

        <div
          className={`flex items-center justify-end transition-all duration-300 ${
            sidebarOpen ? "gap-3" : "gap-4"
          }`}
        >
          <button className="flex h-11 w-11 items-center justify-center rounded-full border-2 border-[#202020] hover:bg-[#DBDBDB]">
            <img src="/icons/heart-icon.svg" alt="Heart" className="h-5 w-5" />
          </button>

          <button className="flex h-11 w-11 items-center justify-center rounded-full border-2 border-[#202020] hover:bg-[#DBDBDB]">
            <img
              src="/icons/shopcart-icon.svg"
              alt="Shopping Cart"
              className="h-5 w-5"
            />
          </button>

          <a
            href="/auth/login"
            className="whitespace-nowrap font-jakarta text-base font-bold"
          >
            Sign in
          </a>
        </div>
      </div>
    </header>
  );
}
