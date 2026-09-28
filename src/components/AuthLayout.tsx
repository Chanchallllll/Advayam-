import Image from "next/image";

export default function AuthLayout({
  title,
  subtitle,
  children,
}: {
  title: string;
  subtitle: string;
  children: React.ReactNode;
}) {
  return (
    <div className="min-h-screen bg-[#FFFFFF]">
      <div className="grid min-h-screen lg:grid-cols-2">
        {/* Left */}
        <div className="flex flex-col justify-center bg-[#DBDBDB] px-16">
          <img
            src="/logos/advayam-logo.svg"
            alt="Advayam"
            className="h-24 md:h-28 w-fit object-contain transition-transform hover:scale-105"
          />

          <h1 className="font-druk mt-8 text-7xl">ADVAYAM</h1>

          <p className="font-satoshi mt-6 max-w-xl text-2xl">
            India's Embedded Ecosystem
          </p>

          <p className="font-jakarta mt-6 max-w-xl text-lg">
            Learn. Prototype. Innovate. Build.
          </p>

          <img
            src="/illustrations/auth-hero.svg"
            alt=""
            className="mt-12 max-w-lg"
          />
        </div>

        {/* Right */}
        <div className="flex items-center justify-center px-8">
          <div className="w-full max-w-md">
            <h2 className="font-jakarta text-4xl font-black">{title}</h2>

            <p className="mt-3 text-slate-600">{subtitle}</p>

            {children}
          </div>
        </div>
      </div>
    </div>
  );
}
