import { ShopProvider } from "@/context/ShopContext";

export default function ShopAvailableLayout({
  children,
}: {
  children: React.ReactNode;
}) {
  return <ShopProvider>{children}</ShopProvider>;
}