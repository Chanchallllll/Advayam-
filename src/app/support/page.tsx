import Header from "@/components/Header";
import Link from "next/link";
import {
  Download,
  FileText,
  HelpCircle,
  Mail,
  MessageSquare,
  Wrench,
} from "lucide-react";

const supportCards = [
  {
    title: "Installation Guide",
    description:
      "Follow the official Advayam IDE setup workflow from download to first project.",
    icon: Wrench,
    href: "/support#installation-guide",
  },
  {
    title: "Download Center",
    description:
      "Download Advayam IDE, VS Code Extension, board tools, drivers, and setup resources.",
    icon: Download,
    href: "/downloads",
  },
  {
    title: "Documentation",
    description:
      "Read hardware, software, programming, board setup, and API reference documentation.",
    icon: FileText,
    href: "/documentation",
  },
  {
    title: "Raise a Ticket",
    description:
      "Report technical issues, account problems, hardware faults, or software bugs.",
    icon: MessageSquare,
    href: "/support/ticket",
  },
];

const installationSteps = [
  {
    number: "01",
    title: "Download the installer",
    description:
      "Download the latest Advayam IDE setup package from the Downloads page.",
  },
  {
    number: "02",
    title: "Run the setup file",
    description:
      "Launch the installer and complete the guided installation process.",
  },
  {
    number: "03",
    title: "Install board drivers",
    description:
      "Install the Advayam USB Driver so the IDE can detect supported hardware.",
  },
  {
    number: "04",
    title: "Create your first project",
    description:
      "Launch Advayam IDE, create a project, build, upload and monitor your device.",
  },
];

export default function SupportPage() {
  return (
    <main className="relative min-h-screen overflow-hidden bg-black text-white">
      <Header />

      <img
        src="/patterns/wave-left.svg"
        alt=""
        className="pointer-events-none absolute bottom-0 left-0 w-[420px] opacity-20"
      />

      <img
        src="/patterns/wave-right.svg"
        alt=""
        className="pointer-events-none absolute right-0 top-[120px] w-[520px] opacity-20"
      />

      <section className="relative z-10 mx-auto max-w-[1300px] px-8 pb-20 pt-[140px]">
        <div className="max-w-3xl">
          <p className="font-jakarta text-sm font-black uppercase tracking-[0.18em] text-[#d95b00]">
            Support
          </p>

          <h1 className="mt-4 font-jakarta text-6xl font-black leading-none text-[#202020]">
            How can we help?
          </h1>

          <p className="mt-6 font-jakarta text-lg font-medium leading-8 text-[#202020]/70">
            Get help with Advayam installation, documentation, downloads,
            technical issues, and product support.
          </p>
        </div>

        <div className="mt-14 grid grid-cols-4 gap-6 max-xl:grid-cols-2 max-md:grid-cols-1">
          {supportCards.map((card) => {
            const Icon = card.icon;

            return (
              <Link
                key={card.title}
                href={card.href}
                className="rounded-3xl border border-[#202020]/10 bg-white p-7 shadow-sm transition hover:-translate-y-1 hover:shadow-md"
              >
                <div className="flex h-12 w-12 items-center justify-center rounded-2xl bg-[#d95b00]/10 text-[#d95b00]">
                  <Icon className="h-6 w-6" />
                </div>

                <h2 className="mt-6 font-jakarta text-xl font-black">
                  {card.title}
                </h2>

                <p className="mt-3 font-jakarta text-sm font-medium leading-6 text-[#202020]/60">
                  {card.description}
                </p>
              </Link>
            );
          })}
        </div>

        <section
          id="installation-guide"
          className="mt-20 rounded-[2rem] border border-[#202020]/10 bg-white/85 p-10 shadow-sm backdrop-blur"
        >
          <div className="flex items-center gap-4">
            <HelpCircle className="h-8 w-8 text-[#d95b00]" />

            <h2 className="font-jakarta text-3xl font-black">
              Advayam Software Installation Guide
            </h2>
          </div>

          <p className="mt-5 max-w-3xl font-jakarta text-sm font-medium leading-7 text-[#202020]/65">
            Follow these steps to install Advayam IDE, configure the required
            board tools, and start your first embedded project.
          </p>

          <div className="mt-8 grid gap-6 md:grid-cols-2">
            {installationSteps.map((step) => (
              <GuideStep
                key={step.number}
                number={step.number}
                title={step.title}
                description={step.description}
              />
            ))}
          </div>

          <div className="mt-10 flex flex-wrap gap-4">
            <Link
              href="/downloads"
              className="rounded-xl bg-[#d95b00] px-6 py-3 font-jakarta text-sm font-black text-white"
            >
              Go to Downloads
            </Link>

            <Link
              href="/support/ticket"
              className="rounded-xl border border-[#202020]/15 bg-white px-6 py-3 font-jakarta text-sm font-black"
            >
              Raise a Ticket
            </Link>
          </div>
        </section>

        <section className="mt-14 rounded-[2rem] border border-[#202020]/10 bg-white/85 p-10 shadow-sm backdrop-blur">
          <div className="flex items-center gap-4">
            <Mail className="h-8 w-8 text-[#d95b00]" />

            <h2 className="font-jakarta text-3xl font-black">
              Contact Support
            </h2>
          </div>

          <p className="mt-5 max-w-3xl font-jakarta text-sm font-medium leading-7 text-[#202020]/65">
            For technical help, business enquiries, partnerships, or product
            guidance, contact the Advayam team through the ticket system or the
            official contact page.
          </p>

          <Link
            href="/contact"
            className="mt-8 inline-flex rounded-xl bg-[#202020] px-6 py-3 font-jakarta text-sm font-black text-white"
          >
            Contact Us
          </Link>
        </section>
      </section>
    </main>
  );
}

function GuideStep({
  number,
  title,
  description,
}: {
  number: string;
  title: string;
  description: string;
}) {
  return (
    <div className="rounded-2xl border border-[#202020]/10 bg-white p-6">
      <span className="font-jakarta text-sm font-black text-[#d95b00]">
        {number}
      </span>

      <h3 className="mt-3 font-jakarta text-lg font-black">{title}</h3>

      <p className="mt-2 font-jakarta text-sm font-medium leading-6 text-[#202020]/60">
        {description}
      </p>
    </div>
  );
}