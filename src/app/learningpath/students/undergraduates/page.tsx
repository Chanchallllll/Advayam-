import Header from "@/components/Header";
import Footer from "@/components/Footer";

export default function UndergraduatesPage() {
  return (
    <main>
      <Header />
      <section className="px-6 pb-20 pt-40">
        <div className="mx-auto max-w-8xl">
          <p className="font-bold text-[#c85a0a]">Students / Undergraduates</p>
          <h1 className="mt-4 max-w-4xl text-6xl font-black">
            Build real-world embedded and IoT applications.
          </h1>
          <div className="mt-12 grid gap-6 md:grid-cols-2">
            <Card
              title="Tutorials"
              items={[
                "GPIO Programming",
                "ADC and PWM",
                "UART SPI I2C",
                "IoT Fundamentals",
                "PlatformIO Workflow",
              ]}
            />
            <Card
              title="Projects"
              items={[
                "Smart Home System",
                "Data Logger",
                "Line Follower Robot",
                "Wireless Sensor Node",
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
