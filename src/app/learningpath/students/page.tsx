import Header from "@/components/Header";
import Footer from "@/components/Footer";

const items = [
  [
    "School (K-12)",
    "/learningpath/students/school",
    "LEDs, sensors, motors, logic building, and simple automation.",
  ],
  [
    "Undergraduates",
    "/learningpath/students/undergraduates",
    "GPIO, protocols, IoT, embedded C/C++, and simulations.",
  ],
  [
    "Postgraduates",
    "/learningpath/students/postgraduates",
    "RTOS, embedded networking, Zephyr, FreeRTOS, and Edge AI.",
  ],
];

export default function StudentsPage() {
  return (
    <main className="flex min-h-screen flex-col">
      <Header />
      <section className="px-6 pb-20 pt-40">
        <div className="mx-auto max-w-8xl">
          <p className="font-bold text-[#c85a0a]">Students</p>
          <h1 className="mt-4 text-6xl font-black">
            Academic paths for every level.
          </h1>
          <div className="mt-12 grid gap-6 md:grid-cols-3">
            {items.map(([title, href, text]) => (
              <a
                key={title}
                href={href}
                className="rounded-[2rem] border-2 border-black p-8 hover:bg-[#DBDBDB]"
              >
                <h2 className="text-3xl font-black">{title}</h2>
                <p className="mt-4 leading-7">{text}</p>
              </a>
            ))}
          </div>
        </div>
      </section>
      <Footer />
    </main>
  );
}
