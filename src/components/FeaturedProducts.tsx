import { ArrowRight } from "lucide-react";

const products = [
  {
    name: "Starter Kit",
    imageLabel: "Image_1",
    badge: "New",
    href: "/shop/starter-kit",
  },
  {
    name: "Starter Kit",
    imageLabel: "Image_2",
    badge: "New",
    href: "/shop/starter-kit-2",
  },
];

export default function FeaturedProducts() {
  return (
    <section className="bg-[#FFFFFF] px-6 py-40 text-[#202020]">
      <img
        src="/patterns/wave-left.svg"
        alt=""
        className="pointer-events-none absolute -left-20 top-[493%] z-0 w-[720px] -translate-y-1/2 opacity-125"
      />
      <div className="mx-auto max-w-5xl">
        <h2 className="font-jakarta text-center text-4xl font-black">
          Featured Products
        </h2>

        <div className="mx-auto mt-8 grid max-w-3xl gap-8 md:grid-cols-2">
          {products.map((product) => (
            <article
              key={product.imageLabel}
              className="border-2 border-black bg-[#FFFFFF]"
            >
              <div className="relative flex h-[250px] items-center justify-center border-b-2 border-black">
                <span className="font-jakarta text-xl font-medium">
                  {product.imageLabel}
                </span>

                <span className="absolute right-4 top-4 rounded-full border-2 border-black px-4 py-2 font-jakarta text-sm font-medium">
                  {product.badge}
                </span>
              </div>

              <div className="grid grid-cols-[1fr_72px] border-b-2 border-black">
                <div className="flex h-[70px] items-center px-6">
                  <h3 className="font-jakarta text-2xl font-black">
                    {product.name}
                  </h3>
                </div>

                <a
                  href={product.href}
                  className="flex items-center justify-center border-l-2 border-black transition hover:bg-[#DBDBDB]"
                  aria-label={`View ${product.name}`}
                >
                  <ArrowRight className="h-5 w-5" />
                </a>
              </div>

              <a
                href={product.href}
                className="flex h-[70px] items-center justify-center font-jakarta text-lg font-semibold transition hover:bg-[#DBDBDB]"
              >
                Buy Now
              </a>
            </article>
          ))}
        </div>
      </div>
    </section>
  );
}
