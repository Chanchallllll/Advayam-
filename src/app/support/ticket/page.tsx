import Header from "@/components/Header";
import TicketForm from "@/components/support/TicketForm";

export default function TicketPage() {
  return (
    <main className="relative h-screen overflow-hidden bg-white text-[#202020]">
      <Header />

      <img
        src="/patterns/wave-left-yellow.svg"
        alt=""
        className="pointer-events-none absolute bottom-0 left-0 w-[320px] opacity-120"
      />

      <img
        src="/patterns/wave-right-yellow.svg"
        alt=""
        className="pointer-events-none absolute right-0 top-[90px] w-[440px] opacity-120"
      />

      <section className="relative z-10 mx-auto flex h-screen max-w-[1500px] items-center px-10 pb-5 pt-[92px] max-sm:px-5">
        <TicketForm />
      </section>
    </main>
  );
}