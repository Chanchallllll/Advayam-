import AuthHeader from "@/components/auth/AuthHeader";
import LoginForm from "@/components/auth/LoginForm";

export default function LoginPage() {
  return (
    <main className="relative min-h-screen overflow-hidden bg-black text-[#202020]">
      <AuthHeader />

      <img
        src="/patterns/wave-left-yellow.svg"
        alt=""
        className="pointer-events-none absolute bottom-0 left-0 w-[420px] opacity-120"
      />

      <img
        src="/patterns/wave-right-yellow.svg"
        alt=""
        className="pointer-events-none absolute bottom-0 right-0 w-[520px] opacity-120"
      />

      <section className="relative z-10 flex min-h-screen items-center justify-center px-6 pb-16 pt-[120px]">
        <LoginForm />
      </section>
    </main>
  );
}