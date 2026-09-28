export type ProductCategory =
  | "core-boards"
  | "base-boards"
  | "accessories"
  | "starter-kits"
  | "sensors"
  | "new-arrivals";

export type PriceFilter = "all" | "under-500" | "above-500";

export type Product = {
  id: number;
  name: string;
  price: number;
  category: ProductCategory;
  image: string;
  isNew: boolean;
};

export const products: Product[] = [
  {
    id: 1,
    name: "ESP32 Core Board",
    price: 499,
    category: "core-boards",
    image: "/products/board.svg",
    isNew: true,
  },
  {
    id: 2,
    name: "STM32H563 Board",
    price: 1299,
    category: "core-boards",
    image: "/products/board.svg",
    isNew: true,
  },
  {
    id: 3,
    name: "RP2040 Core Board",
    price: 699,
    category: "core-boards",
    image: "/products/board.svg",
    isNew: true,
  },
  {
    id: 4,
    name: "nRF52840 Board",
    price: 1499,
    category: "core-boards",
    image: "/products/board.svg",
    isNew: true,
  },
  {
    id: 5,
    name: "EFM8 Learning Board",
    price: 399,
    category: "core-boards",
    image: "/products/board.svg",
    isNew: false,
  },
  {
    id: 6,
    name: "ESP32-S3 Edge Board",
    price: 999,
    category: "core-boards",
    image: "/products/board.svg",
    isNew: true,
  },
  {
    id: 7,
    name: "RP2040 Logger Board",
    price: 899,
    category: "core-boards",
    image: "/products/board.svg",
    isNew: false,
  },
  {
    id: 8,
    name: "Advayam Mini Core",
    price: 299,
    category: "core-boards",
    image: "/products/board.svg",
    isNew: true,
  },
  {
    id: 9,
    name: "Industrial MCU Board",
    price: 1899,
    category: "core-boards",
    image: "/products/board.svg",
    isNew: true,
  },

  {
    id: 10,
    name: "Explorer Board",
    price: 4500,
    category: "base-boards",
    image: "/products/board.svg",
    isNew: true,
  },
  {
    id: 11,
    name: "GPIO Kit",
    price: 299,
    category: "accessories",
    image: "/products/board.svg",
    isNew: false,
  },
];

export const categoryOptions = [
  { label: "Core Boards", value: "core-boards" },
  { label: "Base Boards", value: "base-boards" },
  { label: "Accessories", value: "accessories" },
  { label: "Starter Kits", value: "starter-kits" },
  { label: "Sensors", value: "sensors" },
  { label: "New Arrivals", value: "new-arrivals" },
] as const;

export const priceFilters = [
  { label: "All", value: "all" },
  { label: "Under ₹500", value: "under-500" },
  { label: "Above ₹500", value: "above-500" },
] as const;