import type { Metadata } from "next";
import Header from "@/components/Header";
import Footer from "@/components/Footer";

export const metadata: Metadata = {
  title: "Login",
  description: "Access your Advayam account.",
};

export default function AuthenticationPage() {
  return (
    <main className="flex min-h-screen flex-col">
      <Header />
      <section className="px-6 pb-20 pt-40">
        <div className="mx-auto grid max-w-8xl gap-8 bg-blue md:grid-cols-2">
          <div>
            <h1 className="text-6xl font-black">Login</h1>
            <p className="mt-6">
              Access your Advayam account, orders, projects, and dashboard.
            </p>
          </div>
          <form className="rounded-[2rem] border-2 border-black p-8">
            <input
              className="mb-4 w-full rounded-full border-2 border-black px-5 py-3"
              placeholder="Email"
            />
            <input
              className="mb-6 w-full rounded-full border-2 border-black px-5 py-3"
              placeholder="Password"
              type="password"
            />
            <button className="rounded-full bg-[#c85a0a] px-7 py-4 font-bold text-white">
              Login
            </button>
          </form>
        </div>
      </section>
      <Footer />
    </main>
  );
}
