import type { PriceFilter, Product, ProductCategory } from "@/data/products";

export function filterProducts({
  products,
  activeCategory,
  priceFilter,
  searchQuery,
  favouriteIds,
  favouriteOnly = false,
}: {
  products: Product[];
  activeCategory?: ProductCategory;
  priceFilter: PriceFilter;
  searchQuery: string;
  favouriteIds?: number[];
  favouriteOnly?: boolean;
}) {
  return products.filter((product) => {
    const categoryMatch = activeCategory
      ? product.category === activeCategory
      : true;

    const priceMatch =
      priceFilter === "all" ||
      (priceFilter === "under-500" && product.price < 500) ||
      (priceFilter === "above-500" && product.price >= 500);

    const searchMatch = product.name
      .toLowerCase()
      .includes(searchQuery.toLowerCase());

    const favouriteMatch =
      !favouriteOnly || favouriteIds?.includes(product.id);

    return categoryMatch && priceMatch && searchMatch && favouriteMatch;
  });
}