"use client";

import {
  createContext,
  useContext,
  useEffect,
  useMemo,
  useState,
  type ReactNode,
} from "react";

type ShopContextType = {
  favourites: number[];
  cart: Record<number, number>;

  toggleFavourite: (id: number) => void;

  addToCart: (id: number) => void;
  increaseCart: (id: number) => void;
  decreaseCart: (id: number) => void;
  removeFromCart: (id: number) => void;
  getCartCount: (id: number) => number;

  totalCartItems: number;
  isFavourite: (id: number) => boolean;
};

const ShopContext = createContext<ShopContextType | null>(null);

export function ShopProvider({ children }: { children: ReactNode }) {
  const [favourites, setFavourites] = useState<number[]>([]);
  const [cart, setCart] = useState<Record<number, number>>({});

  useEffect(() => {
    const savedCart = localStorage.getItem("advayam-cart");
    const savedFavourites = localStorage.getItem("advayam-favourites");

    if (savedCart) setCart(JSON.parse(savedCart));
    if (savedFavourites) setFavourites(JSON.parse(savedFavourites));
  }, []);

  useEffect(() => {
    localStorage.setItem("advayam-cart", JSON.stringify(cart));
  }, [cart]);

  useEffect(() => {
    localStorage.setItem("advayam-favourites", JSON.stringify(favourites));
  }, [favourites]);

  function toggleFavourite(id: number) {
    setFavourites((prev) =>
      prev.includes(id) ? prev.filter((item) => item !== id) : [...prev, id]
    );
  }

  function isFavourite(id: number) {
    return favourites.includes(id);
  }

  function addToCart(id: number) {
    setCart((prev) => ({
      ...prev,
      [id]: prev[id] ? prev[id] + 1 : 1,
    }));
  }

  function increaseCart(id: number) {
    addToCart(id);
  }

  function decreaseCart(id: number) {
    setCart((prev) => {
      const current = prev[id] ?? 0;

      if (current <= 1) {
        const updated = { ...prev };
        delete updated[id];
        return updated;
      }

      return {
        ...prev,
        [id]: current - 1,
      };
    });
  }

  function removeFromCart(id: number) {
    setCart((prev) => {
      const updated = { ...prev };
      delete updated[id];
      return updated;
    });
  }

  function getCartCount(id: number) {
    return cart[id] ?? 0;
  }

  const totalCartItems = useMemo(
    () => Object.values(cart).reduce((sum, count) => sum + count, 0),
    [cart]
  );

  return (
    <ShopContext.Provider
      value={{
        favourites,
        cart,
        toggleFavourite,
        addToCart,
        increaseCart,
        decreaseCart,
        removeFromCart,
        getCartCount,
        totalCartItems,
        isFavourite,
      }}
    >
      {children}
    </ShopContext.Provider>
  );
}

export function useShop() {
  const context = useContext(ShopContext);

  if (!context) {
    throw new Error("useShop must be used inside ShopProvider");
  }

  return context;
}