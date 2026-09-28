import Header from "@/components/Header";
import Footer from "@/components/Footer";

export default function PostgraduatesPage() {
  return (
    <main>
      <Header />
      <section className="px-6 pb-20 pt-40">
        <div className="mx-auto max-w-8xl">
          <p className="font-bold text-[#c85a0a]">Students / Postgraduates</p>
          <h1 className="mt-4 max-w-4xl text-6xl font-black">
            Explore advanced embedded research systems.
          </h1>
          <div className="mt-12 grid gap-6 md:grid-cols-2">
            <Card
              title="Advanced Topics"
              items={[
                "FreeRTOS",
                "Zephyr RTOS",
                "Embedded Networking",
                "Edge AI",
                "Research Platforms",
              ]}
            />
            <Card
              title="Outcomes"
              items={[
                "RTOS Task Design",
                "Low-level Control",
                "Networked Systems",
                "AI-ready Edge Devices",
              ]}
            />
          </div>
        </div>
      </section>
      <Footer />
    </main>
  );
}

function Card({ title, items }: { title: string; items: string[] }) {
  return (
    <div className="rounded-[2rem] border-2 border-black p-8">
      <h2 className="text-3xl font-black">{title}</h2>
      <div className="mt-6 grid gap-3">
        {items.map((i) => (
          <p key={i} className="rounded-2xl bg-[#DBDBDB] p-4 font-semibold">
            {i}
          </p>
        ))}
      </div>
    </div>
  );
}
